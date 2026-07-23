"use strict";

function renderDeviceList(cameras) {
  const query = byId("camera-search").value.trim().toLowerCase();
  const visible = cameras.filter(camera => `${camera.name} ${camera.ip} ${camera.group}`.toLowerCase().includes(query));
  const grouped = cameras.some(camera => String(camera.group || "").trim());
  document.querySelector(".device-group-heading strong").textContent = text(grouped ? "allGroups" : "ungrouped");
  byId("device-group-count").textContent = visible.length;
  byId("device-list").innerHTML = visible.length ? visible.map(camera => {
    const temperature = numericTemperature(camera.health);
    const assigned = state.assignments.includes(cameraKey(camera));
    const online = isOnline(camera.status);
    const hot = temperature != null && temperature >= 80;
    const managementActions = canManageCameras()
      ? `<button type="button" data-edit="${camera.index}" title="${text("editCamera")}" aria-label="${text("editCamera")}">&#x270E;</button><button class="danger-action" type="button" data-delete="${camera.index}" title="${text("deleteCamera")}" aria-label="${text("deleteCamera")}">&#x2715;</button>`
      : "";
    return `<article class="device-card ${online ? "online" : ""} ${assigned ? "assigned" : ""} ${hot ? "attention" : ""}" data-camera-card="${camera.index}" tabindex="0">
      <div class="device-card-header"><strong class="device-name"><span class="dot"></span>${escapeHtml(camera.name || camera.ip)}</strong><span class="device-state">${escapeHtml(camera.status || "-")}</span></div>
      <div class="device-card-meta"><span>IP ${escapeHtml(camera.ip)}</span><span>RTSP ${escapeHtml(camera.rtspPort || 554)}</span><span class="device-temperature ${hot ? "hot" : ""}">${temperature == null ? text("noTemperature") : `${Math.round(temperature)} °C`}</span>
        <span class="device-card-actions"><button type="button" data-assign="${camera.index}" title="${text("assignCamera")}" aria-label="${text("assignCamera")}">+</button><button type="button" data-open="${escapeHtml(camera.webUiUrl)}" title="${text("openCamera")}" aria-label="${text("openCamera")}">&#x2197;</button>${managementActions}</span>
      </div>
    </article>`;
  }).join("") : `<div class="empty-devices">${text("noData")}</div>`;

  document.querySelectorAll("[data-camera-card]").forEach(card => {
    const assign = event => {
      if (event?.target?.closest("button")) return;
      const camera = cameras.find(item => item.index === Number(card.dataset.cameraCard));
      if (camera) assignCamera(camera);
    };
    card.addEventListener("click", assign);
    card.addEventListener("keydown", event => { if (event.key === "Enter" || event.key === " ") { event.preventDefault(); assign(event); } });
  });
  document.querySelectorAll("[data-assign]").forEach(button => button.addEventListener("click", event => {
    event.stopPropagation();
    const camera = cameras.find(item => item.index === Number(button.dataset.assign));
    if (camera) assignCamera(camera);
  }));
  document.querySelectorAll("[data-open]").forEach(button => button.addEventListener("click", event => {
    event.stopPropagation();
    window.open(button.dataset.open, "_blank", "noopener");
  }));
  document.querySelectorAll("[data-edit]").forEach(button => button.addEventListener("click", event => {
    event.stopPropagation();
    const camera = cameras.find(item => item.index === Number(button.dataset.edit));
    if (camera) openCameraForm(camera);
  }));
  document.querySelectorAll("[data-delete]").forEach(button => button.addEventListener("click", async event => {
    event.stopPropagation();
    const camera = cameras.find(item => item.index === Number(button.dataset.delete));
    if (camera) await deleteCamera(camera);
  }));
}

function setCameraDialogPage(page) {
  state.cameraDialogPage = page === "discovery" ? "discovery" : "manual";
  byId("camera-form").hidden = state.cameraDialogPage !== "manual";
  byId("camera-page-discovery").hidden = state.cameraDialogPage !== "discovery";
  document.querySelectorAll("[data-camera-page]").forEach(button => button.classList.toggle("active", button.dataset.cameraPage === state.cameraDialogPage));
  window.clearTimeout(state.discoveryTimer);
  if (state.cameraDialogPage === "discovery") {
    byId("camera-dialog-title").textContent = text("discoverCameras");
    loadDiscovery();
  }
}

function openCameraForm(camera = null) {
  if (!canManageCameras()) return;
  state.lastFocus = document.activeElement;
  byId("camera-backdrop").hidden = false;
  byId("camera-edit-index").value = camera ? String(camera.index) : "";
  byId("camera-edit-id").value = camera?.id || "";
  byId("camera-name").value = camera?.name || "";
  byId("camera-host").value = camera?.ip || "";
  byId("camera-profile").value = camera ? (camera.openIpc ? "openipc" : "generic") : "openipc";
  byId("camera-rtsp-port").value = String(camera?.rtspPort || 554);
  byId("camera-onvif-port").value = String(camera?.httpPort || 80);
  byId("camera-hd-path").value = "";
  byId("camera-sd-path").value = "";
  byId("camera-login").value = "";
  byId("camera-password").value = "";
  byId("clear-camera-credentials").checked = false;
  byId("clear-camera-credentials-row").hidden = !camera;
  byId("camera-form-error").textContent = "";
  byId("camera-dialog-title").textContent = text(camera ? "editCamera" : "addCamera");
  setCameraDialogPage("manual");
  byId("camera-host").focus();
}

function openDiscovery() {
  if (!canManageCameras()) return;
  state.lastFocus = document.activeElement;
  byId("camera-backdrop").hidden = false;
  byId("camera-dialog-title").textContent = text("discoverCameras");
  byId("camera-form-error").textContent = "";
  setCameraDialogPage("discovery");
  byId("discovery-start").focus();
}

function closeCameraDialog() {
  window.clearTimeout(state.discoveryTimer);
  state.discoveryTimer = null;
  const backdrop = byId("camera-backdrop");
  if (backdrop) backdrop.hidden = true;
  if (state.lastFocus?.isConnected) state.lastFocus.focus();
}

async function saveCamera(event) {
  event.preventDefault();
  if (!canManageCameras()) return;
  const editIndex = byId("camera-edit-index").value;
  const editing = editIndex !== "";
  const payload = {
    name: byId("camera-name").value.trim(),
    ip: byId("camera-host").value.trim(),
    profile: byId("camera-profile").value,
    rtspPort: Number(byId("camera-rtsp-port").value),
    onvifPort: Number(byId("camera-onvif-port").value)
  };
  const hdPath = byId("camera-hd-path").value.trim();
  const sdPath = byId("camera-sd-path").value.trim();
  if (hdPath) payload.hdPath = hdPath;
  if (sdPath) payload.sdPath = sdPath;
  if (editing) payload.id = byId("camera-edit-id").value;

  const login = byId("camera-login").value.trim();
  const password = byId("camera-password").value;
  if (!editing || login) payload.login = login;
  if (!editing || password) payload.password = password;
  if (editing && byId("clear-camera-credentials").checked) {
    payload.login = "";
    payload.password = "";
  }

  const submit = byId("camera-form-submit");
  submit.disabled = true;
  byId("camera-form-error").textContent = "";
  try {
    const path = editing ? `/api/v1/cameras/${encodeURIComponent(editIndex)}/update` : "/api/v1/cameras";
    await api(path, { method: "POST", body: JSON.stringify(payload) });
    closeCameraDialog();
    await loadDashboard();
    showToast(text(editing ? "cameraUpdated" : "cameraAdded"));
  } catch (error) {
    byId("camera-form-error").textContent = error.message;
  } finally {
    submit.disabled = false;
  }
}

async function deleteCamera(camera) {
  if (!canManageCameras() || !window.confirm(text("deleteConfirm"))) return;
  try {
    await api(`/api/v1/cameras/${encodeURIComponent(camera.index)}/delete`, {
      method: "POST", body: JSON.stringify({ id: camera.id })
    });
    await loadDashboard();
    showToast(text("cameraDeleted"));
  } catch (error) { showToast(error.message, true); }
}

function scheduleDiscoveryRefresh() {
  window.clearTimeout(state.discoveryTimer);
  if (!byId("camera-backdrop").hidden && state.cameraDialogPage === "discovery") {
    state.discoveryTimer = window.setTimeout(loadDiscovery, state.discovery?.running ? 800 : 2500);
  }
}

async function loadDiscovery() {
  window.clearTimeout(state.discoveryTimer);
  if (!canManageCameras() || byId("camera-backdrop").hidden || state.cameraDialogPage !== "discovery") return;
  try {
    state.discovery = await api("/api/v1/discovery");
    renderDiscovery(state.discovery);
  } catch (error) {
    byId("discovery-summary").textContent = error.message;
  } finally {
    scheduleDiscoveryRefresh();
  }
}

function renderDiscovery(data) {
  if (!data) return;
  const interfaceSelect = byId("discovery-interface");
  const selectedInterface = interfaceSelect.value;
  const interfaces = Array.isArray(data.interfaces) ? data.interfaces : [];
  interfaceSelect.innerHTML = `<option value="">${text("allInterfaces")}</option>` + interfaces.map(item =>
    `<option value="${escapeHtml(item.id)}">${escapeHtml(item.name || item.id)} · ${escapeHtml(item.ip || "")}</option>`
  ).join("");
  if ([...interfaceSelect.options].some(option => option.value === selectedInterface)) interfaceSelect.value = selectedInterface;

  byId("discovery-summary").textContent = data.summary || text("noDiscoveryResults");
  byId("discovery-phase").textContent = data.phase || "";
  byId("discovery-progress").value = Number(data.progress || 0);
  byId("discovery-start").textContent = text(data.running ? "stopDiscovery" : "startDiscovery");
  byId("discovery-clear").disabled = Boolean(data.running) || !(data.cameras || []).length;

  const cameras = data.cameras || [];
  byId("discovery-list").innerHTML = cameras.length ? cameras.map(camera => {
    const added = camera.alreadyAdded || camera.validationStatus === "added";
    const details = [camera.manufacturer, camera.methods, camera.confidence ? `${camera.confidence}%` : ""].filter(Boolean).join(" · ");
    return `<article class="discovery-card ${camera.openIpc ? "openipc" : ""} ${added ? "added" : ""}">
      <div><h3>${escapeHtml(camera.name || camera.ip)} · ${escapeHtml(camera.ip)}</h3><p>${escapeHtml(details || camera.evidence || "-")}</p><p>${escapeHtml(camera.validationMessage || camera.evidence || "")}</p></div>
      <button type="button" data-discovery-add="${camera.index}" ${added ? "disabled" : ""}>${text(added ? "alreadyAdded" : "addDiscovered")}</button>
    </article>`;
  }).join("") : `<div class="empty-devices">${text("noDiscoveryResults")}</div>`;
  document.querySelectorAll("[data-discovery-add]").forEach(button => button.addEventListener("click", () => addDiscoveredCamera(Number(button.dataset.discoveryAdd))));
}

async function toggleDiscovery() {
  try {
    if (state.discovery?.running) {
      state.discovery = await api("/api/v1/discovery/stop", { method: "POST", body: "{}" });
      showToast(text("discoveryStopped"));
    } else {
      state.discovery = await api("/api/v1/discovery/start", {
        method: "POST",
        body: JSON.stringify({ interface: byId("discovery-interface").value, deepScan: byId("discovery-deep").checked })
      });
      showToast(text("discoveryStarted"));
    }
    renderDiscovery(state.discovery);
  } catch (error) { showToast(error.message, true); }
  scheduleDiscoveryRefresh();
}

async function clearDiscovery() {
  try {
    state.discovery = await api("/api/v1/discovery/clear", { method: "POST", body: "{}" });
    renderDiscovery(state.discovery);
  } catch (error) { showToast(error.message, true); }
}

async function addDiscoveredCamera(index) {
  try {
    await api("/api/v1/discovery/add", {
      method: "POST",
      body: JSON.stringify({
        index,
        profile: byId("discovery-profile").value,
        login: byId("discovery-login").value.trim(),
        password: byId("discovery-password").value
      })
    });
    byId("discovery-password").value = "";
    await Promise.all([loadDashboard(), loadDiscovery()]);
    showToast(text("cameraAdded"));
  } catch (error) { showToast(error.message, true); }
}

function assignCamera(camera) {
  setCellControlsOpen(null);
  state.assignments[state.activeCell] = cameraKey(camera);
  persistWorkspace();
  renderMonitorGrid(state.dashboard?.cameras || []);
  renderDeviceList(state.dashboard?.cameras || []);
  updatePreviewStats();
  const nextEmpty = state.assignments.findIndex((value, index) => index > state.activeCell && !value);
  if (nextEmpty >= 0) state.activeCell = nextEmpty;
}

function updatePreviewStats() {
  const active = state.assignments.slice(0, state.layout).filter(Boolean).length;
  byId("preview-state").textContent = `${active}/${state.layout}`;
  byId("preview-detail").textContent = text("previewReady");
}

function setLayout(layout, rerender = true) {
  if (!validLayouts.includes(Number(layout))) return;
  setCellControlsOpen(null);
  state.layout = Number(layout);
  while (state.assignments.length < state.layout) state.assignments.push(null);
  state.assignments = state.assignments.slice(0, state.layout);
  state.activeCell = Math.min(state.activeCell, state.layout - 1);
  persistWorkspace();
  document.querySelectorAll("[data-layout]").forEach(button => button.classList.toggle("active", Number(button.dataset.layout) === state.layout));
  if (rerender && state.dashboard) {
    renderMonitorGrid(state.dashboard.cameras || []);
    renderDeviceList(state.dashboard.cameras || []);
    updatePreviewStats();
  }
  byId("layout-menu").hidden = true;
  byId("layout-menu-toggle").setAttribute("aria-expanded", "false");
}
