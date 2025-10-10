import React, { useRef } from 'react';
import { Menu, MenuItem, ListItemIcon, ListItemText, Divider, Box } from '@mui/material';
import {
  Archive as ArchiveIcon,
  Edit as EditIcon,
  Delete as DeleteIcon,
  OpenInBrowser as OpenInBrowserIcon,
  FolderOpen as FolderIcon,
  Terminal as TerminalIcon,
  GroupWork as GroupIcon,
  ArrowRight as ArrowRightIcon,
} from '@mui/icons-material';
import { useLocalization } from '../contexts/LocalizationContext';
import type { Camera, CameraGroup } from '../types';

interface CameraContextMenuProps {
  camera: Camera | null;
  anchorPosition: { left: number; top: number } | null;
  onClose: () => void;
  onArchive: (camera: Camera) => void;
  onEdit: (camera: Camera) => void;
  onDelete: (camera: Camera) => void;
  onOpenInBrowser: (camera: Camera) => void;
  onFileManager: (camera: Camera) => void;
  onSSH: (camera: Camera) => void;
  groups?: CameraGroup[];
  onMoveToGroup?: (camera: Camera, groupId: number | null) => void;
}

export const CameraContextMenu: React.FC<CameraContextMenuProps> = ({
  camera,
  anchorPosition,
  onClose,
  onArchive,
  onEdit,
  onDelete,
  onOpenInBrowser,
  onFileManager,
  onSSH,
  groups = [],
  onMoveToGroup,
}) => {
  const { t } = useLocalization();
  const menuRef = useRef<HTMLDivElement>(null);
  const [groupSubmenuAnchor, setGroupSubmenuAnchor] = React.useState<null | HTMLElement>(null);

  const handleMenuItemClick = (action: () => void) => {
    action();
    onClose();
  };

  const handleGroupSubmenuOpen = (event: React.MouseEvent<HTMLElement>) => {
    setGroupSubmenuAnchor(event.currentTarget);
  };

  const handleGroupSubmenuClose = () => {
    setGroupSubmenuAnchor(null);
  };

  const handleMoveToGroup = (groupId: number | null) => {
    if (camera && onMoveToGroup) {
      onMoveToGroup(camera, groupId);
    }
    handleGroupSubmenuClose();
    onClose();
  };

  // Определяем текущую группу камеры
  const currentGroup = groups.find(group => group.cameraIds.includes(camera?.id || 0));

  if (!camera || !anchorPosition) {
    return null;
  }

  return (
    <Menu
      ref={menuRef}
      open={Boolean(anchorPosition)}
      onClose={onClose}
      anchorReference="anchorPosition"
      anchorPosition={anchorPosition}
      transformOrigin={{
        vertical: 'top',
        horizontal: 'left',
      }}
      PaperProps={{
        sx: {
          backgroundColor: '#2a2f33',
          color: '#fff',
          minWidth: 200,
          '& .MuiMenuItem-root': {
            padding: '8px 16px',
            '&:hover': {
              backgroundColor: '#3b4146',
            },
          },
          '& .MuiListItemIcon-root': {
            color: '#fff',
            minWidth: '36px',
          },
        },
      }}
    >
      <MenuItem onClick={() => handleMenuItemClick(() => onArchive(camera))}>
        <ListItemIcon>
          <ArchiveIcon fontSize="small" />
        </ListItemIcon>
        <ListItemText primary={t('archive_title')} />
      </MenuItem>

      <Divider sx={{ backgroundColor: '#3b4146', margin: '4px 0' }} />

      <MenuItem onClick={() => handleMenuItemClick(() => onOpenInBrowser(camera))}>
        <ListItemIcon>
          <OpenInBrowserIcon fontSize="small" />
        </ListItemIcon>
        <ListItemText primary={t('context_open_in_browser')} />
      </MenuItem>

      <MenuItem onClick={() => handleMenuItemClick(() => onFileManager(camera))}>
        <ListItemIcon>
          <FolderIcon fontSize="small" />
        </ListItemIcon>
        <ListItemText primary={t('context_file_manager')} />
      </MenuItem>

      <MenuItem onClick={() => handleMenuItemClick(() => onSSH(camera))}>
        <ListItemIcon>
          <TerminalIcon fontSize="small" />
        </ListItemIcon>
        <ListItemText primary={t('context_ssh')} />
      </MenuItem>

      <Divider sx={{ backgroundColor: '#3b4146', margin: '4px 0' }} />

      {/* Управление группами */}
      {onMoveToGroup && [
        <MenuItem key="groups-item" onClick={handleGroupSubmenuOpen}>
          <ListItemIcon>
            <GroupIcon fontSize="small" />
          </ListItemIcon>
          <ListItemText primary="Группы" />
          <ArrowRightIcon fontSize="small" />
        </MenuItem>,
        
        <Menu
          key="groups-submenu"
          anchorEl={groupSubmenuAnchor}
          open={Boolean(groupSubmenuAnchor)}
          onClose={handleGroupSubmenuClose}
          anchorOrigin={{
            vertical: 'top',
            horizontal: 'right',
          }}
          transformOrigin={{
            vertical: 'top',
            horizontal: 'left',
          }}
          PaperProps={{
            sx: {
              backgroundColor: '#2a2f33',
              color: '#fff',
              minWidth: 180,
              '& .MuiMenuItem-root': {
                padding: '8px 16px',
                '&:hover': {
                  backgroundColor: '#3b4146',
                },
              },
            },
          }}
        >
          <MenuItem 
            onClick={() => handleMoveToGroup(null)}
            sx={{ 
              fontStyle: currentGroup ? 'normal' : 'italic',
              backgroundColor: !currentGroup ? '#3b4146' : 'transparent'
            }}
          >
            Без группы
          </MenuItem>
          <Divider sx={{ backgroundColor: '#3b4146', margin: '4px 0' }} />
          {groups.map((group) => (
            <MenuItem 
              key={group.id}
              onClick={() => handleMoveToGroup(group.id)}
              sx={{ 
                backgroundColor: currentGroup?.id === group.id ? '#3b4146' : 'transparent'
              }}
            >
              <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
                {group.color && (
                  <Box
                    sx={{
                      width: 12,
                      height: 12,
                      borderRadius: '50%',
                      backgroundColor: group.color,
                    }}
                  />
                )}
                {group.name}
              </Box>
            </MenuItem>
          ))}
        </Menu>,
        
        <Divider key="groups-divider" sx={{ backgroundColor: '#3b4146', margin: '4px 0' }} />
      ]}

      <MenuItem onClick={() => handleMenuItemClick(() => onEdit(camera))}>
        <ListItemIcon>
          <EditIcon fontSize="small" />
        </ListItemIcon>
        <ListItemText primary={t('context_edit')} />
      </MenuItem>

      <MenuItem onClick={() => handleMenuItemClick(() => onDelete(camera))}>
        <ListItemIcon>
          <DeleteIcon fontSize="small" />
        </ListItemIcon>
        <ListItemText primary={t('context_delete')} />
      </MenuItem>
    </Menu>
  );
};