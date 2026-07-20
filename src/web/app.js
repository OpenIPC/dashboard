"use strict";

const translations = {
  en: {
    close: "Close", cancel: "Cancel", discoverCameras: "Discover", addCamera: "Add camera",
    cameraManagement: "Camera management", cameraManagementSubtitle: "Onboarding and discovery use the desktop camera database",
    manualCamera: "Manual camera", genericCamera: "Generic RTSP", cameraName: "Camera name",
    cameraNameOptional: "Optional; defaults to host", cameraHost: "IP address or host",
    cameraProfile: "Camera profile", rtspPort: "RTSP port", httpOnvifPort: "HTTP / ONVIF port",
    hdPath: "HD RTSP path", sdPath: "SD RTSP path", pathAutomatic: "Automatic for selected profile",
    cameraLogin: "Camera login", cameraPassword: "Camera password",
    credentialsPreserved: "Blank preserves stored value when editing",
    secretNote: "Stored credentials and credential-bearing RTSP URLs are never returned to the browser.",
    clearCredentials: "Clear stored camera credentials", saveCamera: "Save camera",
    networkInterface: "Network interface", allInterfaces: "All interfaces", deepScan: "Deep subnet scan",
    startDiscovery: "Start discovery", stopDiscovery: "Stop discovery", clearResults: "Clear results",
    onboardingProfile: "Onboarding profile", automatic: "Automatic", noDiscoveryResults: "No discovery results yet",
    addDiscovered: "Add", alreadyAdded: "Already added", cameraAdded: "Camera added",
    cameraUpdated: "Camera updated", cameraDeleted: "Camera deleted", editCamera: "Edit camera",
    deleteCamera: "Delete camera", deleteConfirm: "Delete this camera from Dashboard?",
    discoveryStarted: "Camera discovery started", discoveryStopped: "Camera discovery stopped",
    companion: "Web companion", username: "Username", password: "Password", login: "Sign in",
    logout: "Logout", language: "Language", layout: "Layout", layoutOne: "1 camera",
    layoutFour: "4 cameras", layoutNine: "9 cameras", gridSize: "Grid size",
    selectEmpty: "Select empty cell", actions: "Actions", collapseActions: "Collapse actions",
    expandActions: "Expand actions", cameras: "Cameras", health: "Health", analytics: "Analytics",
    archive: "Archive", refresh: "Refresh", state: "State", totalShort: "total",
    onlineShort: "online", offlineShort: "offline", previewReady: "Ready", devices: "Devices",
    ungrouped: "Ungrouped", allGroups: "All groups", searchDevices: "Search devices",
    serverReady: "Web server ready", updated: "Updated", emptyCell: "Empty cell",
    emptyCellHint: "Select a camera from the device list", streamUnavailable: "Preview unavailable",
    assignCamera: "Assign camera", openCamera: "Open camera UI", removeCamera: "Remove from layout",
    total: "Total", online: "Online", problems: "Problems", runCheck: "Run check",
    camera: "Camera", status: "Status", reason: "Reason", temperature: "Temperature",
    recentEvents: "Recent events", time: "Time", event: "Event", confidence: "Confidence",
    chooseRecording: "Choose a recording", noData: "No data", sessionExpired: "Session expired",
    loginFailed: "Sign in failed", requestFailed: "Request failed", healthSubtitle: "Camera health center",
    analyticsSubtitle: "Modules and event telemetry", archiveSubtitle: "Recording playback",
    checksRunning: "Camera checks are running", checkStarted: "Health check started",
    activeModules: "Active modules", events: "Events", detections: "Detections", evidence: "Evidence",
    allCameras: "All cameras", recording: "Recording", noTemperature: "-- °C",
    startRecording: "Start recording", stopRecording: "Stop recording", snapshot: "Snapshot",
    mute: "Mute", unmute: "Enable audio", fullscreen: "Fullscreen", ptz: "PTZ",
    showCameraControls: "Show camera controls", hideCameraControls: "Hide camera controls",
    recordingStarted: "Recording started", recordingStopped: "Recording stopped",
    snapshotReady: "Snapshot download started", operationFailed: "Operation failed",
    download: "Download", settings: "Settings", users: "Users", logs: "Logs",
    controlCenter: "Control Center", settingsHint: "Only browser-safe settings are exposed. Server endpoint changes remain desktop-only.",
    saveSettings: "Save settings", settingsSaved: "Settings saved", operator: "Operator",
    administrator: "Administrator", addUser: "Add user", role: "Role", permissions: "Permissions",
    activeSessions: "Active sessions", lastSeen: "Last seen", revoke: "Revoke", delete: "Delete",
    changePassword: "Change password", currentPassword: "Current password", newPassword: "New password",
    passwordChanged: "Password changed; sign in again", allLevels: "All levels", searchLogs: "Search logs",
    diagnosticBundle: "Diagnostic bundle", clearLogs: "Clear logs", level: "Level", message: "Message",
    loadMore: "Load more", loadStatus: "Load status", majestic: "Majestic", network: "Network",
    timeSettings: "Time", cameraLogs: "Camera logs", selectDeviceAction: "Select a safe read-only operation.",
    previewConfig: "Preview config", general: "General", streaming: "Streaming", appearance: "Appearance",
    web: "Web", languageSetting: "Language", notificationsEnabled: "Notifications", preferredStream: "Preferred stream",
    playerFillMode: "Video fill mode", showStatsOverlay: "Show stream statistics", defaultAutoplay: "Autoplay",
    playerBufferMode: "Buffer mode", playerRtspTransport: "RTSP transport", recordingSegmentDuration: "Recording segment (minutes)",
    analyticsEnabled: "Analytics enabled", sidebarVisible: "Sidebar visible", sidebarToolsExpanded: "Sidebar tools expanded",
    webSessionTimeoutMinutes: "Session timeout (minutes)", webSecureCookies: "Secure cookies", webServerEnabled: "Web server enabled",
    webServerAllowRemote: "Remote access", webServerBindAddress: "Bind address", webServerPort: "HTTP port", webSocketPort: "WebSocket port",
    exportConfig: "Export config", importConfig: "Import config", importConfigConfirm: "Import browser-safe settings and cameras from this file? Existing camera credentials will be preserved.",
    configImported: "Configuration imported", invalidConfigFile: "Invalid or oversized configuration file", updateInfo: "Update info",
    syncTime: "Sync time", rebootDevice: "Reboot", startUpdate: "Start update", confirmDeviceAction: "Confirm this action for the selected camera?"
    ,majesticEditor: "Majestic configuration", majesticEditorHint: "Edit JSON, preview the exact diff, then explicitly apply it.",
    previewChanges: "Preview changes", applyChanges: "Apply changes", noChanges: "No changes", confirmApplyChanges: "Apply this reviewed Majestic configuration diff?"
  },
  ru: {
    close: "Закрыть", cancel: "Отмена", discoverCameras: "Поиск", addCamera: "Добавить камеру",
    cameraManagement: "Управление камерами", cameraManagementSubtitle: "Подключение и поиск используют общую базу камер desktop-приложения",
    manualCamera: "Камера вручную", genericCamera: "Обычный RTSP", cameraName: "Название камеры",
    cameraNameOptional: "Необязательно; по умолчанию используется адрес", cameraHost: "IP-адрес или host",
    cameraProfile: "Профиль камеры", rtspPort: "Порт RTSP", httpOnvifPort: "Порт HTTP / ONVIF",
    hdPath: "RTSP-путь HD", sdPath: "RTSP-путь SD", pathAutomatic: "Автоматически для выбранного профиля",
    cameraLogin: "Логин камеры", cameraPassword: "Пароль камеры",
    credentialsPreserved: "При редактировании пустое поле сохраняет текущее значение",
    secretNote: "Сохранённые учётные данные и RTSP URL с секретами никогда не возвращаются в браузер.",
    clearCredentials: "Удалить сохранённые учётные данные камеры", saveCamera: "Сохранить камеру",
    networkInterface: "Сетевой интерфейс", allInterfaces: "Все интерфейсы", deepScan: "Глубокое сканирование подсети",
    startDiscovery: "Начать поиск", stopDiscovery: "Остановить поиск", clearResults: "Очистить результаты",
    onboardingProfile: "Профиль подключения", automatic: "Автоматически", noDiscoveryResults: "Результатов поиска пока нет",
    addDiscovered: "Добавить", alreadyAdded: "Уже добавлена", cameraAdded: "Камера добавлена",
    cameraUpdated: "Камера обновлена", cameraDeleted: "Камера удалена", editCamera: "Редактировать камеру",
    deleteCamera: "Удалить камеру", deleteConfirm: "Удалить эту камеру из Dashboard?",
    discoveryStarted: "Поиск камер запущен", discoveryStopped: "Поиск камер остановлен",
    companion: "Web-компаньон", username: "Пользователь", password: "Пароль", login: "Войти",
    logout: "Выход", language: "Язык", layout: "Раскладка", layoutOne: "1 камера",
    layoutFour: "4 камеры", layoutNine: "9 камер", gridSize: "Размер раскладки",
    selectEmpty: "Выбрать свободную ячейку", actions: "Действия", collapseActions: "Свернуть действия",
    expandActions: "Развернуть действия", cameras: "Камеры", health: "Здоровье", analytics: "Аналитика",
    archive: "Архив", refresh: "Обновить", state: "Состояние", totalShort: "всего",
    onlineShort: "онлайн", offlineShort: "офлайн", previewReady: "Готово", devices: "Устройства",
    ungrouped: "Без группы", allGroups: "Все группы", searchDevices: "Поиск устройств",
    serverReady: "Web-сервер работает", updated: "Обновлено", emptyCell: "Свободная ячейка",
    emptyCellHint: "Выберите камеру из списка устройств", streamUnavailable: "Предпросмотр недоступен",
    assignCamera: "Добавить в раскладку", openCamera: "Открыть Web UI камеры", removeCamera: "Убрать из раскладки",
    total: "Всего", online: "Онлайн", problems: "Проблемы", runCheck: "Проверить все",
    camera: "Камера", status: "Статус", reason: "Причина", temperature: "Температура",
    recentEvents: "Последние события", time: "Время", event: "Событие", confidence: "Уверенность",
    chooseRecording: "Выберите запись", noData: "Нет данных", sessionExpired: "Сессия завершена",
    loginFailed: "Не удалось войти", requestFailed: "Ошибка запроса", healthSubtitle: "Центр здоровья камер",
    analyticsSubtitle: "Модули и телеметрия событий", archiveSubtitle: "Просмотр записей",
    checksRunning: "Выполняется проверка камер", checkStarted: "Проверка здоровья запущена",
    activeModules: "Активные модули", events: "События", detections: "Детекции", evidence: "Доказательства",
    allCameras: "Все камеры", recording: "Запись", noTemperature: "-- °C",
    startRecording: "Начать запись", stopRecording: "Остановить запись", snapshot: "Снимок",
    mute: "Выключить звук", unmute: "Включить звук", fullscreen: "На весь экран", ptz: "PTZ",
    showCameraControls: "Показать управление камерой", hideCameraControls: "Скрыть управление камерой",
    recordingStarted: "Запись начата", recordingStopped: "Запись остановлена",
    snapshotReady: "Скачивание снимка началось", operationFailed: "Операция не выполнена",
    download: "Скачать", settings: "Настройки", users: "Пользователи", logs: "Логи",
    controlCenter: "Центр управления", settingsHint: "Доступны только безопасные для браузера настройки. Изменение адреса сервера выполняется в desktop.",
    saveSettings: "Сохранить настройки", settingsSaved: "Настройки сохранены", operator: "Оператор",
    administrator: "Администратор", addUser: "Добавить пользователя", role: "Роль", permissions: "Права",
    activeSessions: "Активные сессии", lastSeen: "Последняя активность", revoke: "Завершить", delete: "Удалить",
    changePassword: "Сменить пароль", currentPassword: "Текущий пароль", newPassword: "Новый пароль",
    passwordChanged: "Пароль изменён; войдите снова", allLevels: "Все уровни", searchLogs: "Поиск по логам",
    diagnosticBundle: "Диагностический пакет", clearLogs: "Очистить логи", level: "Уровень", message: "Сообщение",
    loadMore: "Загрузить ещё", loadStatus: "Загрузить статус", majestic: "Majestic", network: "Сеть",
    timeSettings: "Время", cameraLogs: "Логи камеры", selectDeviceAction: "Выберите безопасную операцию чтения.",
    previewConfig: "Предпросмотр конфигурации", general: "Основные", streaming: "Потоки", appearance: "Интерфейс",
    web: "Web", languageSetting: "Язык", notificationsEnabled: "Уведомления", preferredStream: "Предпочитаемый поток",
    playerFillMode: "Заполнение видео", showStatsOverlay: "Показывать статистику потока", defaultAutoplay: "Автовоспроизведение",
    playerBufferMode: "Режим буфера", playerRtspTransport: "RTSP-транспорт", recordingSegmentDuration: "Сегмент записи (минуты)",
    analyticsEnabled: "Аналитика включена", sidebarVisible: "Показывать боковую панель", sidebarToolsExpanded: "Действия развёрнуты",
    webSessionTimeoutMinutes: "Таймаут сессии (минуты)", webSecureCookies: "Secure cookies", webServerEnabled: "Web-сервер включён",
    webServerAllowRemote: "Удалённый доступ", webServerBindAddress: "Адрес привязки", webServerPort: "HTTP-порт", webSocketPort: "WebSocket-порт",
    exportConfig: "Экспорт конфигурации", importConfig: "Импорт конфигурации", importConfigConfirm: "Импортировать безопасные настройки и камеры из файла? Сохранённые данные доступа к камерам не изменятся.",
    configImported: "Конфигурация импортирована", invalidConfigFile: "Некорректный или слишком большой файл конфигурации", updateInfo: "Данные обновления",
    syncTime: "Синхронизировать время", rebootDevice: "Перезагрузить", startUpdate: "Запустить обновление", confirmDeviceAction: "Подтвердить это действие для выбранной камеры?"
    ,majesticEditor: "Конфигурация Majestic", majesticEditorHint: "Измените JSON, проверьте точный diff и только затем примените его.",
    previewChanges: "Проверить изменения", applyChanges: "Применить", noChanges: "Изменений нет", confirmApplyChanges: "Применить проверенный diff конфигурации Majestic?"
  }
};

const validLayouts = [1, 4, 9];
const savedLayout = Number(localStorage.getItem("openipc-web-layout") || 4);
let savedAssignments = [];
try { savedAssignments = JSON.parse(localStorage.getItem("openipc-web-assignments") || "[]"); } catch (_) {}

const state = {
  language: localStorage.getItem("openipc-language") || "ru",
  server: null,
  presentation: null,
  capabilities: null,
  session: null,
  dashboard: null,
  health: null,
  analytics: null,
  archive: [],
  settings: null,
  userAdmin: null,
  logs: [],
  logCursor: 0,
  diagnostics: null,
  socket: null,
  refreshTimer: null,
  previewTimer: null,
  toastTimer: null,
  discoveryTimer: null,
  deviceTimer: null,
  majesticOperationId: "",
  majesticPreviewId: "",
  lastFocus: null,
  discovery: null,
  cameraDialogPage: "manual",
  webrtcPeers: new Map(),
  monitorSignature: "",
  layout: validLayouts.includes(savedLayout) ? savedLayout : 4,
  activeCell: 0,
  openControlsCell: null,
  assignments: Array.isArray(savedAssignments) ? savedAssignments : [],
  currentView: "cameras"
};

const byId = id => document.getElementById(id);
const text = key => translations[state.language][key] || translations.en[key] || key;
const escapeHtml = value => String(value ?? "").replace(/[&<>'"]/g, char => ({"&":"&amp;","<":"&lt;",">":"&gt;","'":"&#39;",'"':"&quot;"})[char]);
const isOnline = status => ["online", "онлайн"].includes(String(status || "").trim().toLowerCase());
const cameraKey = camera => String(camera?.id || `index:${camera?.index}`);
const canManageCameras = () => Boolean(state.session && (((Number(state.session.permissions) & 0xff) === 0xff) || (Number(state.session.permissions) & 0x10)));

function persistWorkspace() {
  localStorage.setItem("openipc-web-layout", String(state.layout));
  localStorage.setItem("openipc-web-assignments", JSON.stringify(state.assignments.slice(0, state.layout)));
}

function applyLanguage() {
  document.documentElement.lang = state.language;
  document.querySelectorAll("[data-i18n]").forEach(node => { node.textContent = text(node.dataset.i18n); });
  document.querySelectorAll("[data-i18n-placeholder]").forEach(node => { node.placeholder = text(node.dataset.i18nPlaceholder); });
  document.querySelectorAll("[data-i18n-title]").forEach(node => { node.title = text(node.dataset.i18nTitle); node.setAttribute("aria-label", text(node.dataset.i18nTitle)); });
  byId("language").textContent = state.language === "ru" ? "EN" : "RU";
  byId("language-login").textContent = state.language === "ru" ? "EN" : "RU";
  if (state.dashboard) {
    state.monitorSignature = "";
    renderDashboard(state.dashboard);
  }
  if (state.health) renderHealth(state.health);
  if (state.analytics) renderAnalytics(state.analytics);
  renderArchive();
  if (state.discovery) renderDiscovery(state.discovery);
  if (state.settings) renderSettings(state.settings);
  if (state.userAdmin) renderUsers(state.userAdmin);
  if (state.logs.length) renderLogs();
  if (state.diagnostics) renderDiagnostics(state.diagnostics);
  if (byId("camera-backdrop") && !byId("camera-backdrop").hidden) {
    byId("camera-dialog-title").textContent = text(state.cameraDialogPage === "discovery"
      ? "discoverCameras" : (byId("camera-edit-index").value === "" ? "addCamera" : "editCamera"));
  }
  if (state.currentView !== "cameras") updateDialogHeader(state.currentView);
}

async function api(path, options = {}) {
  const headers = { Accept: "application/json", ...(options.headers || {}) };
  if (options.method === "POST") headers["X-OpenIPC-CSRF"] = "1";
  if (options.body) headers["Content-Type"] = "application/json";
  const response = await fetch(path, { credentials: "same-origin", ...options, headers });
  const payload = await response.json().catch(() => ({ ok: false, error: text("requestFailed") }));
  if (!response.ok) {
    if (response.status === 401 && path !== "/api/v1/auth/login") showLogin(text("sessionExpired"));
    throw new Error(payload.error || text("requestFailed"));
  }
  return payload.data;
}

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
  byId("device-commands").hidden = !canManageCameras();
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
  state.assignments = state.assignments.slice(0, state.layout).map(key => keys.has(String(key)) ? String(key) : null);
  while (state.assignments.length < state.layout) state.assignments.push(null);
  if (!state.assignments.some(Boolean) && cameras.length) state.assignments[0] = cameraKey(cameras[0]);
  state.activeCell = Math.min(state.activeCell, state.layout - 1);
  if (state.openControlsCell !== null
      && (state.openControlsCell >= state.layout || !state.assignments[state.openControlsCell])) {
    state.openControlsCell = null;
  }
  persistWorkspace();
}

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

function selectNextEmptyCell() {
  setCellControlsOpen(null);
  const next = state.assignments.findIndex(value => !value);
  state.activeCell = next >= 0 ? next : (state.activeCell + 1) % state.layout;
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
  byId("permission-editor").innerHTML = catalog.map(permission =>
    `<label><input type="checkbox" data-new-permission="${Number(permission.value)}" ${[1, 2, 4, 64].includes(Number(permission.value)) ? "checked" : ""}><span>${escapeHtml(permission.id)}</span></label>`
  ).join("");
  const users = data.users || [];
  byId("users-body").innerHTML = users.length ? users.map(user => `<tr><td>${escapeHtml(user.username)}</td><td>${escapeHtml(text(user.role === "admin" ? "administrator" : "operator"))}</td><td>${Number(user.permissions)}</td><td><div class="inline-actions">${user.role === "admin" ? "" : `<input type="number" min="0" max="127" value="${Number(user.permissions)}" data-permissions-user="${escapeHtml(user.username)}"><button type="button" data-save-permissions="${escapeHtml(user.username)}">${text("saveSettings")}</button>`}${user.username === state.session?.username ? "" : `<button class="danger" type="button" data-delete-user="${escapeHtml(user.username)}">${text("delete")}</button>`}</div></td></tr>`).join("") : `<tr><td colspan="4">${text("noData")}</td></tr>`;
  const sessions = data.sessions || [];
  byId("sessions-body").innerHTML = sessions.length ? sessions.map(session => `<tr><td>${escapeHtml(session.username)}${session.current ? " · current" : ""}</td><td>${formatDateTime(session.lastSeenAt)}</td><td>${session.current ? "" : `<button type="button" data-revoke-session="${escapeHtml(session.id)}">${text("revoke")}</button>`}</td></tr>`).join("") : `<tr><td colspan="3">${text("noData")}</td></tr>`;
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
      role, permissions
    }) });
    byId("new-user-name").value = ""; byId("new-user-password").value = "";
    await loadUsers();
  } catch (error) { showToast(error.message, true); }
}

async function saveUserPermissions(username) {
  const input = document.querySelector(`[data-permissions-user="${CSS.escape(username)}"]`);
  try {
    await api("/api/v1/users/permissions", { method: "POST", body: JSON.stringify({ username, permissions: Number(input.value) }) });
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
  try {
    await api("/api/v1/sessions/revoke", { method: "POST", body: JSON.stringify({ id }) });
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
  state.socket = new WebSocket(`${protocol}://${location.hostname}:${state.server.webSocketPort}`);
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
