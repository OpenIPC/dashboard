"use strict";

function showToast(message, error = false) {
  const toast = byId("toast");
  toast.textContent = message;
  toast.classList.toggle("error", error);
  toast.hidden = false;
  window.clearTimeout(state.toastTimer);
  state.toastTimer = window.setTimeout(() => { toast.hidden = true; }, 3500);
}

function showLogin(message = "") {
  stopAllWebRtcPeers();
  stopPushToTalk();
  window.clearInterval(state.pageCycleTimer);
  state.pageCycleTimer = null;
  if (state.kiosk) setKiosk(false);
  closeCameraDialog();
  if (state.socket) state.socket.close();
  state.monitorSignature = "";
  window.clearInterval(state.refreshTimer);
  window.clearInterval(state.previewTimer);
  byId("app-view").hidden = true;
  byId("login-view").hidden = false;
  byId("login-error").textContent = message;
  byId("password").value = "";
  byId("username").focus();
}

function showApp(session) {
  state.session = session;
  state.capabilities = session.capabilities || null;
  byId("login-view").hidden = true;
  byId("app-view").hidden = false;
  byId("current-user").textContent = session.username;
  byId("sidebar-user").textContent = session.username;
  byId("device-commands").hidden = !Boolean(
    state.capabilities?.administration?.cameraOnboarding);
  applyCapabilityVisibility();
  restoreSidebarState();
  setLayout(state.layout, false);
  loadPresentation();
  loadDashboard();
  connectSocket();
  window.clearInterval(state.refreshTimer);
  state.refreshTimer = window.setInterval(() => {
    if (!state.socket || state.socket.readyState !== WebSocket.OPEN) loadDashboard();
  }, 5000);
  window.clearInterval(state.previewTimer);
  state.previewTimer = window.setInterval(refreshPreviewImages, 2000);
}

function applyCapabilityVisibility() {
  const administration = state.capabilities?.administration || {};
  [["settings", administration.settings], ["users", administration.users],
   ["logs", administration.logs], ["devices", administration.devices]].forEach(([name, visible]) => {
    const action = byId(`action-${name}`);
    const tab = byId(`tab-${name}`);
    if (action) action.hidden = !visible;
    if (tab) tab.hidden = !visible;
  });
}

async function loadPresentation() {
  try {
    state.presentation = await api("/api/v1/presentation");
    state.capabilities = state.presentation.capabilities || state.capabilities;
    applyDesignTokens(state.presentation.designTokens || {});
    applyCapabilityVisibility();
  } catch (error) {
    console.warn("Presentation contract is unavailable", error);
  }
}

function applyDesignTokens(tokens) {
  const colors = tokens.colors || {};
  const metrics = tokens.metrics || {};
  const root = document.documentElement.style;
  const colorVariables = {
    background: "--bg", sidebar: "--chrome", surface: "--panel-2",
    surfaceAlt: "--panel", tile: "--panel-3", stroke: "--border",
    strokeStrong: "--border-strong", accent: "--blue", success: "--green",
    danger: "--red", warning: "--amber", textPrimary: "--text", textMuted: "--muted"
  };
  Object.entries(colorVariables).forEach(([key, variable]) => {
    if (colors[key]) root.setProperty(variable, colors[key]);
  });
  if (metrics.sidebarWidth) root.setProperty("--sidebar", `${metrics.sidebarWidth}px`);
  if (metrics.topBarHeight) root.setProperty("--topbar", `${metrics.topBarHeight}px`);
  if (metrics.statusBarHeight) root.setProperty("--statusbar", `${metrics.statusBarHeight}px`);
}

async function loadDashboard() {
  try {
    renderDashboard(await api("/api/v1/dashboard"));
    byId("connection-state").classList.add("connected");
  } catch (error) {
    byId("connection-state").classList.remove("connected");
    console.error(error);
  }
}

function normalizeAssignments(cameras) {
  const keys = new Set(cameras.map(cameraKey));
  state.assignments = state.assignments.map(key => keys.has(String(key)) ? String(key) : null);
  compactAssignmentPages();
  if (!state.assignments.some(Boolean) && cameras.length) state.assignments[0] = cameraKey(cameras[0]);
  state.page = Math.min(state.page, workspacePageCount() - 1);
  const start = workspacePageStart();
  if (state.activeCell < start || state.activeCell >= start + state.layout) state.activeCell = start;
  if (state.openControlsCell !== null
      && !state.assignments[state.openControlsCell]) {
    state.openControlsCell = null;
  }
  persistWorkspace();
  updatePageControls();
}

function compactAssignmentPages() {
  while (state.assignments.length < state.layout
         || (state.assignments.length % state.layout) !== 0) state.assignments.push(null);
  while (state.assignments.length > state.layout
         && state.assignments.slice(-state.layout).every(value => !value)) {
    state.assignments.splice(-state.layout);
  }
}

function selectNextEmptyCell() {
  setCellControlsOpen(null);
  const start = workspacePageStart();
  let next = state.assignments.findIndex((value, index) => index >= start
    && index < start + state.layout && !value);
  if (next < 0) {
    const existing = state.assignments.findIndex(value => !value);
    if (existing >= 0) {
      next = existing;
      state.page = Math.floor(existing / state.layout);
    } else {
      state.assignments.push(...Array(state.layout).fill(null));
      state.page = workspacePageCount() - 1;
      next = workspacePageStart();
    }
  }
  state.activeCell = next;
  persistWorkspace();
  updatePageControls();
  if (state.dashboard) renderMonitorGrid(state.dashboard.cameras || []);
}

function workspacePageCount() {
  return Math.max(1, Math.ceil(Math.max(state.assignments.length, state.layout) / state.layout));
}

function workspacePageStart() { return state.page * state.layout; }

function updatePageControls() {
  const count = workspacePageCount();
  state.page = Math.max(0, Math.min(state.page, count - 1));
  document.querySelectorAll("[data-page-indicator]").forEach(node => {
    node.textContent = `${state.page + 1} / ${count}`;
  });
  document.querySelectorAll("[data-cycle-toggle]").forEach(button => {
    button.classList.toggle("active", state.pageCycling);
    button.setAttribute("aria-pressed", String(state.pageCycling));
  });
  document.querySelectorAll('[data-page-nav="-1"]').forEach(button => { button.disabled = count <= 1; });
  document.querySelectorAll('[data-page-nav="1"]').forEach(button => { button.disabled = count <= 1; });
  schedulePageCycling();
}

function setPage(page, wrap = false) {
  const count = workspacePageCount();
  let next = Number(page);
  if (wrap) next = ((next % count) + count) % count;
  else next = Math.max(0, Math.min(next, count - 1));
  if (next === state.page && state.monitorSignature) { updatePageControls(); return; }
  stopPushToTalk();
  setCellControlsOpen(null);
  state.page = next;
  state.activeCell = workspacePageStart();
  state.monitorSignature = "";
  persistWorkspace();
  updatePageControls();
  if (state.dashboard) renderMonitorGrid(state.dashboard.cameras || []);
}

function schedulePageCycling() {
  if (!state.pageCycling || workspacePageCount() <= 1 || byId("app-view").hidden) {
    window.clearInterval(state.pageCycleTimer);
    state.pageCycleTimer = null;
    return;
  }
  if (state.pageCycleTimer) return;
  state.pageCycleTimer = window.setInterval(() => setPage(state.page + 1, true), 10000);
}

function togglePageCycling() {
  state.pageCycling = !state.pageCycling;
  persistWorkspace();
  updatePageControls();
}

async function toggleKiosk() {
  if (state.kiosk) {
    if (document.fullscreenElement && document.exitFullscreen) await document.exitFullscreen().catch(() => {});
    setKiosk(false);
    return;
  }
  setKiosk(true);
  if (document.documentElement.requestFullscreen) {
    await document.documentElement.requestFullscreen().catch(() => {});
  }
}

function setKiosk(enabled) {
  state.kiosk = Boolean(enabled);
  byId("app-view").classList.toggle("kiosk", state.kiosk);
  document.body.classList.toggle("kiosk-active", state.kiosk);
  state.monitorSignature = "";
  if (state.dashboard) renderMonitorGrid(state.dashboard.cameras || []);
}

function renderHealth(data) {
  state.health = data;
  const results = data.results || [];
  const cameras = state.dashboard?.cameras || [];
  const problems = results.filter(result => result.success === false || ["offline", "error", "problem"].includes(String(result.status || "").toLowerCase())).length;
  byId("health-total").textContent = cameras.length || results.length;
  byId("health-online").textContent = cameras.filter(camera => isOnline(camera.status)).length;
  byId("health-problems").textContent = problems;
  byId("health-progress").textContent = data.running ? `${text("checksRunning")}: ${data.completedProbes || 0} / ${data.totalProbes || 0}` : "";
  byId("health-body").innerHTML = results.length ? results.map(result => {
    const temperature = numericTemperature(result);
    return `<tr><td>${escapeHtml(result.cameraName || result.name || result.cameraIp || result.ip || "-")}</td><td>${escapeHtml(result.cameraIp || result.ip || "-")}</td><td>${escapeHtml(result.status || (result.success ? "OK" : "Error"))}</td><td>${escapeHtml(result.reason || result.message || "-")}</td><td>${temperature == null ? text("noTemperature") : `${Math.round(temperature)} °C`}</td></tr>`;
  }).join("") : `<tr><td colspan="5">${text("noData")}</td></tr>`;
}

async function loadAnalytics() {
  try { renderAnalytics(await api("/api/v1/analytics")); } catch (error) { byId("analytics-body").innerHTML = `<tr><td colspan="4">${escapeHtml(error.message)}</td></tr>`; }
}

function renderAnalytics(data) {
  state.analytics = data;
  const modules = data.modules || [];
  const events = data.events || [];
  const evidence = data.evidence || {};
  const diagnostics = data.diagnostics || {};
  const activeModules = modules.filter(module => ![false, "disabled", "off", "error"].includes(module.enabled ?? String(module.status || "").toLowerCase())).length;
  const detections = Number(diagnostics.detections || diagnostics.totalDetections || events.length || 0);
  const evidenceCount = Number(evidence.files || evidence.total || evidence.snapshots || 0);
  byId("analytics-summary").innerHTML = [
    ["activeModules", activeModules, "success"], ["events", events.length, ""], ["detections", detections, ""], ["evidence", evidenceCount, ""]
  ].map(([label, value, css]) => `<div><span>${text(label)}</span><strong class="${css}">${escapeHtml(value)}</strong></div>`).join("");
  byId("analytics-modules").innerHTML = modules.length ? modules.map(module => `<article class="module-card"><strong>${escapeHtml(module.name || module.moduleName || module.id || "Module")}</strong><p>${escapeHtml(module.status || module.backend || module.state || "-")}</p></article>`).join("") : `<div class="empty-devices">${text("noData")}</div>`;
  byId("analytics-body").innerHTML = events.length ? events.map(event => `<tr><td>${formatDateTime(event.timestamp || event.createdAt)}</td><td>${escapeHtml(event.cameraName || event.cameraId || event.cameraIp || "-")}</td><td>${escapeHtml(event.label || event.type || event.moduleId || "-")}</td><td>${formatConfidence(event.confidence)}</td></tr>`).join("") : `<tr><td colspan="4">${text("noData")}</td></tr>`;
}

function formatConfidence(value) {
  const number = Number(value || 0);
  return `${Math.round(number * (number <= 1 ? 100 : 1))}%`;
}

function formatDateTime(value) {
  const date = new Date(value);
  return Number.isNaN(date.getTime()) ? escapeHtml(value || "-") : date.toLocaleString();
}

function populateArchiveCameras(cameras) {
  const select = byId("archive-camera");
  const selected = select.value;
  select.innerHTML = `<option value="">${text("allCameras")}</option>` + cameras.map(camera => `<option value="${escapeHtml(camera.id || "")}">${escapeHtml(camera.name || camera.ip)}</option>`).join("");
  if ([...select.options].some(option => option.value === selected)) select.value = selected;
}

function populateControlCameras(cameras) {
  const select = byId("control-camera");
  if (!select) return;
  const selected = select.value;
  select.innerHTML = cameras.map(camera => `<option value="${Number(camera.index)}">${escapeHtml(camera.name || camera.ip)} · ${escapeHtml(camera.ip)}</option>`).join("");
  if ([...select.options].some(option => option.value === selected)) select.value = selected;
}

async function loadArchive() {
  try {
    const camera = byId("archive-camera").value;
    const query = camera ? `?limit=200&camera=${encodeURIComponent(camera)}` : "?limit=200";
    state.archive = await api(`/api/v1/archive${query}`);
    renderArchive();
  } catch (error) {
    byId("archive-list").textContent = error.message;
  }
}

function renderArchive() {
  if (!byId("archive-list")) return;
  const canDownload = Boolean(state.capabilities?.archive?.download);
  byId("archive-list").innerHTML = state.archive.length ? state.archive.map(item => `<article class="archive-item"><button class="archive-play" type="button" data-stream="${escapeHtml(item.streamUrl)}"><strong>${escapeHtml(item.fileName)}</strong><span>${formatDateTime(item.startTime)} &middot; ${(Number(item.sizeBytes || 0) / 1048576).toFixed(1)} MB</span></button>${canDownload ? `<a href="${escapeHtml(item.streamUrl)}?download=1" download title="${text("download")}">&#8681;</a>` : ""}</article>`).join("") : `<div class="empty-devices">${text("noData")}</div>`;
  document.querySelectorAll(".archive-play").forEach(button => button.addEventListener("click", () => {
    document.querySelectorAll(".archive-item").forEach(item => item.classList.remove("active"));
    button.closest(".archive-item").classList.add("active");
    byId("archive-empty").hidden = true;
    byId("archive-player").src = button.dataset.stream;
    byId("archive-player").play().catch(() => {});
  }));
}

const settingLabels = {
  language: "languageSetting", notificationsEnabled: "notificationsEnabled",
  preferredStream: "preferredStream", playerFillMode: "playerFillMode",
  showStatsOverlay: "showStatsOverlay", defaultAutoplay: "defaultAutoplay",
  playerBufferMode: "playerBufferMode", playerRtspTransport: "playerRtspTransport",
  recordingSegmentDuration: "recordingSegmentDuration", analyticsEnabled: "analyticsEnabled",
  sidebarVisible: "sidebarVisible", sidebarToolsExpanded: "sidebarToolsExpanded",
  webSessionTimeoutMinutes: "webSessionTimeoutMinutes", webSecureCookies: "webSecureCookies",
  webServerEnabled: "webServerEnabled", webServerAllowRemote: "webServerAllowRemote",
  webServerBindAddress: "webServerBindAddress", webServerPort: "webServerPort",
  webSocketPort: "webSocketPort"
};

function updateDialogHeader(view) {
  const meta = dialogMeta[view];
  if (!meta) return;
  byId("dialog-title").textContent = text(meta.title);
  byId("dialog-subtitle").textContent = text(meta.subtitle);
  byId("dialog-symbol").textContent = meta.symbol;
}

function openView(view) {
  document.querySelectorAll(".action-button[data-view]").forEach(button => button.classList.toggle("active", button.dataset.view === view));
  if (view === "cameras") { closeDialog(); return; }
  state.currentView = view;
  if (byId("dialog-backdrop").hidden) state.lastFocus = document.activeElement;
  updateDialogHeader(view);
  byId("dialog-backdrop").hidden = false;
  document.querySelectorAll(".dialog-tabs [data-view]").forEach(button => button.classList.toggle("active", button.dataset.view === view));
  document.querySelectorAll(".dialog-page").forEach(page => { page.hidden = page.id !== `page-${view}`; });
  if (view === "health" && state.health) renderHealth(state.health);
  if (view === "analytics") loadAnalytics();
  if (view === "archive") loadArchive();
  if (view === "settings") loadSettings();
  if (view === "users") loadUsers();
  if (view === "logs") loadLogs();
  if (view === "devices") loadDeviceWorkspace();
  byId("dialog-close").focus();
}

function closeDialog() {
  state.currentView = "cameras";
  byId("dialog-backdrop").hidden = true;
  document.querySelectorAll(".action-button[data-view]").forEach(button => button.classList.toggle("active", button.dataset.view === "cameras"));
  byId("archive-player").pause();
  window.clearTimeout(state.deviceTimer);
  if (state.lastFocus?.isConnected) state.lastFocus.focus();
}

function trapDialogFocus(event, container) {
  if (event.key !== "Tab" || !container) return;
  const focusable = [...container.querySelectorAll("button:not([disabled]):not([hidden]), a[href], input:not([disabled]):not([hidden]), select:not([disabled]):not([hidden]), textarea:not([disabled]):not([hidden]), [tabindex]:not([tabindex='-1'])")]
    .filter(element => element.offsetParent !== null);
  if (!focusable.length) return;
  const first = focusable[0];
  const last = focusable[focusable.length - 1];
  if (event.shiftKey && document.activeElement === first) { event.preventDefault(); last.focus(); }
  else if (!event.shiftKey && document.activeElement === last) { event.preventDefault(); first.focus(); }
}

function restoreSidebarState() {
  const collapsed = localStorage.getItem("openipc-web-tools-collapsed") === "1";
  byId("dashboard-sidebar").classList.toggle("tools-collapsed", collapsed);
  byId("sidebar-toggle").setAttribute("aria-expanded", String(!collapsed));
  byId("sidebar-toggle").title = text(collapsed ? "expandActions" : "collapseActions");
}

function toggleSidebarTools() {
  const sidebar = byId("dashboard-sidebar");
  const collapsed = !sidebar.classList.contains("tools-collapsed");
  sidebar.classList.toggle("tools-collapsed", collapsed);
  byId("sidebar-toggle").setAttribute("aria-expanded", String(!collapsed));
  byId("sidebar-toggle").title = text(collapsed ? "expandActions" : "collapseActions");
  localStorage.setItem("openipc-web-tools-collapsed", collapsed ? "1" : "0");
}

function connectSocket() {
  if (!state.server?.webSocketsAvailable) return;
  const protocol = location.protocol === "https:" ? "wss" : "ws";
  state.socket = new WebSocket(state.server.webSocketUrl
    || `${protocol}://${location.hostname}:${state.server.webSocketPort}`);
  state.socket.addEventListener("open", () => {
    byId("connection-state").classList.add("connected");
    state.monitorSignature = "";
    if (state.dashboard) renderMonitorGrid(state.dashboard.cameras || []);
  });
  state.socket.addEventListener("close", () => {
    byId("connection-state").classList.remove("connected");
    stopAllWebRtcPeers();
    state.monitorSignature = "";
    if (state.dashboard && !byId("app-view").hidden) {
      renderMonitorGrid(state.dashboard.cameras || []);
    }
    window.setTimeout(connectSocket, 5000);
  });
  state.socket.addEventListener("message", event => {
    try { handleSocketMessage(JSON.parse(event.data)); } catch (_) {}
  });
}

document.addEventListener("DOMContentLoaded", async () => {
  applyLanguage();
  setLayout(state.layout, false);
  try { state.server = await api("/api/v1/server"); } catch (error) { showLogin(error.message); return; }
  try { showApp(await api("/api/v1/auth/session")); } catch (_) { showLogin(); }
});

byId("login-form").addEventListener("submit", async event => {
  event.preventDefault();
  byId("login-error").textContent = "";
  try {
    const session = await api("/api/v1/auth/login", { method: "POST", body: JSON.stringify({ username: byId("username").value, password: byId("password").value }) });
    showApp(session);
  } catch (error) { byId("login-error").textContent = error.message || text("loginFailed"); }
});

byId("logout").addEventListener("click", async () => { try { await api("/api/v1/auth/logout", { method: "POST" }); } finally { showLogin(); } });
byId("refresh-dashboard").addEventListener("click", loadDashboard);
byId("camera-search").addEventListener("input", () => { if (state.dashboard) renderDeviceList(state.dashboard.cameras || []); });
byId("add-camera").addEventListener("click", () => openCameraForm());
byId("discover-cameras").addEventListener("click", openDiscovery);
byId("sidebar-toggle").addEventListener("click", toggleSidebarTools);
byId("select-empty-cell").addEventListener("click", selectNextEmptyCell);
document.querySelectorAll("[data-page-nav]").forEach(button => button.addEventListener("click", () => {
  setPage(state.page + Number(button.dataset.pageNav), true);
}));
document.querySelectorAll("[data-cycle-toggle]").forEach(button => button.addEventListener("click", togglePageCycling));
document.querySelectorAll("[data-kiosk-toggle]").forEach(button => button.addEventListener("click", toggleKiosk));
document.addEventListener("fullscreenchange", () => { if (!document.fullscreenElement && state.kiosk) setKiosk(false); });
byId("layout-label").addEventListener("click", () => setLayout(state.layout === 1 ? 4 : state.layout === 4 ? 9 : 1));
byId("layout-menu-toggle").addEventListener("click", event => {
  event.stopPropagation();
  const menu = byId("layout-menu");
  menu.hidden = !menu.hidden;
  byId("layout-menu-toggle").setAttribute("aria-expanded", String(!menu.hidden));
});
document.querySelectorAll("#layout-menu [data-layout]").forEach(button => button.addEventListener("click", () => setLayout(Number(button.dataset.layout))));
document.addEventListener("click", event => { if (!event.target.closest(".layout-picker")) { byId("layout-menu").hidden = true; byId("layout-menu-toggle").setAttribute("aria-expanded", "false"); } });

document.querySelectorAll("[data-view]").forEach(button => button.addEventListener("click", () => openView(button.dataset.view)));
byId("dialog-close").addEventListener("click", closeDialog);
byId("dialog-backdrop").addEventListener("click", event => { if (event.target === byId("dialog-backdrop")) closeDialog(); });
byId("camera-dialog-close").addEventListener("click", closeCameraDialog);
byId("camera-form-cancel").addEventListener("click", closeCameraDialog);
byId("camera-backdrop").addEventListener("click", event => { if (event.target === byId("camera-backdrop")) closeCameraDialog(); });
byId("camera-form").addEventListener("submit", saveCamera);
byId("discovery-start").addEventListener("click", toggleDiscovery);
byId("discovery-clear").addEventListener("click", clearDiscovery);
document.querySelectorAll("[data-camera-page]").forEach(button => button.addEventListener("click", () => {
  if (button.dataset.cameraPage === "manual") {
    byId("camera-dialog-title").textContent = text(byId("camera-edit-index").value === "" ? "addCamera" : "editCamera");
  }
  setCameraDialogPage(button.dataset.cameraPage);
}));
document.addEventListener("keydown", event => {
  if (!byId("camera-backdrop").hidden) {
    if (event.key === "Escape") closeCameraDialog();
    else trapDialogFocus(event, document.querySelector(".camera-manager-dialog"));
    return;
  }
  if (!byId("dialog-backdrop").hidden) {
    if (event.key === "Escape") closeDialog();
    else trapDialogFocus(event, document.querySelector(".workspace-dialog"));
    return;
  }
  if (event.key === "Escape" && state.openControlsCell !== null) {
    setCellControlsOpen(null);
    return;
  }
  if (event.altKey && !event.ctrlKey && !event.metaKey
      && ["1", "2", "3"].includes(event.key)) {
    event.preventDefault();
    setLayout({ "1": 1, "2": 4, "3": 9 }[event.key]);
  }
});

byId("run-health").addEventListener("click", async () => {
  try {
    await api("/api/v1/health/run", { method: "POST", body: JSON.stringify({ profile: "quick" }) });
    showToast(text("checkStarted"));
    window.setTimeout(loadDashboard, 600);
  } catch (error) { showToast(error.message, true); }
});
byId("refresh-archive").addEventListener("click", loadArchive);
byId("archive-camera").addEventListener("change", loadArchive);
byId("settings-form").addEventListener("submit", saveSettings);
byId("configuration-import").addEventListener("change", importConfiguration);
byId("user-create-form").addEventListener("submit", createUser);
byId("password-change-form").addEventListener("submit", changePassword);
byId("refresh-logs").addEventListener("click", () => loadLogs(true));
byId("log-level").addEventListener("change", () => loadLogs(true));
byId("log-search").addEventListener("search", () => loadLogs(true));
byId("load-more-logs").addEventListener("click", () => { if (state.logCursor >= 0) loadLogs(false); });
byId("clear-logs").addEventListener("click", async () => {
  try { await api("/api/v1/logs/clear", { method: "POST", body: "{}" }); await loadLogs(true); }
  catch (error) { showToast(error.message, true); }
});
document.querySelectorAll("[data-device-operation]").forEach(button => button.addEventListener("click", () => startDeviceOperation(button.dataset.deviceOperation)));
byId("camex-form").addEventListener("submit", previewCamex);
byId("majestic-preview").addEventListener("click", previewMajesticChanges);
byId("majestic-apply").addEventListener("click", applyMajesticChanges);
document.querySelectorAll("#language, #language-login").forEach(button => button.addEventListener("click", () => {
  state.language = state.language === "ru" ? "en" : "ru";
  localStorage.setItem("openipc-language", state.language);
  applyLanguage();
}));

window.setInterval(() => { byId("status-clock").textContent = new Date().toLocaleTimeString(); }, 1000);
