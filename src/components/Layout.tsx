
import React, { useState, useEffect } from 'react';
import { check } from '@tauri-apps/plugin-updater';
import { ask } from '@tauri-apps/plugin-dialog';
import { relaunch } from '@tauri-apps/plugin-process';

import DevicePanel from './DevicePanel';
import AppFooter from './AppFooter';
import { Box, Button, Typography } from '@mui/material';
import { CameraContextMenuProvider } from '../contexts/CameraContextMenuContext';
import LogViewerModal from './LogViewerModal';
// Динамический импорт Tauri window API

interface LayoutProps {
  children: React.ReactNode;
}

const Layout: React.FC<LayoutProps> = ({ children }) => {
  // left navigation removed per request
  // Удалён динамический импорт, используем прямой импорт appWindow
  const [isSimMaximized, setIsSimMaximized] = useState(false);
  const [isSimMinimized, setIsSimMinimized] = useState(false);
  const [isSimClosed, setIsSimClosed] = useState(false);

  useEffect(() => {
    const checkForUpdates = async () => {
      try {
        const update = await check();
        if (update?.available) {
          const yes = await ask(
            `New version ${update.version} is available!\n\n${update.body || 'No release notes.'}\n\nDo you want to install it now?`, 
            { title: 'Update Available', kind: 'info', okLabel: 'Install & Restart', cancelLabel: 'Later' }
          );
          if (yes) {
            await update.downloadAndInstall();
            await relaunch();
          }
        }
      } catch (error) {
        console.error('Failed to check for updates:', error);
      }
    };

    checkForUpdates();
  }, []);

  return (
    <CameraContextMenuProvider>
      <Box sx={{ display: 'flex', minHeight: '100vh', bgcolor: '#23272b' }}>
        {/* Левая панель навигации полностью удалена */}
        
        <Box sx={{ flexGrow: 1, display: 'flex', flexDirection: 'column', minHeight: '100vh', bgcolor: '#23272b', position: 'relative' }}>
          {/* Системная шапка окна используется по умолчанию. Кастомная шапка удалена. */}
          {/* In-browser simulated closed overlay */}
          {isSimClosed && (
            <Box sx={{ position: 'fixed', left: 0, right: 0, top: 0, bottom: 0, bgcolor: '#23272b', zIndex: 2000, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', gap: 2 }}>
              <Typography variant="h5" sx={{ color: '#fff' }}>App closed (dev mode)</Typography>
              <Button variant="contained" onClick={() => { setIsSimClosed(false); setIsSimMinimized(false); setIsSimMaximized(false); }} sx={{ bgcolor: '#50545a', color: '#fff' }}>Reopen</Button>
            </Box>
          )}
          <Box
            component="main"
            sx={{
              flexGrow: 1,
              p: 0,
              bgcolor: '#23272b',
              minHeight: 0,
              width: '100%',
              display: 'flex',
              flexDirection: 'column',
            }}
          >
            <Box
              sx={
                isSimMinimized
                  ? { display: 'none' }
                  : isSimMaximized
                  ? {
                      position: 'fixed',
                      left: 0,
                      right: 0,
                      top: 0,
                      bottom: 0,
                      p: 2,
                      boxSizing: 'border-box',
                      display: 'grid',
                      gridTemplateColumns: '1fr 340px',
                      gap: 2,
                      bgcolor: '#23272b',
                      zIndex: 1300,
                    }
                  : {
                      display: 'grid',
                      gridTemplateColumns: '1fr 340px',
                      gap: 2,
                      flexGrow: 1,
                      minHeight: 0,
                      height: '100%',
                      p: 2,
                      boxSizing: 'border-box',
                    }
              }
            >
              <Box sx={{ minHeight: 0, overflow: 'hidden' }}>{children}</Box>
              <Box sx={{ minHeight: 0 }}>
                <DevicePanel />
              </Box>
            </Box>
          </Box>
          <AppFooter />
        </Box>
      </Box>
      <LogViewerModal />
    </CameraContextMenuProvider>
  );
};

export default Layout;