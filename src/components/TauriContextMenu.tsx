import React, { useCallback, useEffect } from 'react';
import { invoke } from '@tauri-apps/api/core';
import type { Camera } from '../types';

interface TauriContextMenuProps {
  camera: Camera | null;
  anchorPosition: { left: number; top: number } | null;
  onClose: () => void;
  onArchive: () => void;
  onEdit: (camera: Camera) => void;
  onDelete: (camera: Camera) => void;
  onOpenInBrowser: (camera: Camera) => void;
  onFileManager: (camera: Camera) => void;
  onSSH: (camera: Camera) => void;
}

export const TauriContextMenu: React.FC<TauriContextMenuProps> = ({
  camera,
  anchorPosition,
  onClose,
  onArchive,
  onEdit,
  onDelete,
  onOpenInBrowser,
  onFileManager,
  onSSH,
}) => {
  const handleMenuAction = useCallback((action: string) => {
    if (!camera) return;

    switch (action) {
      case 'archive':
        onArchive();
        break;
      case 'edit':
        onEdit(camera);
        break;
      case 'delete':
        onDelete(camera);
        break;
      case 'browser':
        onOpenInBrowser(camera);
        break;
      case 'filemanager':
        onFileManager(camera);
        break;
      case 'ssh':
        onSSH(camera);
        break;
    }
    onClose();
  }, [camera, onArchive, onClose, onDelete, onEdit, onFileManager, onOpenInBrowser, onSSH]);

  const showNativeContextMenu = useCallback(async () => {
    if (!camera) return;

    try {
      // Создаем меню с помощью Tauri API
      const menuItems = [
        { id: 'archive', label: '📁 Архив' },
        { id: 'edit', label: '✏️ Редактировать камеру' },
        { id: 'delete', label: '🗑️ Удалить камеру' },
        { id: 'separator1', label: '-' },
        { id: 'browser', label: '🌐 Открыть в браузере' },
        { id: 'filemanager', label: '📂 Файловый менеджер' },
        { id: 'ssh', label: '🖥️ SSH терминал' },
      ];

      // Вызываем Tauri команду для показа контекстного меню
      const result = await invoke('show_context_menu', {
        items: menuItems,
        position: anchorPosition
      });

      // Обрабатываем результат
      handleMenuAction(result as string);
    } catch (error) {
      console.error('Failed to show context menu:', error);
      // Fallback - закрываем меню
      onClose();
    }
  }, [anchorPosition, camera, handleMenuAction, onClose]);

  useEffect(() => {
    if (camera && anchorPosition) {
      // Показываем нативное контекстное меню Tauri
      void showNativeContextMenu();
    }
  }, [anchorPosition, camera, showNativeContextMenu]);

  // Этот компонент не рендерит ничего визуально, так как использует нативное меню
  return null;
};