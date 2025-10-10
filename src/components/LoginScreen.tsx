import React, { useCallback, useEffect, useState } from 'react';
import {
  Box,
  Button,
  Checkbox,
  CircularProgress,
  FormControlLabel,
  Paper,
  TextField,
  Typography,
} from '@mui/material';
import { useLocalization } from '../contexts/LocalizationContext';
import { useAuth } from '../contexts/AuthContext';

const LoginScreen: React.FC = () => {
  const { t } = useLocalization();
  const { login, authenticating, error, clearError } = useAuth();
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [rememberMe, setRememberMe] = useState(false);
  const [submitted, setSubmitted] = useState(false);

  useEffect(() => {
    if (submitted) {
      clearError();
      setSubmitted(false);
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [username, password]);

  const handleSubmit = useCallback(
    async (event?: React.FormEvent<HTMLFormElement>) => {
      event?.preventDefault();
      clearError();
      if (!username.trim() || !password) {
        setSubmitted(true);
        return;
      }
      await login(username.trim(), password, rememberMe);
    },
    [clearError, login, password, rememberMe, username],
  );

  const validationError = submitted && (!username.trim() || !password)
    ? t('username_and_password_required')
    : null;

  return (
    <Box
      component="form"
      onSubmit={handleSubmit}
      sx={{
        position: 'fixed',
        inset: 0,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        bgcolor: 'rgba(0, 0, 0, 0.8)',
        zIndex: 2000,
      }}
    >
      <Paper
        elevation={6}
        sx={{
          width: 360,
          p: 4,
          display: 'flex',
          flexDirection: 'column',
          gap: 2,
          bgcolor: '#2a2f33',
        }}
      >
        <Typography variant="h6" component="h1" textAlign="center" sx={{ color: '#fff', fontWeight: 600 }}>
          {t('login_prompt')}
        </Typography>
        <TextField
          label={t('login')}
          value={username}
          onChange={(e) => setUsername(e.target.value)}
          autoFocus
          fullWidth
          autoComplete="username"
          variant="outlined"
        />
        <TextField
          label={t('password')}
          type="password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          fullWidth
          autoComplete="current-password"
          variant="outlined"
          onKeyDown={(event) => {
            if (event.key === 'Enter') {
              void handleSubmit();
            }
          }}
        />
        <FormControlLabel
          control={
            <Checkbox
              checked={rememberMe}
              onChange={(e) => setRememberMe(e.target.checked)}
              color="primary"
            />
          }
          label={t('remember_me')}
        />
        <Typography
          variant="body2"
          sx={{
            color: 'rgba(255, 255, 255, 0.7)',
            textAlign: 'center',
            fontSize: 13,
            lineHeight: 1.4,
          }}
        >
          {t('default_credentials_hint')}
        </Typography>
        {(error || validationError) && (
          <Typography variant="body2" color="error" textAlign="center">
            {error || validationError}
          </Typography>
        )}
        <Button
          type="submit"
          variant="contained"
          color="primary"
          fullWidth
          disabled={authenticating}
          sx={{ mt: 1, fontWeight: 600 }}
        >
          {authenticating ? <CircularProgress size={22} color="inherit" /> : t('login_btn')}
        </Button>
      </Paper>
    </Box>
  );
};

export default LoginScreen;
