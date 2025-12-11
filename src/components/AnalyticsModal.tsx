import React, { useState, useMemo } from 'react';
import {
  Dialog,
  DialogContent,
  DialogTitle,
  IconButton,
  Box,
  Tab,
  Tabs,
  useTheme,
  useMediaQuery
} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import { useLocalization } from '../hooks/useLocalization';
import FaceSnapshotsPanel from './analytics/FaceSnapshotsPanel';
import ObjectCounterPanel from './analytics/ObjectCounterPanel';
import LicensePlatePanel from './analytics/LicensePlatePanel';

interface AnalyticsModalProps {
  open: boolean;
  onClose: () => void;
  initialTab?: 'face' | 'objects' | 'plates';
}

type TabValue = 'face' | 'objects' | 'plates';

const AnalyticsModal: React.FC<AnalyticsModalProps> = ({ open, onClose, initialTab = 'objects' }) => {
  const { t } = useLocalization();
  const theme = useTheme();
  const fullScreen = useMediaQuery(theme.breakpoints.down('md'));
  const [tab, setTab] = useState<TabValue>(initialTab);

  const handleTabChange = (_event: React.SyntheticEvent, value: TabValue) => {
    setTab(value);
  };

  const tabs = useMemo(
    () => [
      {
        value: 'face' as TabValue,
        label: t('analytics_face_tab'),
      },
      {
        value: 'objects' as TabValue,
        label: t('analytics_object_tab'),
      },
      {
        value: 'plates' as TabValue,
        label: t('analytics_plate_tab'),
      },
    ],
    [t],
  );

  return (
    <Dialog
      open={open}
      onClose={onClose}
      fullScreen={fullScreen}
      maxWidth="xl"
      fullWidth
      PaperProps={{
        sx: {
          height: '90vh',
          maxHeight: '90vh',
          display: 'flex',
          flexDirection: 'column',
          bgcolor: '#1e1e1e',
          backgroundImage: 'none',
        },
      }}
    >
      <DialogTitle sx={{ p: 0, borderBottom: '1px solid rgba(255,255,255,0.1)' }}>
        <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', px: 2 }}>
          <Tabs
            value={tab}
            onChange={handleTabChange}
            variant="scrollable"
            scrollButtons="auto"
            sx={{
              minHeight: 48,
              '& .MuiTabs-indicator': {
                backgroundColor: '#22d3ee',
              },
            }}
          >
            {tabs.map(tabConfig => (
              <Tab
                key={tabConfig.value}
                value={tabConfig.value}
                disableRipple
                label={tabConfig.label}
                sx={{ 
                  textTransform: 'none', 
                  minHeight: 48,
                  fontWeight: 600,
                  '&.Mui-selected': {
                    color: '#22d3ee',
                  }
                }}
              />
            ))}
          </Tabs>
          <IconButton onClick={onClose} sx={{ color: 'rgba(255,255,255,0.7)' }}>
            <CloseIcon />
          </IconButton>
        </Box>
      </DialogTitle>
      <DialogContent sx={{ p: 0, display: 'flex', flexDirection: 'column', flex: 1, overflow: 'hidden' }}>
        <Box sx={{ flex: 1, minHeight: 0, overflow: 'auto', display: 'flex', flexDirection: 'column' }}>
          {tab === 'face' && (
            <Box sx={{ flex: 1, p: 2, display: 'flex', flexDirection: 'column' }}>
              <FaceSnapshotsPanel variant="embedded" />
            </Box>
          )}
          {tab === 'objects' && (
            <Box sx={{ flex: 1, p: 2, display: 'flex', flexDirection: 'column' }}>
              <ObjectCounterPanel />
            </Box>
          )}
          {tab === 'plates' && (
            <Box sx={{ flex: 1, p: 2, display: 'flex', flexDirection: 'column' }}>
              <LicensePlatePanel />
            </Box>
          )}
        </Box>
      </DialogContent>
    </Dialog>
  );
};

export default AnalyticsModal;
