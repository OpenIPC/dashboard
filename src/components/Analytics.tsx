import React, { useEffect, useMemo, useState } from 'react';
import { Box, Button, Paper, Stack, Tab, Tabs, Typography } from '@mui/material';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import { useNavigate, useSearchParams } from 'react-router-dom';
import FaceSnapshotsPanel from './analytics/FaceSnapshotsPanel';
import ObjectCounterPanel from './analytics/ObjectCounterPanel';
import LicensePlatePanel from './analytics/LicensePlatePanel';
import { useLocalization } from '../hooks/useLocalization';

type TabValue = 'face' | 'objects' | 'plates';

const TAB_VALUES: TabValue[] = ['face', 'objects', 'plates'];

const Analytics: React.FC = () => {
  const { t } = useLocalization();
  const navigate = useNavigate();
  const [searchParams, setSearchParams] = useSearchParams();
  const initialTab = (searchParams.get('view') as TabValue) ?? 'objects';
  const [tab, setTab] = useState<TabValue>(TAB_VALUES.includes(initialTab) ? initialTab : 'face');

  useEffect(() => {
    const viewParam = searchParams.get('view');
    if (viewParam && TAB_VALUES.includes(viewParam as TabValue) && viewParam !== tab) {
      setTab(viewParam as TabValue);
    }
  }, [searchParams, tab]);

  useEffect(() => {
    if (!searchParams.get('view')) {
      const next = new URLSearchParams(searchParams);
      next.set('view', tab);
      setSearchParams(next, { replace: true });
    }
  }, [searchParams, setSearchParams, tab]);

  const handleTabChange = (_event: React.SyntheticEvent, value: TabValue) => {
    setTab(value);
    const next = new URLSearchParams(searchParams);
    next.set('view', value);
    setSearchParams(next, { replace: true });
  };

  const tabs = useMemo(
    () => [
      {
        value: 'face' as TabValue,
        label: t('analytics_face_tab'),
        description: t('analytics_face_tab_hint'),
      },
      {
        value: 'objects' as TabValue,
        label: t('analytics_object_tab'),
        description: t('analytics_object_tab_hint'),
      },
      {
        value: 'plates' as TabValue,
        label: t('analytics_plate_tab'),
        description: t('analytics_plate_tab_hint'),
      },
    ],
    [t],
  );

  const handleBack = () => {
    if (window.history.length > 1) {
      navigate(-1);
      return;
    }
    navigate('/');
  };

  return (
    <Box sx={{ p: 2, height: '100%', display: 'flex', flexDirection: 'column', gap: 2 }}>
      <Stack direction={{ xs: 'column', sm: 'row' }} spacing={1} alignItems={{ xs: 'flex-start', sm: 'center' }}>
        <Button
          variant="text"
          color="inherit"
          startIcon={<ArrowBackIcon />}
          onClick={handleBack}
          sx={{ textTransform: 'none', minWidth: 0, px: 0.5 }}
        >
          {t('snapshots_back_button')}
        </Button>
        <Box>
          <Typography variant="h4" gutterBottom sx={{ mb: 0 }}>
            {t('analytics_viewer_title')}
          </Typography>
          <Typography variant="body2" color="text.secondary">
            {t('analytics_viewer_subtitle')}
          </Typography>
        </Box>
      </Stack>

      <Paper variant="outlined" sx={{ px: 1.5 }}>
        <Tabs
          value={tab}
          onChange={handleTabChange}
          variant="scrollable"
          scrollButtons="auto"
          sx={{
            minHeight: 44,
            '& .MuiTabs-flexContainer': {
              gap: 0.5,
            },
          }}
        >
          {tabs.map(tabConfig => (
            <Tab
              key={tabConfig.value}
              value={tabConfig.value}
              disableRipple
              sx={{
                textTransform: 'none',
                minHeight: 44,
                alignItems: 'flex-start',
                px: 1.25,
                py: 0.75,
              }}
              label={
                <Box sx={{ textAlign: 'left', lineHeight: 1.2 }}>
                  <Typography variant="body2" fontWeight={600}>
                    {tabConfig.label}
                  </Typography>
                  <Typography variant="caption" color="text.secondary">
                    {tabConfig.description}
                  </Typography>
                </Box>
              }
            />
          ))}
        </Tabs>
      </Paper>

      <Box sx={{ flex: 1, minHeight: 0 }}>
        {tab === 'face' && (
          <Box sx={{ height: '100%', p: 2, pt: 0 }}>
            <FaceSnapshotsPanel variant="embedded" />
          </Box>
        )}
        {tab === 'objects' && (
          <Box sx={{ height: '100%', p: 2, pt: 0 }}>
            <ObjectCounterPanel />
          </Box>
        )}
        {tab === 'plates' && (
          <Box sx={{ height: '100%', p: 2, pt: 0 }}>
            <LicensePlatePanel />
          </Box>
        )}
      </Box>
    </Box>
  );
};

export default Analytics;