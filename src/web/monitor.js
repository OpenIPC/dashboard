"use strict";

function renderDashboard(data) {
  state.dashboard = data;
  const summary = data.summary || {};
  const cameras = data.cameras || [];
  normalizeAssignments(cameras);
  byId("sidebar-total").textContent = summary.total || 0;
  byId("sidebar-online").textContent = summary.online || 0;
  byId("sidebar-offline").textContent = summary.offline || 0;
  byId("device-total").textContent = summary.total || 0;
  byId("updated-at").textContent = data.generatedAt ? new Date(data.generatedAt).toLocaleTimeString() : "-";
  renderMonitorGrid(cameras);
  renderDeviceList(cameras);
  updatePreviewStats();
  populateArchiveCameras(cameras);
  populateControlCameras(cameras);
  if (data.health) renderHealth(data.health);
}

function findCameraByKey(cameras, key) {
  return cameras.find(camera => cameraKey(camera) === String(key));
}

function renderMonitorGrid(cameras) {
  const grid = byId("monitor-grid");
  const signature = JSON.stringify({
    layout: state.layout,
    activeCell: state.activeCell,
    assignments: state.assignments.slice(0, state.layout),
    cameras: cameras.map(camera => [cameraKey(camera), camera.index, camera.ip,
      camera.name, camera.status, camera.recording, camera.previewStreamUrl, camera.previewUrl])
  });
  if (signature === state.monitorSignature) return;
  state.monitorSignature = signature;
  stopAllWebRtcPeers();
  grid.dataset.layout = String(state.layout);
  grid.innerHTML = Array.from({ length: state.layout }, (_, cellIndex) => {
    const camera = findCameraByKey(cameras, state.assignments[cellIndex]);
    const active = cellIndex === state.activeCell ? " active" : "";
    if (!camera) {
      return `<article class="stream-cell${active}" data-cell="${cellIndex}" tabindex="0">
        <div class="empty-cell"><span class="plus">+</span><strong>${text("emptyCell")}</strong><span>${text("emptyCellHint")}</span></div>
      </article>`;
    }
    const online = isOnline(camera.status);
    const canAudio = Boolean(state.capabilities?.monitor?.audio);
    const canPtz = Boolean(state.capabilities?.monitor?.ptz);
    const controlsOpen = cellIndex === state.openControlsCell;
    const controlsTitle = text(controlsOpen ? "hideCameraControls" : "showCameraControls");
    return `<article class="stream-cell${active}${controlsOpen ? " controls-open" : ""}${camera.recording ? " recording" : ""}" data-cell="${cellIndex}" data-camera-index="${camera.index}" tabindex="0">
      <div class="stream-fallback"><span class="camera-symbol">O</span><strong>${escapeHtml(camera.ip)}</strong><span>${text("streamUnavailable")}</span></div>
      <video data-webrtc-video autoplay muted playsinline hidden></video>
      <img data-stream-image data-preview-url="${escapeHtml(camera.previewStreamUrl || camera.previewUrl)}" alt="${escapeHtml(camera.name || camera.ip)}" hidden>
      <span class="stream-overlay stream-status ${online ? "online" : ""}"><span class="dot"></span>${escapeHtml(camera.status || (online ? "Online" : "Offline"))}</span>
      <span class="stream-overlay stream-meta"><span data-stream-transport>WebRTC</span> &middot; ${escapeHtml(camera.ip)} &middot; RTSP ${escapeHtml(camera.rtspPort || 554)}</span>
      <span class="stream-overlay stream-name">${escapeHtml(camera.name || camera.ip)}</span>
      <button class="cell-controls-toggle" type="button" data-controls-toggle="${cellIndex}" aria-controls="cell-controls-${cellIndex}" aria-expanded="${controlsOpen}" title="${controlsTitle}" aria-label="${controlsTitle}"></button>
      <div class="cell-controls" id="cell-controls-${cellIndex}" role="toolbar" aria-label="${text("actions")}">
        <button type="button" data-record-camera="${camera.index}" class="${camera.recording ? "recording-active" : ""}" title="${text(camera.recording ? "stopRecording" : "startRecording")}" aria-label="${text(camera.recording ? "stopRecording" : "startRecording")}">&#9679;</button>
        <button type="button" data-snapshot-camera="${camera.index}" title="${text("snapshot")}" aria-label="${text("snapshot")}">&#9635;</button>
        ${canAudio ? `<button type="button" data-audio-toggle title="${text("unmute")}" aria-label="${text("unmute")}">&#128263;</button><input type="range" data-audio-volume min="0" max="1" step="0.05" value="1" title="${text("mute")}">` : ""}
        ${canPtz ? `<button type="button" data-ptz-toggle title="${text("ptz")}" aria-label="${text("ptz")}">PTZ</button>` : ""}
        <button type="button" data-fullscreen-cell title="${text("fullscreen")}" aria-label="${text("fullscreen")}">&#9974;</button>
      </div>
      ${canPtz ? `<div class="ptz-pad" hidden><button type="button" data-ptz="up">&#9650;</button><button type="button" data-ptz="left">&#9664;</button><button type="button" data-ptz="stop">&#9632;</button><button type="button" data-ptz="right">&#9654;</button><button type="button" data-ptz="down">&#9660;</button><button type="button" data-ptz="zoom-in">+</button><button type="button" data-ptz="zoom-out">−</button></div>` : ""}
      <button class="cell-clear" type="button" data-clear-cell="${cellIndex}" title="${text("removeCamera")}" aria-label="${text("removeCamera")}">&#x00D7;</button>
    </article>`;
  }).join("");

  grid.querySelectorAll(".stream-cell").forEach(cell => {
    const activate = () => {
      const cellIndex = Number(cell.dataset.cell);
      if (state.activeCell !== cellIndex) setCellControlsOpen(null);
      state.activeCell = cellIndex;
      renderMonitorGrid(cameras);
      renderDeviceList(cameras);
    };
    cell.addEventListener("click", activate);
    cell.addEventListener("keydown", event => { if (event.key === "Enter" || event.key === " ") { event.preventDefault(); activate(); } });
  });
  grid.querySelectorAll("[data-clear-cell]").forEach(button => button.addEventListener("click", event => {
    event.stopPropagation();
    const index = Number(button.dataset.clearCell);
    state.assignments[index] = null;
    state.activeCell = index;
    setCellControlsOpen(null);
    persistWorkspace();
    renderMonitorGrid(cameras);
    renderDeviceList(cameras);
    updatePreviewStats();
  }));
  bindMonitorControls(cameras);
  grid.querySelectorAll("[data-stream-image]").forEach(image => {
    image.addEventListener("load", () => { image.closest(".stream-cell").classList.remove("stream-failed"); });
    image.addEventListener("error", () => { image.closest(".stream-cell").classList.add("stream-failed"); });
  });
  grid.querySelectorAll("[data-webrtc-video]").forEach(video => {
    const cell = video.closest(".stream-cell");
    const camera = findCameraByKey(cameras, state.assignments[Number(cell.dataset.cell)]);
    if (camera) startLivePreview(cell, video, camera);
  });
}

function setCellControlsOpen(cellIndex = null) {
  const nextCell = Number.isInteger(cellIndex) ? cellIndex : null;
  state.openControlsCell = nextCell;
  byId("monitor-grid").querySelectorAll(".stream-cell").forEach(cell => {
    const isOpen = Number(cell.dataset.cell) === nextCell;
    cell.classList.toggle("controls-open", isOpen);
    const toggle = cell.querySelector("[data-controls-toggle]");
    if (toggle) {
      const title = text(isOpen ? "hideCameraControls" : "showCameraControls");
      toggle.setAttribute("aria-expanded", String(isOpen));
      toggle.title = title;
      toggle.setAttribute("aria-label", title);
    }
    if (!isOpen) {
      const ptzPad = cell.querySelector(".ptz-pad");
      if (ptzPad) ptzPad.hidden = true;
    }
  });
}

function bindMonitorControls(cameras) {
  const grid = byId("monitor-grid");
  grid.querySelectorAll(".cell-controls-toggle, .cell-controls, .ptz-pad").forEach(node => node.addEventListener("click", event => event.stopPropagation()));
  grid.querySelectorAll("[data-controls-toggle]").forEach(button => button.addEventListener("click", () => {
    const cellIndex = Number(button.dataset.controlsToggle);
    setCellControlsOpen(state.openControlsCell === cellIndex ? null : cellIndex);
  }));
  grid.querySelectorAll("[data-record-camera]").forEach(button => button.addEventListener("click", () => toggleRecording(Number(button.dataset.recordCamera))));
  grid.querySelectorAll("[data-snapshot-camera]").forEach(button => button.addEventListener("click", () => downloadSnapshot(Number(button.dataset.snapshotCamera))));
  grid.querySelectorAll("[data-fullscreen-cell]").forEach(button => button.addEventListener("click", () => enterCellFullscreen(button.closest(".stream-cell"))));
  grid.querySelectorAll("[data-audio-toggle]").forEach(button => button.addEventListener("click", () => toggleCellAudio(button.closest(".stream-cell"), button)));
  grid.querySelectorAll("[data-audio-volume]").forEach(input => input.addEventListener("input", event => {
    event.stopPropagation();
    const video = input.closest(".stream-cell")?.querySelector("video");
    if (video) { video.volume = Number(input.value); video.muted = Number(input.value) <= 0; }
  }));
  grid.querySelectorAll("[data-ptz-toggle]").forEach(button => button.addEventListener("click", () => {
    const pad = button.closest(".stream-cell")?.querySelector(".ptz-pad");
    if (pad) pad.hidden = !pad.hidden;
  }));
  grid.querySelectorAll("[data-ptz]").forEach(button => {
    const cell = button.closest(".stream-cell");
    const cameraIndex = Number(cell?.dataset.cameraIndex);
    const action = button.dataset.ptz;
    const move = () => sendPtz(cameraIndex, action);
    if (action === "stop") button.addEventListener("click", move);
    else {
      button.addEventListener("pointerdown", event => { event.preventDefault(); move(); });
      ["pointerup", "pointercancel", "pointerleave"].forEach(name => button.addEventListener(name, () => sendPtz(cameraIndex, "stop")));
    }
  });
}

async function toggleRecording(cameraIndex) {
  const camera = (state.dashboard?.cameras || []).find(item => Number(item.index) === cameraIndex);
  if (!camera) return;
  try {
    await api("/api/v1/recording", {
      method: "POST", body: JSON.stringify({ cameraIndex, action: camera.recording ? "stop" : "start" })
    });
    showToast(text(camera.recording ? "recordingStopped" : "recordingStarted"));
    await loadDashboard();
  } catch (error) { showToast(error.message || text("operationFailed"), true); }
}

function downloadSnapshot(cameraIndex) {
  const link = document.createElement("a");
  link.href = `/api/v1/cameras/${encodeURIComponent(cameraIndex)}/snapshot.jpg`;
  link.download = "";
  document.body.append(link);
  link.click();
  link.remove();
  showToast(text("snapshotReady"));
}

function toggleCellAudio(cell, button) {
  const video = cell?.querySelector("video");
  if (!video) return;
  video.muted = !video.muted;
  if (!video.muted && video.volume <= 0) video.volume = 1;
  button.textContent = video.muted ? "🔇" : "🔊";
  button.title = text(video.muted ? "unmute" : "mute");
  button.setAttribute("aria-label", button.title);
  video.play().catch(() => {});
}

async function enterCellFullscreen(cell) {
  if (!cell) return;
  try {
    if (document.fullscreenElement) await document.exitFullscreen();
    else if (cell.requestFullscreen) await cell.requestFullscreen();
  } catch (error) { showToast(error.message || text("operationFailed"), true); }
}

async function sendPtz(cameraIndex, direction) {
  const vectors = {
    up: [0, 0.65, 0], down: [0, -0.65, 0], left: [-0.65, 0, 0],
    right: [0.65, 0, 0], "zoom-in": [0, 0, 0.65], "zoom-out": [0, 0, -0.65]
  };
  const vector = vectors[direction];
  const payload = direction === "stop" || !vector
    ? { cameraIndex, action: "stop" }
    : { cameraIndex, action: "move", x: vector[0], y: vector[1], zoom: vector[2] };
  const cell = document.querySelector(`.stream-cell[data-camera-index="${cameraIndex}"]`);
  cell?.classList.toggle("ptz-active", payload.action === "move");
  try { await api("/api/v1/ptz", { method: "POST", body: JSON.stringify(payload) }); }
  catch (error) { cell?.classList.remove("ptz-active"); showToast(error.message, true); }
}

function previewRequestUrl(baseUrl) {
  const quality = state.layout === 1 ? "hd" : "sd";
  return `${baseUrl}?quality=${quality}&frame=${Date.now()}`;
}

function refreshPreviewImages() {
  if (byId("app-view").hidden || state.currentView !== "cameras") return;
  document.querySelectorAll("[data-stream-image]").forEach(image => {
    if (image.closest(".stream-cell")?.classList.contains("stream-failed")) {
      image.src = previewRequestUrl(image.dataset.previewUrl);
    }
  });
}

function sendSocketMessage(payload) {
  if (!state.socket || state.socket.readyState !== WebSocket.OPEN) return false;
  state.socket.send(JSON.stringify(payload));
  return true;
}

function webRtcSupported() {
  return Boolean(state.server?.webRtcAvailable && window.RTCPeerConnection
    && state.socket?.readyState === WebSocket.OPEN);
}

function createPeerId() {
  if (window.crypto?.randomUUID) return window.crypto.randomUUID().replaceAll("-", "");
  return `peer_${Date.now()}_${Math.random().toString(36).slice(2)}`;
}

function startLivePreview(cell, video, camera) {
  const image = cell.querySelector("[data-stream-image]");
  if (!webRtcSupported()) {
    startMjpegFallback({ cell, image, camera });
    return;
  }

  const peerId = createPeerId();
  const pc = new RTCPeerConnection({ iceServers: [], bundlePolicy: "max-bundle" });
  const peer = { peerId, pc, cell, video, image, camera, pendingIce: [], timer: null,
    fallback: false };
  state.webrtcPeers.set(peerId, peer);
  peer.timer = window.setTimeout(() => activateMjpegFallback(peer, "WebRTC timeout"), 8000);

  pc.addEventListener("track", event => {
    const stream = event.streams[0] || new MediaStream([event.track]);
    video.srcObject = stream;
    video.hidden = false;
    image.hidden = true;
    cell.classList.remove("stream-failed");
    video.play().catch(() => {});
  });
  pc.addEventListener("icecandidate", event => {
    if (!event.candidate) return;
    sendSocketMessage({ type: "webrtc-ice", peerId,
      mlineIndex: event.candidate.sdpMLineIndex || 0,
      candidate: event.candidate.candidate });
  });
  pc.addEventListener("connectionstatechange", () => {
    if (pc.connectionState === "connected") {
      window.clearTimeout(peer.timer);
      cell.querySelector("[data-stream-transport]").textContent = "WebRTC";
      cell.classList.add("webrtc-active");
    } else if (pc.connectionState === "failed" || pc.connectionState === "closed") {
      activateMjpegFallback(peer, `WebRTC ${pc.connectionState}`);
    }
  });

  if (!sendSocketMessage({ type: "webrtc-start", peerId, cameraIndex: camera.index,
      quality: state.layout === 1 ? "hd" : "sd" })) {
    activateMjpegFallback(peer, "WebSocket unavailable");
  }
}

function startMjpegFallback(peer) {
  if (!peer?.image || !peer.camera) return;
  const baseUrl = peer.camera.previewStreamUrl || peer.camera.previewUrl;
  peer.image.hidden = false;
  if (baseUrl) peer.image.src = previewRequestUrl(baseUrl);
  peer.cell?.querySelector("[data-stream-transport]")?.replaceChildren("MJPEG");
}

function activateMjpegFallback(peer, reason) {
  if (!peer || peer.fallback) return;
  peer.fallback = true;
  window.clearTimeout(peer.timer);
  state.webrtcPeers.delete(peer.peerId);
  sendSocketMessage({ type: "webrtc-stop", peerId: peer.peerId });
  peer.pc?.close();
  if (peer.video) {
    peer.video.srcObject = null;
    peer.video.hidden = true;
  }
  peer.cell?.classList.remove("webrtc-active");
  startMjpegFallback(peer);
  console.warn(reason);
}

function stopAllWebRtcPeers() {
  for (const peer of state.webrtcPeers.values()) {
    window.clearTimeout(peer.timer);
    sendSocketMessage({ type: "webrtc-stop", peerId: peer.peerId });
    peer.pc?.close();
    if (peer.video) peer.video.srcObject = null;
  }
  state.webrtcPeers.clear();
}

async function handleSocketMessage(message) {
  if (message.type === "dashboard") {
    renderDashboard(message.data);
    return;
  }
  if (message.type === "logs") {
    if (state.currentView === "logs") {
      const incoming = message.data?.items || [];
      const ids = new Set(state.logs.map(item => item.id));
      state.logs = [...incoming.filter(item => !ids.has(item.id)), ...state.logs].slice(0, 500);
      renderLogs();
    }
    return;
  }
  const peer = state.webrtcPeers.get(message.peerId);
  if (!peer) return;
  try {
    if (message.type === "webrtc-offer") {
      await peer.pc.setRemoteDescription({ type: "offer", sdp: message.sdp });
      for (const candidate of peer.pendingIce.splice(0)) {
        await peer.pc.addIceCandidate(candidate);
      }
      const answer = await peer.pc.createAnswer();
      await peer.pc.setLocalDescription(answer);
      sendSocketMessage({ type: "webrtc-answer", peerId: peer.peerId,
        sdp: peer.pc.localDescription.sdp });
    } else if (message.type === "webrtc-ice") {
      const candidate = { candidate: message.candidate,
        sdpMLineIndex: Number(message.mlineIndex || 0) };
      if (peer.pc.remoteDescription) await peer.pc.addIceCandidate(candidate);
      else peer.pendingIce.push(candidate);
    } else if (message.type === "webrtc-error") {
      activateMjpegFallback(peer, message.error || "WebRTC error");
    }
  } catch (error) {
    activateMjpegFallback(peer, error.message || "WebRTC negotiation failed");
  }
}

function numericTemperature(value, depth = 0) {
  if (depth > 4 || value == null) return null;
  if (Array.isArray(value)) {
    for (const item of value) { const result = numericTemperature(item, depth + 1); if (result != null) return result; }
    return null;
  }
  if (typeof value !== "object") return null;
  for (const [key, item] of Object.entries(value)) {
    const normalized = key.toLowerCase().replace(/[_-]/g, "");
    if (["temperature", "temperaturec", "temp", "cputemperature", "soctemperature"].includes(normalized)) {
      const number = Number(item);
      if (Number.isFinite(number)) return number;
    }
  }
  for (const item of Object.values(value)) { const result = numericTemperature(item, depth + 1); if (result != null) return result; }
  return null;
}
