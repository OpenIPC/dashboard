import { useContext } from 'react';
import { CameraContextMenuContext } from '../contexts/CameraContextMenuContextData';

export const useCameraContextMenu = () => {
  const context = useContext(CameraContextMenuContext);
  if (!context) {
    throw new Error('useCameraContextMenu must be used within a CameraContextMenuProvider');
  }
  return context;
};
