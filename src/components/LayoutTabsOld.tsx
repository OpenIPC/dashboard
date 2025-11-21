import React from 'react';
import {
  Box,
  Tab,
  Tabs,
  IconButton,
  Menu,
  MenuItem,
  Tooltip,
  Typography
} from '@mui/material';
import {
  Add as AddIcon,
  Close as CloseIcon,
  MoreVert as MoreVertIcon,
  Save as SaveIcon,
  FolderOpen as LoadIcon
} from '@mui/icons-material';
import { useLocalization } from '../hooks/useLocalization';
import type { LayoutTab } from '../types';

interface LayoutTabsProps {
  tabs: LayoutTab[];
  activeTabId: string | null;
  onTabChange: (tabId: string) => void;
  onTabClose: (tabId: string) => void;
  onNewTab: () => void;
  onSaveTemplate: () => void;
  onLoadTemplate: () => void;
  onManageTemplates: () => void;
}

const LayoutTabs: React.FC<LayoutTabsProps> = ({
  tabs,
  activeTabId,
  onTabChange,
  onTabClose,
  onNewTab,
  onSaveTemplate,
  onLoadTemplate,
  onManageTemplates
}) => {
  const { t } = useLocalization();
  const [menuAnchorEl, setMenuAnchorEl] = React.useState<null | HTMLElement>(null);

  // РџСЂРѕРІРµСЂСЏРµРј, РµСЃС‚СЊ Р»Рё РєРѕРЅС‚РµРЅС‚ РІРѕ РІРєР»Р°РґРєРµ (РєР°РјРµСЂС‹)
  const hasContentInTab = (tab: LayoutTab) => {
    return tab.template && tab.template.cameraAssignments && tab.template.cameraAssignments.length > 0;
  };

  const handleMenuOpen = (event: React.MouseEvent<HTMLElement>) => {
    setMenuAnchorEl(event.currentTarget);
  };

  const handleMenuClose = () => {
    setMenuAnchorEl(null);
  };

  const handleSaveTemplate = () => {
    handleMenuClose();
    onSaveTemplate();
  };

  const handleLoadTemplate = () => {
    handleMenuClose();
    onLoadTemplate();
  };

  const handleManageTemplates = () => {
    handleMenuClose();
    onManageTemplates();
  };

  return (
    <Box
      sx={{
        bgcolor: '#2c3037',
        display: 'flex',
        alignItems: 'center',
        minHeight: 42,
        px: 1.5,
        background: 'linear-gradient(180deg, #363940 0%, #2c3037 100%)',
        borderBottom: '1px solid #1e2125'
      }}
    >
      {/* Р’РєР»Р°РґРєРё Рё РєРЅРѕРїРєР° РґРѕР±Р°РІР»РµРЅРёСЏ */}
      <Box sx={{ flex: 1, display: 'flex', alignItems: 'center' }}>
        {tabs.length > 0 ? (
          <>
            <Tabs
              key={`tabs-${tabs.length}-${activeTabId}`}
              value={activeTabId || false}
              onChange={(_, tabId) => onTabChange(tabId)}
              variant="scrollable"
              scrollButtons="auto"
              sx={{
                '& .MuiTabs-indicator': {
                  display: 'none'
                },
                '& .MuiTabs-scrollButtons': {
                  color: 'rgba(255,255,255,0.6)',
                  '&.Mui-disabled': {
                    opacity: 0.3
                  }
                },
                '& .MuiTabs-flexContainer': {
                  gap: 0
                },
                '& .MuiTabs-scroller': {
                  overflow: 'visible !important'
                }
              }}
            >
              {tabs.filter(tab => tab.isActive || hasContentInTab(tab)).map((tab) => (
                <Tab
                  key={tab.id}
                  value={tab.id}
                  label={
                    <Box sx={{ 
                      display: 'flex', 
                      alignItems: 'center', 
                      gap: 0.5,
                      px: 1,
                      py: 0.25,
                      height: 32
                    }}>
                      <Typography
                        variant="body2"
                        sx={{
                          overflow: 'hidden',
                          textOverflow: 'ellipsis',
                          whiteSpace: 'nowrap',
                          maxWidth: 100,
                          fontSize: '0.8rem',
                          fontWeight: 500
                        }}
                      >
                        {tab.name}
                      </Typography>
                      {tabs.length > 1 && (
                        <IconButton
                          size="small"
                          onClick={(e) => {
                            e.stopPropagation();
                            onTabClose(tab.id);
                          }}
                          sx={{
                            color: 'rgba(255,255,255,0.5)',
                            padding: '2px',
                            ml: 0.5,
                            '&:hover': {
                              color: 'rgba(255,255,255,0.9)',
                              bgcolor: 'rgba(255,255,255,0.1)'
                            }
                          }}
                        >
                          <CloseIcon sx={{ fontSize: 14 }} />
                        </IconButton>
                      )}
                    </Box>
                  }
                  sx={{
                    minHeight: 32,
                    minWidth: 80,
                    maxWidth: 180,
                    textTransform: 'none',
                    borderRadius: 0,
                    mx: 0,
                    py: 0,
                    position: 'relative',
                    border: '1px solid #404550',
                    borderRight: 'none',
                    '&:last-child': {
                      borderRight: '1px solid #404550'
                    },
                    '&.Mui-selected': {
                      bgcolor: '#4a5058',
                      color: '#fff',
                      borderColor: '#5a6068',
                      zIndex: 1,
                      '&::after': {
                        content: '""',
                        position: 'absolute',
                        bottom: -1,
                        left: 0,
                        right: 0,
                        height: '2px',
                        bgcolor: '#4a90e2'
                      }
                    },
                    '&:hover:not(.Mui-selected)': {
                      bgcolor: '#3a3e45',
                      borderColor: '#4a5058'
                    },
                    color: 'rgba(255,255,255,0.7)',
                    bgcolor: '#363940'
                  }}
                />
              ))}
            </Tabs>
            
            {/* Кнопка добавления новой вкладки рядом с вкладками */}
            <Tooltip title={t('new_layout_tab')}>
              <IconButton
                size="small"
                onClick={onNewTab}
                sx={{
                  ml: 0.5,
                  color: 'rgba(255,255,255,0.7)',
                  border: '1px solid #404550',
                  borderRadius: 0,
                  minWidth: 28,
                  width: 28,
                  height: 32,
                  '&:hover': {
                    color: '#fff',
                    bgcolor: '#3a3e45',
                    borderColor: '#4a5058'
                  }
                }}
              >
                <AddIcon fontSize="small" />
              </IconButton>
            </Tooltip>
          </>
        ) : (
          <>
            <Typography variant="body2" color="rgba(255,255,255,0.5)" sx={{ ml: 2 }}>
              {t('no_layout_tabs')}
            </Typography>
            <Tooltip title={t('new_layout_tab')}>
              <IconButton
                size="small"
                onClick={onNewTab}
                sx={{
                  ml: 2,
                  color: 'rgba(255,255,255,0.7)',
                  border: '1px solid #404550',
                  borderRadius: 0,
                  width: 32,
                  height: 36,
                  '&:hover': {
                    color: '#fff',
                    bgcolor: '#3a3e45',
                    borderColor: '#4a5058'
                  }
                }}
              >
                <AddIcon fontSize="small" />
              </IconButton>
            </Tooltip>
          </>
        )}
      </Box>

      {/* РљРЅРѕРїРєРё СѓРїСЂР°РІР»РµРЅРёСЏ */}
      <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
        <Tooltip title={t('layout_options')}>
          <IconButton
            size="small"
            onClick={handleMenuOpen}
            sx={{
              color: 'rgba(255,255,255,0.7)',
              '&:hover': {
                color: '#fff',
                bgcolor: 'rgba(255,255,255,0.1)'
              }
            }}
          >
            <MoreVertIcon fontSize="small" />
          </IconButton>
        </Tooltip>

        <Menu
          anchorEl={menuAnchorEl}
          open={Boolean(menuAnchorEl)}
          onClose={handleMenuClose}
          PaperProps={{
            sx: {
              bgcolor: '#2a2d31',
              border: '1px solid rgba(255,255,255,0.1)',
              '& .MuiMenuItem-root': {
                color: 'rgba(255,255,255,0.87)',
                '&:hover': {
                  bgcolor: 'rgba(255,255,255,0.1)'
                }
              }
            }
          }}
        >
          <MenuItem onClick={handleSaveTemplate}>
            <SaveIcon sx={{ mr: 1, fontSize: 18 }} />
            {t('save_current_layout')}
          </MenuItem>
          <MenuItem onClick={handleLoadTemplate}>
            <LoadIcon sx={{ mr: 1, fontSize: 18 }} />
            {t('load_saved_layout')}
          </MenuItem>
          <MenuItem onClick={handleManageTemplates}>
            <MoreVertIcon sx={{ mr: 1, fontSize: 18 }} />
            {t('layout_templates')}
          </MenuItem>
        </Menu>
      </Box>
    </Box>
  );
};

export default LayoutTabs;

