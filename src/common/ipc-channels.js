// Этот файл будет единственным источником истины для всех названий IPC-каналов.

module.exports = {
  // Window & App
  MINIMIZE_WINDOW: 'minimize-window',
  MAXIMIZE_WINDOW: 'maximize-window',
  CLOSE_WINDOW: 'close-window',
  GET_APP_VERSION_INFO: 'get-app-version-info',
  GET_BRANDING_CONFIG: 'get-branding-config',
  OPEN_EXTERNAL_LINK: 'open-external-link',
  ON_WINDOW_MAXIMIZED: 'window-maximized',
  ON_WINDOW_UNMAXIMIZED: 'window-unmaximized',

  // Clipboard
  CLIPBOARD_READ: 'clipboardRead',
  CLIPBOARD_WRITE: 'clipboardWrite',

  // Auth & Users
  LOGIN: 'login',
  LOGOUT_CLEAR_CREDS: 'logout-clear-credentials',
  GET_USERS: 'get-users',
  ADD_USER: 'add-user',
  UPDATE_USER_PASSWORD: 'update-user-password',
  UPDATE_USER_ROLE: 'update-user-role',
  UPDATE_USER_PERMISSIONS: 'update-user-permissions',
  DELETE_USER: 'delete-user',
  ON_AUTO_LOGIN_SUCCESS: 'auto-login-success',
  RENDERER_READY: 'renderer-ready-for-autologin',

  // Config & Settings
  LOAD_APP_SETTINGS: 'load-app-settings',
  SAVE_APP_SETTINGS: 'save-app-settings',
  LOAD_CONFIG: 'load-configuration',
  SAVE_CONFIG: 'save-configuration',
  EXPORT_CONFIG: 'export-config',
  IMPORT_CONFIG: 'import-config',
  GET_TRANSLATION: 'get-translation-file',
  SELECT_DIRECTORY: 'select-directory',
  
  // Local Filesystem
  GET_LOCAL_DISK_LIST: 'get-local-disk-list',
  LIST_LOCAL_FILES: 'list-local-files',

  // Camera & Stream
  GET_CAMERA_PULSE: 'get-camera-pulse',
  PTZ_CONTROL: 'ptz-control',
  GET_CAMERA_TIME: 'get-camera-time',
  GET_CAMERA_SETTINGS: 'get-camera-settings',
  SET_CAMERA_SETTINGS: 'set-camera-settings',
  RESTART_MAJESTIC: 'restart-majestic',
  OPEN_IN_BROWSER: 'open-in-browser',
  START_VIDEO_STREAM: 'start-video-stream',
  STOP_VIDEO_STREAM: 'stop-video-stream',
  PAUSE_VIDEO_STREAM: 'pause-video-stream',
  RESUME_VIDEO_STREAM: 'resume-video-stream',
  OPEN_FILE_MANAGER: 'open-file-manager',
  OPEN_SSH_TERMINAL: 'open-ssh-terminal',

  // Video Analytics
  TOGGLE_ANALYTICS: 'toggle-analytics',
  ON_ANALYTICS_UPDATE: 'analytics-update',
  ON_ANALYTICS_STATUS_CHANGE: 'analytics-status-change',
  ON_ANALYTICS_PROVIDER_INFO: 'analytics-provider-info',

  // Recording & Archive
  TOGGLE_RECORDING: 'toggle-recording',
  ON_RECORDING_STATE_CHANGE: 'recording-state-change',
  OPEN_RECORDINGS_FOLDER: 'open-recordings-folder',
  GET_RECORDINGS_FOR_DATE: 'get-recordings-for-date',
  EXPORT_ARCHIVE_CLIP: 'export-archive-clip',
  EXPORT_ARCHIVE_CLIP_BATCH: 'export-archive-clip-batch',
  GET_EVENTS_FOR_DATE: 'get-events-for-date',
  GET_DATES_WITH_ACTIVITY: 'get-dates-with-activity',
  PREPARE_ARCHIVE_FOR_HLS: 'prepare-archive-for-hls',

  // Archive Thumbnails
  GET_ARCHIVE_THUMBNAILS: 'get-archive-thumbnails',

  // System & Events
  SAVE_SCREENSHOT: 'save-screenshot',
  GET_SYSTEM_STATS: 'get-system-stats',
  ON_STREAM_INFO_UPDATE: 'stream-info-update',
  ON_STREAM_DIED: 'stream-died',
  ON_STREAM_STATS: 'stream-stats',
  ON_MEDIAMTX_STATS_UPDATE: 'mediamtx-stats-update',
  // MediaMTX update lifecycle / status messages
  ON_MEDIAMTX_UPDATE: 'mediamtx-update-status',
  ON_MAIN_ERROR: 'on-main-error',
  SHOW_CAMERA_CONTEXT_MENU: 'show-camera-context-menu',
  ON_CONTEXT_MENU_COMMAND: 'context-menu-command',
  PROBE_CAMERA_INFO: 'probe-camera-info',
  PROBE_ONVIF_STREAM_URI: 'probe-onvif-stream-uri',
  TEST_RTSP_URL: 'test-rtsp-url',
  SHOW_GROUP_CONTEXT_MENU: 'show-group-context-menu',
  ON_GROUP_CONTEXT_MENU_COMMAND: 'group-context-menu-command',
  KILL_ALL_FFMPEG: 'kill-all-ffmpeg',
  LOG_FROM_RENDERER: 'log',

  // Updates
  CHECK_FOR_UPDATES: 'check-for-updates',
  ON_UPDATE_STATUS: 'update-status',
  DOWNLOAD_UPDATE: 'download-update',
  QUIT_AND_INSTALL_UPDATE: 'quit-and-install-update',

  // Discovery
  DISCOVER_DEVICES: 'discover-devices',
  ON_DEVICE_FOUND: 'device-found',
  
  // NETIP
  GET_NETIP_SETTINGS: 'get-netip-settings',
  SET_NETIP_SETTINGS: 'set-netip-settings',
  
  // Reporting
  OPEN_IMAGE_FILES: 'open-and-read-image-files',
  SUBMIT_REPORT: 'submit-report',

  // Module System
  GET_AVAILABLE_MODULES: 'get-available-modules',
  SAVE_ENABLED_MODULES: 'save-enabled-modules',
  GET_RENDERER_MODULES: 'get-renderer-modules',
  GET_DETECTED_PLATES: 'get-detected-plates',
  PREPARE_LICENSE_PLATE_RUNTIME: 'prepare-license-plate-runtime',

  // SCP (File Manager)
  SCP_CONNECT: 'scp-connect',
  SCP_LIST: 'scp-list',
  SCP_DOWNLOAD: 'scp-download',
  SCP_UPLOAD: 'scp-upload',
  SCP_MKDIR: 'scp-mkdir',
  SCP_DELETE_FILE: 'scp-delete-file',
  SCP_DELETE_DIR: 'scp-delete-dir',
};