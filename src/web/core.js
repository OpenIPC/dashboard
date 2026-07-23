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
    sessionSource: "Origin / peer", sessionExpiry: "Idle / absolute expiry", revokeReason: "Reason for revocation",
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
    sessionSource: "Origin / адрес", sessionExpiry: "Idle / абсолютный срок", revokeReason: "Причина завершения сессии",
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
