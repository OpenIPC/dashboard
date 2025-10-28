import { HashRouter as Router, Routes, Route } from 'react-router-dom';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import CssBaseline from '@mui/material/CssBaseline';
import { Box, CircularProgress, Typography } from '@mui/material';
import Dashboard from './components/Dashboard';
import { LocalizationProvider } from './contexts/LocalizationContext';
import { AppStateProvider } from './contexts/AppStateContext';
import { AuthProvider, useAuth } from './contexts/AuthContext';
import Layout from './components/Layout';
import LoginScreen from './components/LoginScreen';
import { useEffect, useState } from 'react';

const theme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#1976d2',
    },
    background: {
      default: '#121212',
      paper: '#1e1e1e',
    },
  },
});

const AuthGate: React.FC = () => {
  const { user, initializing } = useAuth();
  const [debugInfo, setDebugInfo] = useState<string>('');

  useEffect(() => {
    const timer = setTimeout(() => {
      if (initializing) {
        setDebugInfo('App is stuck in initializing state');
      }
    }, 5000);

    return () => clearTimeout(timer);
  }, [initializing]);

  useEffect(() => {
    console.log('AuthGate render:', { user: !!user, initializing });
    if (user) {
      console.log('User authenticated:', user);
    }
  }, [user, initializing]);

  if (initializing) {
    return (
      <Box
        sx={{
          position: 'fixed',
          inset: 0,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          bgcolor: 'rgba(0, 0, 0, 0.85)',
          zIndex: 2000,
          gap: 2,
        }}
      >
        <CircularProgress size={48} />
        <Typography color="white">Loading...</Typography>
        {debugInfo && (
          <Typography color="error" variant="body2">
            {debugInfo}
          </Typography>
        )}
      </Box>
    );
  }

  if (!user) {
    return <LoginScreen />;
  }

  return (
    <AppStateProvider>
      <Router>
        <Layout>
          <Routes>
            <Route path="/" element={<Dashboard />} />
          </Routes>
        </Layout>
      </Router>
    </AppStateProvider>
  );
};

function App() {
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    console.log('App component mounted');
    
    // Добавляем обработчик ошибок
    const handleError = (event: ErrorEvent) => {
      console.error('Global error:', event.error);
      setError(event.error?.message || 'Unknown error');
    };

    const handleUnhandledRejection = (event: PromiseRejectionEvent) => {
      console.error('Unhandled promise rejection:', event.reason);
      setError(event.reason?.message || 'Promise rejection');
    };

    window.addEventListener('error', handleError);
    window.addEventListener('unhandledrejection', handleUnhandledRejection);

    return () => {
      window.removeEventListener('error', handleError);
      window.removeEventListener('unhandledrejection', handleUnhandledRejection);
    };
  }, []);

  if (error) {
    return (
      <Box
        sx={{
          position: 'fixed',
          inset: 0,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          bgcolor: '#121212',
          color: 'white',
          p: 3,
        }}
      >
        <Typography variant="h5" color="error" gutterBottom>
          Application Error
        </Typography>
        <Typography variant="body1" sx={{ mb: 2 }}>
          {error}
        </Typography>
        <button
          onClick={() => {
            setError(null);
            window.location.reload();
          }}
          style={{
            padding: '8px 16px',
            backgroundColor: '#1976d2',
            color: 'white',
            border: 'none',
            borderRadius: '4px',
            cursor: 'pointer',
          }}
        >
          Reload Application
        </button>
      </Box>
    );
  }

  return (
    <LocalizationProvider>
      <AuthProvider>
        <ThemeProvider theme={theme}>
          <CssBaseline />
          <AuthGate />
        </ThemeProvider>
      </AuthProvider>
    </LocalizationProvider>
  );
}

export default App;
