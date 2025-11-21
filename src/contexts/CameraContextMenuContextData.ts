import { createContext } from 'react';
import type { Camera, CameraGroup } from '../types';

export type CameraContextMenuAction = (camera: Camera) => void;
export type MoveToGroupAction = (camera: Camera, groupId: number | null) => void;

export type CameraContextMenuHandlers = {
  onArchive?: CameraContextMenuAction;
  onEdit?: CameraContextMenuAction;
  onDelete?: CameraContextMenuAction;
  onOpenInBrowser?: CameraContextMenuAction;
  onFileManager?: CameraContextMenuAction;
  onSSH?: CameraContextMenuAction;
  onMoveToGroup?: MoveToGroupAction;
};

export interface OpenCameraContextMenuPayload {
  camera: Camera;
  anchorPosition: { left: number; top: number };
  handlers?: CameraContextMenuHandlers;
  groups?: CameraGroup[];
}

export interface CameraContextMenuContextValue {
  openCameraContextMenu: (payload: OpenCameraContextMenuPayload) => void;
  closeCameraContextMenu: () => void;
  registerDefaultCameraContextMenuHandlers: (handlers: CameraContextMenuHandlers) => void;
  getDefaultCameraContextMenuHandlers: () => CameraContextMenuHandlers;
}

export const CameraContextMenuContext = createContext<
  CameraContextMenuContextValue | undefined
>(undefined);
