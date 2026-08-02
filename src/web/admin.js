"use strict";

async function loadSettings() {
  try { renderSettings(await api("/api/v1/settings")); }
  catch (error) { byId("settings-error").textContent = error.message; }
}

function renderSettings(data) {
  state.settings = data;
  const values = data.values || {};
  const groups = new Map();
  (data.schema || []).forEach(definition => {
    if (!groups.has(definition.group)) groups.set(definition.group, []);
    groups.get(definition.group).push(definition);
  });
  byId("settings-fields").innerHTML = [...groups.entries()].map(([group, definitions]) =>
    `<section class="settings-group"><h3>${escapeHtml(text(group) || group)}</h3>${definitions.map(definition => {
      const value = values[definition.key];
      const disabled = definition.readOnly ? " disabled" : "";
      let control = "";
      if (definition.type === "boolean") {
        control = `<input data-setting="${escapeHtml(definition.key)}" data-setting-type="boolean" type="checkbox" ${value ? "checked" : ""}${disabled}>`;
      } else if (definition.type === "select") {
        control = `<select data-setting="${escapeHtml(definition.key)}" data-setting-type="select"${disabled}>${(definition.options || []).map(option => `<option value="${escapeHtml(option)}" ${String(option) === String(value) ? "selected" : ""}>${escapeHtml(option)}</option>`).join("")}</select>`;
      } else {
        const attributes = definition.type === "integer" ? ` type="number" min="${definition.minimum ?? ""}" max="${definition.maximum ?? ""}" step="${definition.step || 1}"` : ` type="text"`;
        control = `<input data-setting="${escapeHtml(definition.key)}" data-setting-type="${escapeHtml(definition.type)}"${attributes} value="${escapeHtml(value ?? "")}"${disabled}>`;
      }
      return `<label class="setting-field ${definition.readOnly ? "read-only" : ""}"><span>${escapeHtml(text(settingLabels[definition.key] || definition.key))}</span>${control}</label>`;
    }).join("")}</section>`).join("");
}

async function saveSettings(event) {
  event.preventDefault();
  const patch = {};
  byId("settings-fields").querySelectorAll("[data-setting]:not(:disabled)").forEach(control => {
    const type = control.dataset.settingType;
    patch[control.dataset.setting] = type === "boolean" ? control.checked
      : (type === "integer" || ["playerFillMode", "playerBufferMode"].includes(control.dataset.setting)
        ? Number(control.value) : control.value);
  });
  byId("settings-error").textContent = "";
  try {
    const result = await api("/api/v1/settings", { method: "POST", body: JSON.stringify({ settings: patch }) });
    state.settings.values = result.values;
    renderSettings(state.settings);
    showToast(text("settingsSaved"));
  } catch (error) { byId("settings-error").textContent = error.message; }
}

async function loadUsers() {
  try { renderUsers(await api("/api/v1/users")); }
  catch (error) { byId("users-body").innerHTML = `<tr><td colspan="4">${escapeHtml(error.message)}</td></tr>`; }
}

function renderUsers(data) {
  state.userAdmin = data;
  const catalog = data.permissions || [];
  const cameras = state.dashboard?.cameras || [];
  const scopeOptions = selected => cameras.map(camera => {
    const key = cameraKey(camera);
    return `<option value="${escapeHtml(key)}" ${selected.includes(key) ? "selected" : ""}>${escapeHtml(camera.name || camera.ip)}</option>`;
  }).join("");
  byId("new-user-camera-scopes").innerHTML = scopeOptions([]);
  byId("permission-editor").innerHTML = catalog.map(permission =>
    `<label><input type="checkbox" data-new-permission="${Number(permission.value)}" ${[1, 2, 4, 64].includes(Number(permission.value)) ? "checked" : ""}><span>${escapeHtml(permission.id)}</span></label>`
  ).join("");
  const users = data.users || [];
  byId("users-body").innerHTML = users.length ? users.map(user => {
    const scopes = Array.isArray(user.cameraScopes) ? user.cameraScopes : [];
    return `<tr><td>${escapeHtml(user.username)}</td><td>${escapeHtml(text(user.role === "admin" ? "administrator" : "operator"))}</td><td>${Number(user.permissions)}${user.role === "admin" ? "" : `<label class="table-scope"><span>${text("cameraScope")}: ${scopes.length ? text("selectedCamerasScope") : text("allCamerasScope")}</span><select multiple size="3" data-scopes-user="${escapeHtml(user.username)}">${scopeOptions(scopes)}</select></label>`}</td><td><div class="inline-actions">${user.role === "admin" ? "" : `<input type="number" min="0" max="255" value="${Number(user.permissions)}" data-permissions-user="${escapeHtml(user.username)}"><button type="button" data-save-permissions="${escapeHtml(user.username)}">${text("saveSettings")}</button>`}${user.username === state.session?.username ? "" : `<button class="danger" type="button" data-delete-user="${escapeHtml(user.username)}">${text("delete")}</button>`}</div></td></tr>`;
  }).join("") : `<tr><td colspan="4">${text("noData")}</td></tr>`;
  const sessions = data.sessions || [];
  byId("sessions-body").innerHTML = sessions.length ? sessions.map(session => `<tr><td>${escapeHtml(session.username)}${session.current ? " · current" : ""}</td><td>${escapeHtml(session.origin || session.peerAddress || "—")}</td><td>${formatDateTime(session.lastSeenAt)}</td><td>${formatDateTime(session.expiresAt)}<br><small>${formatDateTime(session.absoluteExpiresAt)}</small></td><td>${session.current ? "" : `<button type="button" data-revoke-session="${escapeHtml(session.id)}">${text("revoke")}</button>`}</td></tr>`).join("") : `<tr><td colspan="5">${text("noData")}</td></tr>`;
  document.querySelectorAll("[data-save-permissions]").forEach(button => button.addEventListener("click", () => saveUserPermissions(button.dataset.savePermissions)));
  document.querySelectorAll("[data-delete-user]").forEach(button => button.addEventListener("click", () => deleteUser(button.dataset.deleteUser)));
  document.querySelectorAll("[data-revoke-session]").forEach(button => button.addEventListener("click", () => revokeSession(button.dataset.revokeSession)));
}

async function createUser(event) {
  event.preventDefault();
  const role = byId("new-user-role").value;
  let permissions = 0;
  document.querySelectorAll("[data-new-permission]:checked").forEach(input => { permissions |= Number(input.dataset.newPermission); });
  try {
    await api("/api/v1/users/create", { method: "POST", body: JSON.stringify({
      username: byId("new-user-name").value.trim(), password: byId("new-user-password").value,
      role, permissions,
      cameraScopes: [...byId("new-user-camera-scopes").selectedOptions].map(option => option.value)
    }) });
    byId("new-user-name").value = ""; byId("new-user-password").value = "";
    await loadUsers();
  } catch (error) { showToast(error.message, true); }
}

async function saveUserPermissions(username) {
  const input = document.querySelector(`[data-permissions-user="${CSS.escape(username)}"]`);
  const scopes = document.querySelector(`[data-scopes-user="${CSS.escape(username)}"]`);
  try {
    await api("/api/v1/users/permissions", { method: "POST", body: JSON.stringify({
      username, permissions: Number(input.value),
      cameraScopes: scopes ? [...scopes.selectedOptions].map(option => option.value) : []
    }) });
    await loadUsers();
  } catch (error) { showToast(error.message, true); }
}

async function deleteUser(username) {
  if (!window.confirm(`${text("delete")}: ${username}?`)) return;
  try {
    await api("/api/v1/users/delete", { method: "POST", body: JSON.stringify({ username }) });
    await loadUsers();
  } catch (error) { showToast(error.message, true); }
}

async function revokeSession(id) {
  const reason = window.prompt(text("revokeReason"), "administrator");
  if (reason === null) return;
  try {
    await api("/api/v1/sessions/revoke", { method: "POST", body: JSON.stringify({ id, reason: reason.trim() }) });
    await loadUsers();
  } catch (error) { showToast(error.message, true); }
}

async function changePassword(event) {
  event.preventDefault();
  try {
    await api("/api/v1/users/password", { method: "POST", body: JSON.stringify({
      username: state.session.username, oldPassword: byId("old-password").value,
      newPassword: byId("new-password").value
    }) });
    showLogin(text("passwordChanged"));
  } catch (error) { showToast(error.message, true); }
}

async function loadLogs(reset = true) {
  if (reset) { state.logs = []; state.logCursor = 0; }
  const params = new URLSearchParams({ limit: "100", cursor: String(state.logCursor),
    level: byId("log-level").value, search: byId("log-search").value.trim() });
  try {
    const data = await api(`/api/v1/logs?${params}`);
    state.logs.push(...(data.items || []));
    state.logCursor = Number(data.nextCursor);
    renderLogs();
    await loadDiagnostics();
  } catch (error) { byId("logs-body").innerHTML = `<tr><td colspan="3">${escapeHtml(error.message)}</td></tr>`; }
}

function renderLogs() {
  byId("logs-body").innerHTML = state.logs.length ? state.logs.map(item => `<tr><td>${formatDateTime(item.timestamp)}</td><td>${escapeHtml(item.level)}</td><td>${escapeHtml(item.message)}</td></tr>`).join("") : `<tr><td colspan="3">${text("noData")}</td></tr>`;
  byId("load-more-logs").hidden = state.logCursor < 0;
  const params = new URLSearchParams({ limit: "200", download: "1", level: byId("log-level").value,
    search: byId("log-search").value.trim() });
  byId("download-logs").href = `/api/v1/logs?${params}`;
}

async function loadDiagnostics() {
  try { renderDiagnostics(await api("/api/v1/diagnostics")); }
  catch (_) {}
}

function renderDiagnostics(data) {
  state.diagnostics = data;
  const process = data.process || {};
  const server = data.server || {};
  const cameras = data.cameras || {};
  byId("diagnostics-summary").innerHTML = [
    ["CPU", `${Number(process.cpuPercent || 0).toFixed(1)}%`],
    ["RAM", `${Number(process.memoryMiB || 0).toFixed(1)} MiB`],
    ["Sessions", server.activeSessions || 0], ["Cameras", `${cameras.online || 0}/${cameras.total || 0}`]
  ].map(([label, value]) => `<div><span>${escapeHtml(label)}</span><strong>${escapeHtml(value)}</strong></div>`).join("");
}

function loadDeviceWorkspace() {
  populateControlCameras(state.dashboard?.cameras || []);
  byId("device-operation-state").textContent = "";
  state.majesticPreviewId = "";
  byId("majestic-apply").disabled = true;
}

async function startDeviceOperation(operation) {
  const cameraIndex = Number(byId("control-camera").value);
  const normalized = { timeSettings: "time", cameraLogs: "logs" }[operation] || operation;
  const mutation = ["sync-time", "reboot", "github-update"].includes(normalized);
  if (mutation && !window.confirm(`${text("confirmDeviceAction")}\n${text(normalized === "sync-time" ? "syncTime" : normalized === "reboot" ? "rebootDevice" : "startUpdate")}`)) return;
  window.clearTimeout(state.deviceTimer);
  byId("device-operation-state").textContent = `${text("checksRunning")}…`;
  try {
    const idempotencyKey = mutation
      ? (globalThis.crypto?.randomUUID?.() || `${Date.now()}-${Math.random().toString(16).slice(2)}`)
      : "";
    const job = await api("/api/v1/devices/operation", {
      method: "POST",
      headers: mutation ? { "Idempotency-Key": idempotencyKey } : {},
      body: JSON.stringify({ cameraIndex, operation: normalized,
        ...(mutation ? { confirm: normalized } : {}) })
    });
    await pollDeviceOperation(job.id);
  } catch (error) {
    byId("device-operation-state").textContent = error.message;
    showToast(error.message, true);
  }
}

async function importConfiguration(event) {
  const input = event.currentTarget;
  const file = input.files?.[0];
  input.value = "";
  if (!file) return;
  const status = byId("configuration-import-state");
  if (file.size < 2 || file.size > 1024 * 1024) {
    status.textContent = text("invalidConfigFile");
    return;
  }
  if (!window.confirm(text("importConfigConfirm"))) return;
  try {
    const raw = await file.text();
    JSON.parse(raw);
    const result = await api("/api/v1/configuration/import", { method: "POST", body: raw });
    status.textContent = `${text("configImported")}: +${result.added || 0}, ~${result.updated || 0}`;
    showToast(text("configImported"));
    await Promise.all([loadSettings(), loadDashboard()]);
  } catch (error) {
    status.textContent = error.message || text("invalidConfigFile");
    showToast(status.textContent, true);
  }
}

async function pollDeviceOperation(id, attempt = 0) {
  try {
    const job = await api(`/api/v1/devices/operations/${encodeURIComponent(id)}`);
    byId("device-operation-state").textContent = job.status;
    if (job.status === "pending" && attempt < 60) {
      state.deviceTimer = window.setTimeout(() => pollDeviceOperation(id, attempt + 1), 500);
      return;
    }
    byId("device-output").textContent = job.status === "succeeded"
      ? JSON.stringify(job.data, null, 2) : (job.error || text("operationFailed"));
    if (job.status === "succeeded" && job.operation === "majestic" && job.data?.config) {
      state.majesticOperationId = id;
      state.majesticPreviewId = "";
      byId("majestic-editor").hidden = false;
      byId("majestic-json").value = JSON.stringify(job.data.config, null, 2);
      byId("majestic-diff").textContent = "";
      byId("majestic-apply").disabled = true;
    }
  } catch (error) { byId("device-operation-state").textContent = error.message; }
}

async function previewMajesticChanges() {
  try {
    const edited = JSON.parse(byId("majestic-json").value);
    const result = await api("/api/v1/devices/majestic/preview", { method: "POST",
      body: JSON.stringify({ operationId: state.majesticOperationId, edited }) });
    state.majesticPreviewId = result.previewId;
    byId("majestic-diff").textContent = JSON.stringify(result.changes || [], null, 2);
    byId("majestic-apply").disabled = false;
  } catch (error) {
    state.majesticPreviewId = "";
    byId("majestic-apply").disabled = true;
    byId("majestic-diff").textContent = error.message;
  }
}

async function applyMajesticChanges() {
  const previewId = state.majesticPreviewId;
  if (!previewId || !window.confirm(text("confirmApplyChanges"))) return;
  const idempotencyKey = globalThis.crypto?.randomUUID?.()
    || `${Date.now()}-${Math.random().toString(16).slice(2)}`;
  try {
    const job = await api("/api/v1/devices/majestic/apply", { method: "POST",
      headers: { "Idempotency-Key": idempotencyKey },
      body: JSON.stringify({ previewId, confirm: previewId }) });
    state.majesticPreviewId = "";
    byId("majestic-apply").disabled = true;
    await pollDeviceOperation(job.id);
  } catch (error) { showToast(error.message, true); }
}

async function previewCamex(event) {
  event.preventDefault();
  try {
    const result = await api("/api/v1/camex/preview", { method: "POST", body: JSON.stringify({
      serverHost: byId("camex-host").value.trim(), port: Number(byId("camex-port").value),
      clientId: byId("camex-client-id").value.trim()
    }) });
    byId("camex-output").textContent = [result.serverCommand, result.clientCommand, result.serverConfig].join("\n\n");
  } catch (error) { byId("camex-output").textContent = error.message; }
}

const dialogMeta = {
  health: { title: "health", subtitle: "healthSubtitle", symbol: "◇" },
  analytics: { title: "analytics", subtitle: "analyticsSubtitle", symbol: "⌕" },
  archive: { title: "archive", subtitle: "archiveSubtitle", symbol: "▣" },
  settings: { title: "settings", subtitle: "settingsHint", symbol: "⚙" },
  users: { title: "users", subtitle: "permissions", symbol: "☺" },
  logs: { title: "logs", subtitle: "diagnosticBundle", symbol: "☷" },
  devices: { title: "controlCenter", subtitle: "selectDeviceAction", symbol: "◆" }
};
