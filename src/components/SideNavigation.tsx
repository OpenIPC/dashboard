import React from 'react';
import { Link as RouterLink, useLocation } from 'react-router-dom';
import { 
  Box, 
  List, 
  ListItem, 
  ListItemButton, 
  ListItemIcon, 
  ListItemText,
  Divider
} from '@mui/material';
import { 
  Dashboard as DashboardIcon,
  Videocam as VideocamIcon,
  VideoCall as VideoCallIcon,
  VideoLibrary as VideoLibraryIcon,
  Analytics as AnalyticsIcon,
  TerminalOutlined as TerminalIcon,
  FolderOpen as FolderIcon,
  Settings as SettingsIcon,
  BugReport as BugReportIcon
} from '@mui/icons-material';
import { useLocalization } from '../contexts/LocalizationContext';

const SideNavigation: React.FC = () => {
  const location = useLocation();
  const { t } = useLocalization();
  
  // Navigation items configuration
  const navigationItems = [
    { 
      path: '/', 
      icon: <DashboardIcon />, 
      label: t('dashboard') 
    },
    { 
      path: '/cameras', 
      icon: <VideocamIcon />, 
      label: t('cameras_mediamtx') 
    },
    { 
      path: '/direct-cameras', 
      icon: <VideoCallIcon />, 
      label: t('direct_connection') 
    },
    { 
      path: '/recordings', 
      icon: <VideoLibraryIcon />, 
      label: t('recordings') 
    },
    { 
      path: '/analytics', 
      icon: <AnalyticsIcon />, 
      label: t('analytics') 
    },
    { 
      path: '/terminal', 
      icon: <TerminalIcon />, 
      label: t('terminal') 
    },
    { 
      path: '/files', 
      icon: <FolderIcon />, 
      label: t('files') 
    },
    { 
      path: '/settings', 
      icon: <SettingsIcon />, 
      label: t('settings') 
    },
    { 
      path: '/rtsp-test', 
      icon: <BugReportIcon />, 
      label: t('rtsp_test') 
    }
  ];

  return (
    <Box 
      sx={{ 
        width: '220px', 
        bgcolor: 'background.paper',
        borderRight: '1px solid',
        borderColor: 'divider',
        height: '100%',
        overflowY: 'auto'
      }}
    >
      <List component="nav">
        {navigationItems.map((item, index) => (
          <React.Fragment key={item.path}>
            {index === 3 && <Divider />}
            <ListItem disablePadding>
              <ListItemButton 
                component={RouterLink} 
                to={item.path}
                selected={location.pathname === item.path}
                sx={{
                  '&.Mui-selected': {
                    bgcolor: 'rgba(25, 118, 210, 0.15)',
                    '&:hover': {
                      bgcolor: 'rgba(25, 118, 210, 0.25)'
                    }
                  }
                }}
              >
                <ListItemIcon>
                  {item.icon}
                </ListItemIcon>
                <ListItemText primary={item.label} />
              </ListItemButton>
            </ListItem>
          </React.Fragment>
        ))}
      </List>
    </Box>
  );
};

export default SideNavigation;