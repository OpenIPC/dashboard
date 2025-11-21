import React, { useCallback, useEffect, useRef } from 'react';
import {
  Box,
  IconButton,
  Tooltip
} from '@mui/material';
import {
  Add as AddIcon
} from '@mui/icons-material';
import { useLocalization } from '../hooks/useLocalization';
import type { LayoutTab } from '../types';

interface LayoutTabsProps {
  tabs: LayoutTab[];
  activeTabId: string | null;
  onTabChange: (tabId: string) => void;
  onTabClose: (tabId: string) => void;
  onNewTab: () => void;
  onManageLayouts: () => void;
}

const LayoutTabs: React.FC<LayoutTabsProps> = ({
  tabs,
  activeTabId,
  onTabChange,
  onTabClose,
  onNewTab,
  onManageLayouts
}) => {
  const { t } = useLocalization();
  const tabsContainerRef = useRef<HTMLDivElement>(null);
  const tabCount = tabs.length;

  const handleTabClick = useCallback((tabId: string) => {
    onTabChange(tabId);
  }, [onTabChange]);

  const handleTabClose = useCallback((event: React.MouseEvent | MouseEvent, tabId: string) => {
    event.stopPropagation();
    if (tabCount <= 1) {
      return; // Не разрешаем удалять последнюю вкладку
    }
    if (window.confirm(t('confirm_delete_layout') || 'Вы уверены, что хотите удалить эту вкладку?')) {
      onTabClose(tabId);
    }
  }, [onTabClose, t, tabCount]);

  // Обновляем DOM при изменении вкладок
  useEffect(() => {
    if (!tabsContainerRef.current) return;

    const container = tabsContainerRef.current;
    container.innerHTML = '';

    // Показываем все вкладки
    const visibleTabs = tabs;

    visibleTabs.forEach((tab) => {
      const tabElement = document.createElement('button');
      tabElement.className = `tab ${tab.id === activeTabId ? 'active' : ''}`;
      tabElement.dataset.layoutId = tab.id;
      
      tabElement.innerHTML = `
        <span>${tab.name}</span>
        ${tabs.length > 1 ? '<span class="close-tab-btn">×</span>' : ''}
      `;

      // Обработчик клика по вкладке
      tabElement.addEventListener('click', (e) => {
        e.preventDefault();
        handleTabClick(tab.id);
      });

      // Обработчик закрытия вкладки
      const closeBtn = tabElement.querySelector<HTMLElement>('.close-tab-btn');
      if (closeBtn) {
        closeBtn.addEventListener('click', (event) => {
          event.stopPropagation();
          handleTabClose(event, tab.id);
        });
      }

      container.appendChild(tabElement);
    });
  }, [tabs, activeTabId, t, handleTabClick, handleTabClose]);

  return (
    <Box
      sx={{
        bgcolor: '#2c3037',
        display: 'flex',
        alignItems: 'center',
        minHeight: 42,
        px: 1.5,
        background: 'linear-gradient(180deg, #363940 0%, #2c3037 100%)',
        borderBottom: '1px solid #1e2125',
        '& .tab': {
          minHeight: '32px',
          minWidth: '80px',
          maxWidth: '180px',
          border: '1px solid #404550',
          borderRight: 'none',
          backgroundColor: '#363940',
          color: 'rgba(255,255,255,0.7)',
          cursor: 'pointer',
          display: 'inline-flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          padding: '4px 8px',
          fontSize: '0.8rem',
          fontWeight: '500',
          fontFamily: 'inherit',
          outline: 'none',
          position: 'relative',
          margin: 0,
          '&:last-child': {
            borderRight: '1px solid #404550'
          },
          '&.active': {
            backgroundColor: '#4a5058',
            color: '#fff',
            borderColor: '#5a6068',
            zIndex: 1,
            '&::after': {
              content: '""',
              position: 'absolute',
              bottom: '-1px',
              left: 0,
              right: 0,
              height: '2px',
              backgroundColor: '#4a90e2'
            }
          },
          '&:hover:not(.active)': {
            backgroundColor: '#3a3e45',
            borderColor: '#4a5058'
          },
          '& .close-tab-btn': {
            marginLeft: '8px',
            padding: '2px',
            borderRadius: '2px',
            color: 'rgba(255,255,255,0.5)',
            cursor: 'pointer',
            '&:hover': {
              color: 'rgba(255,255,255,0.9)',
              backgroundColor: 'rgba(255,255,255,0.1)'
            }
          }
        }
      }}
    >
      {/* Контейнер для вкладок */}
      <Box 
        ref={tabsContainerRef}
        className="tabs"
        sx={{ 
          flex: 1, 
          display: 'flex', 
          alignItems: 'center'
        }}
      />

      {/* Кнопка добавления новой вкладки */}
      <Tooltip title={t('new_layout_tab')}>
        <IconButton
          size="medium"
          onClick={onNewTab}
          sx={{
            ml: 0.5,
            color: '#f3f8ff',
            border: '1px solid #3a4a60',
            borderRadius: '6px',
            width: 40,
            height: 40,
            mt: '-10px',
            background: 'linear-gradient(180deg, #3b4f68 0%, #2c3f54 100%)',
            boxShadow: 'inset 0 0 0 1px rgba(255,255,255,0.1)',
            display: 'inline-flex',
            alignItems: 'center',
            justifyContent: 'center',
            '&:hover': {
              color: '#fff',
              bgcolor: '#3f5a78',
              borderColor: '#3a5981'
            }
          }}
        >
          <AddIcon fontSize="medium" />
        </IconButton>
      </Tooltip>

      <Tooltip title={t('manage_templates')}>
        <IconButton
          size="medium"
          onClick={onManageLayouts}
          sx={{
            ml: 0.5,
            color: '#fff',
            border: '1px solid #2f3f54',
            borderRadius: '6px',
            width: 40,
            height: 40,
            mt: '-10px',
            px: 0,
            background: 'linear-gradient(180deg, #33435a 0%, #253448 100%)',
            boxShadow: 'inset 0 0 0 1px rgba(255,255,255,0.12)',
            display: 'inline-flex',
            alignItems: 'center',
            justifyContent: 'center',
            '&:hover': {
              bgcolor: '#3e5980',
              borderColor: '#40638f'
            }
          }}
        >
          <Box
            component="span"
            sx={{
              display: 'inline-flex',
              alignItems: 'center',
              justifyContent: 'center'
            }}
          >
            <svg
              width="26"
              height="26"
              viewBox="0 0 16 16"
              fill="none"
              xmlns="http://www.w3.org/2000/svg"
            >
              <rect x="1.5" y="1.5" width="13" height="13" rx="2" stroke="white" strokeWidth="1.2" />
              <rect x="4" y="4" width="3" height="3" rx="0.6" fill="white" />
              <rect x="9" y="4" width="3" height="3" rx="0.6" fill="white" />
              <rect x="4" y="9" width="3" height="3" rx="0.6" fill="white" />
              <rect x="9" y="9" width="3" height="3" rx="0.6" fill="white" />
            </svg>
          </Box>
        </IconButton>
      </Tooltip>
    </Box>
  );
};

export default LayoutTabs;