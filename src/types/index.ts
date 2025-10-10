// Общие типы для всего приложения

export type StreamQuality = 'sd' | 'hd';

export const MAX_DASHBOARD_CELLS = 64;

export interface Camera {
  id: number;
  name: string;
  ip: string;
  protocol: string;
  port: number;
  user: string;
  pass: string;
  pass_enc?: string;
  path_hd: string;
  path_sd: string;
  status: string;
  onvifPort?: number;
  groupId?: number | null;
  // Дополнительные поля для совместимости
  main_stream?: string;
  sub_stream?: string;
  brand?: string;
  isConnected?: boolean;
  streamUrl?: string;
}

export interface CameraStats {
  codec?: string;
  bitrate?: string;
  resolution?: string;
  fps?: number;
}

export interface CameraGroup {
  id: number;
  name: string;
  color?: string;
  cameraIds: number[];
  createdAt: Date;
}

// Типы для шаблонов раскладки
export interface LayoutTemplate {
  id: string;
  name: string;
  description?: string;
  gridSize: number; // количество ячеек (1, 4, 9, 16, 25, 32, 64)
  cameraAssignments: LayoutCameraAssignment[];
  createdAt: Date;
  updatedAt: Date;
}

export interface LayoutCameraAssignment {
  cellIndex: number; // индекс ячейки (0-63)
  cameraId: number | null; // ID камеры или null для пустой ячейки
}

export interface LayoutTemplatePreview {
  id: string;
  name: string;
  description?: string;
  gridSize: number;
  cameraCount: number; // количество назначенных камер
  previewCameras: string[]; // первые несколько имен камер для превью
  createdAt: Date;
}

export interface StoredLayoutTemplatePreview extends Omit<LayoutTemplatePreview, 'createdAt'> {
  createdAt: string;
}

export interface StoredLayoutTemplate extends Omit<LayoutTemplate, 'createdAt' | 'updatedAt'> {
  createdAt: string;
  updatedAt: string;
}

// Типы для системы вкладок раскладок
export interface LayoutTab {
  id: string;
  name: string;
  template: LayoutTemplate;
  isActive: boolean;
}

export interface StoredLayoutTab {
  id: string;
  name: string;
  templateId: string;
}

export interface LayoutTabsState {
  tabs: LayoutTab[];
  activeTabId: string | null;
}

export interface DashboardCellState {
  cameraId: number | null;
  quality: StreamQuality;
  muted: boolean;
  paused: boolean;
}

export interface DashboardState {
  gridSize: number;
  cellStates: DashboardCellState[];
  savedTemplatePreviews?: StoredLayoutTemplatePreview[];
  layoutTemplates?: StoredLayoutTemplate[];
  layoutTabs?: StoredLayoutTab[];
  activeLayoutTabId?: string | null;
}

export type UserRole = 'admin' | 'operator';

export type UserPermissions = {
  view_archive?: boolean;
  export_archive?: boolean;
  edit_cameras?: boolean;
  delete_cameras?: boolean;
  access_settings?: boolean;
  manage_layout?: boolean;
};

export interface AuthUser {
  username: string;
  role: UserRole;
  permissions?: UserPermissions;
}