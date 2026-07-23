#!/usr/bin/env node
/* End-to-end smoke test for the embedded Web UI. Requires playwright@1.61.1. */

import { spawn } from "node:child_process";
import { mkdtemp, mkdir, rm, writeFile } from "node:fs/promises";
import net from "node:net";
import os from "node:os";
import path from "node:path";
import process from "node:process";
import { chromium, firefox, webkit } from "playwright";

const args = new Map();
for (let index = 2; index < process.argv.length; index += 2) {
  args.set(process.argv[index], process.argv[index + 1]);
}

const executable = path.resolve(args.get("--executable") || "");
const requestedBrowsers = (args.get("--browsers") || "chromium")
  .split(",").map(value => value.trim()).filter(Boolean);
const artifactRoot = args.get("--artifacts") ? path.resolve(args.get("--artifacts")) : null;
const username = "admin";
const password = "OpenIPC-P11-Smoke!";
const webModules = ["core.js", "monitor.js", "devices.js", "admin.js", "app.js"];

if (!args.get("--executable")) {
  throw new Error("Usage: web_browser_smoke.mjs --executable <Dashboard binary> [--browsers chromium,firefox,edge]");
}

async function freePort() {
  return await new Promise((resolve, reject) => {
    const server = net.createServer();
    server.once("error", reject);
    server.listen(0, "127.0.0.1", () => {
      const port = server.address().port;
      server.close(error => error ? reject(error) : resolve(port));
    });
  });
}

async function waitForReady(url, child, output, timeoutMs = 25000) {
  const deadline = Date.now() + timeoutMs;
  while (Date.now() < deadline) {
    if (child.exitCode !== null) {
      throw new Error(`Dashboard exited before readiness (${child.exitCode})\n${output.value}`);
    }
    try {
      const response = await fetch(`${url}/api/v1/health/ready`);
      if (response.ok && (await response.json()).data?.ready === true) return;
    } catch (_) {}
    await new Promise(resolve => setTimeout(resolve, 200));
  }
  throw new Error(`Dashboard readiness timeout\n${output.value}`);
}

function launchBrowser(name) {
  if (name === "chromium") return chromium.launch({ headless: true });
  if (name === "firefox") return firefox.launch({ headless: true });
  if (name === "webkit") return webkit.launch({ headless: true });
  if (name === "edge") return chromium.launch({ headless: true, channel: "msedge" });
  throw new Error(`Unsupported browser: ${name}`);
}

async function assertNoHorizontalOverflow(page, label) {
  const dimensions = await page.evaluate(() => ({
    documentClient: document.documentElement.clientWidth,
    documentScroll: document.documentElement.scrollWidth,
    bodyClient: document.body.clientWidth,
    bodyScroll: document.body.scrollWidth
  }));
  if (dimensions.documentScroll > dimensions.documentClient + 1
      || dimensions.bodyScroll > dimensions.bodyClient + 1) {
    throw new Error(`${label} has horizontal overflow: ${JSON.stringify(dimensions)}`);
  }
}

async function login(page, baseUrl, loginUsername = username, loginPassword = password) {
  await page.goto(baseUrl, { waitUntil: "networkidle" });
  await page.locator("#username").fill(loginUsername);
  await page.locator("#password").fill(loginPassword);
  await Promise.all([
    page.waitForResponse(response => response.url().endsWith("/api/v1/auth/login")),
    page.locator("#login-form button[type=submit]").click()
  ]);
  await page.locator("#app-view").waitFor({ state: "visible" });
}

async function browserApi(page, apiPath, options = {}) {
  return await page.evaluate(async ({ apiPath: target, options: request }) => {
    const response = await fetch(target, {
      credentials: "same-origin",
      headers: { "Content-Type": "application/json", ...(request.headers || {}) },
      ...request,
      body: request.body === undefined ? undefined : JSON.stringify(request.body)
    });
    let body = null;
    try { body = await response.json(); } catch (_) {}
    return { status: response.status, body };
  }, { apiPath, options });
}

async function assertAccessibleControls(page, label) {
  const missing = await page.evaluate(() => Array.from(
    document.querySelectorAll("button, a[href], input, select, textarea"))
    .filter(element => element.getClientRects().length > 0 && !element.disabled)
    .filter(element => {
      const id = element.id;
      const explicitLabel = id && document.querySelector(`label[for="${id}"]`);
      const wrappedLabel = element.closest("label");
      const ownText = (element.textContent || "").trim();
      return !(element.getAttribute("aria-label") || element.getAttribute("aria-labelledby")
        || element.getAttribute("title") || element.getAttribute("placeholder")
        || explicitLabel || wrappedLabel || ownText);
    })
    .map(element => element.id || element.outerHTML.slice(0, 100)));
  if (missing.length) throw new Error(`${label} has unnamed controls: ${missing.join(", ")}`);
}

async function runBrowser(name, baseUrl) {
  const browser = await launchBrowser(name);
  const errors = [];
  const failedAssets = [];
  try {
    const desktop = await browser.newPage({ viewport: { width: 1440, height: 900 } });
    desktop.on("console", message => {
      if (message.type() === "error") errors.push(`${name}/desktop: ${message.text()}`);
    });
    desktop.on("response", response => {
      const resource = new URL(response.url()).pathname.slice(1);
      if (webModules.includes(resource) && !response.ok()) failedAssets.push(`${resource}: ${response.status()}`);
    });
    await login(desktop, baseUrl);
    await desktop.waitForFunction(() => typeof renderMonitorGrid === "function"
      && typeof openDiscovery === "function" && typeof renderUsers === "function");
    await assertNoHorizontalOverflow(desktop, `${name}/desktop`);
    await assertAccessibleControls(desktop, `${name}/desktop`);

    const settings = await browserApi(desktop, "/api/v1/settings");
    if (settings.status !== 200) throw new Error(`${name}: settings GET returned ${settings.status}`);
    const settingsUpdate = await browserApi(desktop, "/api/v1/settings", {
      method: "POST", body: { language: settings.body?.data?.values?.language || "ru" }
    });
    if (settingsUpdate.status !== 200) throw new Error(`${name}: settings POST returned ${settingsUpdate.status}`);
    const archiveMissing = await browserApi(desktop, `/api/v1/archive/file/${"0".repeat(64)}`);
    if (archiveMissing.status !== 404) throw new Error(`${name}: invalid archive id returned ${archiveMissing.status}`);

    const viewerPassword = "Viewer-P11-Smoke!";
    const viewerCreate = await browserApi(desktop, "/api/v1/users/create", {
      method: "POST", body: { username: "viewer", password: viewerPassword, role: "operator", permissions: 1 }
    });
    if (![201, 409].includes(viewerCreate.status)) {
      throw new Error(`${name}: viewer creation returned ${viewerCreate.status}`);
    }

    const csrf = await desktop.context().request.post(`${baseUrl}/api/v1/settings`, {
      headers: { Origin: "http://untrusted.invalid" }, data: { language: "ru" }
    });
    if (csrf.status() !== 403) throw new Error(`${name}: cross-origin settings mutation returned ${csrf.status()}`);

    const healthAction = desktop.locator('[data-view="health"]');
    await healthAction.focus();
    await healthAction.press("Enter");
    await desktop.locator("#dialog-backdrop").waitFor({ state: "visible" });
    await desktop.keyboard.press("Escape");
    await desktop.locator("#dialog-backdrop").waitFor({ state: "hidden" });
    const returnedFocus = await desktop.evaluate(() => document.activeElement?.id || "");
    if (returnedFocus !== "action-health") throw new Error(`${name}: dialog focus returned to ${returnedFocus}`);

    for (const view of ["health", "settings", "users"]) {
      await desktop.locator(`[data-view="${view}"]`).click();
      await desktop.locator("#dialog-backdrop").waitFor({ state: "visible" });
      await assertNoHorizontalOverflow(desktop, `${name}/desktop/${view}`);
      await desktop.locator("#dialog-close").click();
    }

    if (artifactRoot) {
      await mkdir(artifactRoot, { recursive: true });
      await desktop.screenshot({ path: path.join(artifactRoot, `${name}-desktop.png`), fullPage: true });
    }

    const viewer = await browser.newPage({ viewport: { width: 1024, height: 768 } });
    await login(viewer, baseUrl, "viewer", viewerPassword);
    if (await viewer.locator("#action-settings").isVisible()
        || await viewer.locator("#action-users").isVisible()) {
      throw new Error(`${name}: restricted administration actions are visible to viewer`);
    }
    const deniedUsers = await browserApi(viewer, "/api/v1/users");
    const deniedArchive = await browserApi(viewer, `/api/v1/archive/file/${"0".repeat(64)}`);
    if (deniedUsers.status !== 403 || deniedArchive.status !== 403) {
      throw new Error(`${name}: role boundary failed (${deniedUsers.status}/${deniedArchive.status})`);
    }
    await viewer.close();

    const mobile = await browser.newPage({ viewport: { width: 390, height: 844 }, isMobile: true });
    mobile.on("console", message => {
      if (message.type() === "error") errors.push(`${name}/mobile: ${message.text()}`);
    });
    await login(mobile, baseUrl);
    await mobile.locator("#monitor-grid").waitFor({ state: "visible" });
    await assertNoHorizontalOverflow(mobile, `${name}/mobile`);
    await assertAccessibleControls(mobile, `${name}/mobile`);
    if (artifactRoot) {
      await mobile.screenshot({ path: path.join(artifactRoot, `${name}-mobile.png`), fullPage: true });
    }

    if (failedAssets.length || errors.length) {
      throw new Error(`Browser errors: ${[...failedAssets, ...errors].join(" | ")}`);
    }
    await desktop.close();
    await mobile.close();
    return { browser: name, desktop: "1440x900", mobile: "390x844", modules: webModules.length };
  } finally {
    await browser.close();
  }
}

const tempRoot = await mkdtemp(path.join(os.tmpdir(), "openipc-web-smoke-"));
const configRoot = path.join(tempRoot, "config");
const dataRoot = path.join(tempRoot, "data");
const homeRoot = path.join(tempRoot, "home");
await mkdir(dataRoot, { recursive: true });
await mkdir(homeRoot, { recursive: true });
const passwordFile = path.join(tempRoot, "initial-admin-password.txt");
await writeFile(passwordFile, `${password}\n`, { encoding: "utf8", mode: 0o600 });

const httpPort = await freePort();
const webSocketPort = await freePort();
const baseUrl = `http://127.0.0.1:${httpPort}`;
const environment = {
  ...process.env,
  HOME: homeRoot,
  APPDATA: configRoot,
  LOCALAPPDATA: dataRoot,
  XDG_CONFIG_HOME: configRoot,
  XDG_DATA_HOME: dataRoot,
  OPENIPC_DISABLE_CUSTOM_LOGGER: "1",
  OPENIPC_TEST_SECRET_STORE: "settings",
  OPENIPC_INITIAL_ADMIN_PASSWORD_FILE: passwordFile,
  OPENIPC_WEB_DEPLOYMENT_PROFILE: "localhost",
  OPENIPC_WEB_BIND_ADDRESS: "127.0.0.1",
  OPENIPC_WEB_PORT: String(httpPort),
  OPENIPC_WEBSOCKET_PORT: String(webSocketPort),
  OPENIPC_DATA_ROOT: homeRoot
};
const child = spawn(executable, ["--server-only", "--initialize-admin", username], {
  env: environment,
  stdio: ["ignore", "pipe", "pipe"],
  windowsHide: true
});
const output = { value: "" };
for (const stream of [child.stdout, child.stderr]) {
  stream.on("data", chunk => { output.value = (output.value + chunk.toString()).slice(-32000); });
}

try {
  await waitForReady(baseUrl, child, output);
  const results = [];
  for (const browserName of requestedBrowsers) results.push(await runBrowser(browserName, baseUrl));
  console.log(JSON.stringify({ ok: true, baseUrl, results }));
} finally {
  if (child.exitCode === null) child.kill();
  await Promise.race([
    new Promise(resolve => child.once("exit", resolve)),
    new Promise(resolve => setTimeout(resolve, 5000))
  ]);
  await rm(tempRoot, { recursive: true, force: true });
}
