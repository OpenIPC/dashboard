import React, { createContext, useCallback, useContext, useMemo, useRef, useState } from 'react';
import type { ReactNode } from 'react';
import { CameraContextMenu } from '../components/CameraContextMenu';
import type { Camera, CameraGroup } from '../types';

type CameraContextMenuAction = (camera: Camera) => void;
type MoveToGroupAction = (camera: Camera, groupId: number | null) => void;

export type CameraContextMenuHandlers = {
  onArchive?: CameraContextMenuAction;
  onEdit?: CameraContextMenuAction;
  onDelete?: CameraContextMenuAction;
  onOpenInBrowser?: CameraContextMenuAction;
  onFileManager?: CameraContextMenuAction;
  onSSH?: CameraContextMenuAction;
  onMoveToGroup?: MoveToGroupAction;
};

interface OpenCameraContextMenuPayload {
  camera: Camera;
  anchorPosition: { left: number; top: number };
  handlers?: CameraContextMenuHandlers;
  groups?: CameraGroup[];
}

interface CameraContextMenuContextValue {
  openCameraContextMenu: (payload: OpenCameraContextMenuPayload) => void;
  closeCameraContextMenu: () => void;
  registerDefaultCameraContextMenuHandlers: (handlers: CameraContextMenuHandlers) => void;
  getDefaultCameraContextMenuHandlers: () => CameraContextMenuHandlers;
}

interface CameraContextMenuState {
  camera: Camera;
  anchorPosition: { left: number; top: number };
  handlers: CameraContextMenuHandlers;
  groups: CameraGroup[];
}

const CameraContextMenuContext = createContext<CameraContextMenuContextValue | undefined>(undefined);

export const CameraContextMenuProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [state, setState] = useState<CameraContextMenuState | null>(null);
  const defaultHandlersRef = useRef<CameraContextMenuHandlers>({});

  const closeCameraContextMenu = useCallback(() => setState(null), []);

  const openCameraContextMenu = useCallback((payload: OpenCameraContextMenuPayload) => {
    setState({
      camera: payload.camera,
      anchorPosition: payload.anchorPosition,
      handlers: {
        ...defaultHandlersRef.current,
        ...(payload.handlers ?? {}),
      },
      groups: payload.groups ?? [],
    });
  }, []);

  const registerDefaultCameraContextMenuHandlers = useCallback((handlers: CameraContextMenuHandlers) => {
    defaultHandlersRef.current = handlers;
  }, []);

  const getDefaultCameraContextMenuHandlers = useCallback(() => {
    return defaultHandlersRef.current;
  }, []);

  const contextValue = useMemo<CameraContextMenuContextValue>(() => ({
    openCameraContextMenu,
    closeCameraContextMenu,
    registerDefaultCameraContextMenuHandlers,
    getDefaultCameraContextMenuHandlers,
  }), [closeCameraContextMenu, getDefaultCameraContextMenuHandlers, openCameraContextMenu, registerDefaultCameraContextMenuHandlers]);

  return (
    <CameraContextMenuContext.Provider value={contextValue}>
      {children}
      <CameraContextMenu
        camera={state?.camera ?? null}
        anchorPosition={state?.anchorPosition ?? null}
        onClose={closeCameraContextMenu}
        onArchive={(camera) => state?.handlers.onArchive?.(camera)}
        onEdit={(camera) => state?.handlers.onEdit?.(camera)}
        onDelete={(camera) => state?.handlers.onDelete?.(camera)}
        onOpenInBrowser={(camera) => state?.handlers.onOpenInBrowser?.(camera)}
        onFileManager={(camera) => state?.handlers.onFileManager?.(camera)}
        onSSH={(camera) => state?.handlers.onSSH?.(camera)}
        groups={state?.groups ?? []}
        onMoveToGroup={(camera, groupId) => state?.handlers.onMoveToGroup?.(camera, groupId)}
      />
    </CameraContextMenuContext.Provider>
  );
};

export const useCameraContextMenu = (): CameraContextMenuContextValue => {
  const context = useContext(CameraContextMenuContext);
  if (!context) {
    throw new Error('useCameraContextMenu must be used within a CameraContextMenuProvider');
  }
  return context;
};
