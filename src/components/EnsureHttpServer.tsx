import { useState, useEffect } from 'react';
import type { ReactNode } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';

interface EnsureHttpServerProps {
  children: ReactNode;
  onServerStarted?: () => void;
  onServerError?: (error: string) => void;
}

/**
 * Component that ensures the HTTP server is running
 * This can be used as a wrapper around video players that require the HTTP server
 */
const EnsureHttpServer = ({ children, onServerStarted, onServerError }: EnsureHttpServerProps) => {
  const [serverRunning, setServerRunning] = useState(false);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    // Function to check if HTTP server is running
    const checkServer = async () => {
      try {
        setLoading(true);
        // First just check if server is already running
        const isRunning = await invoke<boolean>('check_http_server');
        
        if (isRunning) {
          console.log('HTTP Server already running');
          setServerRunning(true);
          if (onServerStarted) onServerStarted();
        } else {
          console.log('HTTP Server not running, starting...');
          // Try to start the server
          const started = await invoke<boolean>('start_http_server');
          setServerRunning(started);
          if (started && onServerStarted) onServerStarted();
          if (!started) {
            const errMsg = 'Failed to start HTTP server';
            setError(errMsg);
            if (onServerError) onServerError(errMsg);
          }
        }
      } catch (err: unknown) {
        console.error('Error starting HTTP server:', err);
        const errorMessage = err instanceof Error ? err.message : 'Unknown error starting HTTP server';
        setError(errorMessage);
        if (onServerError) onServerError(errorMessage);
      } finally {
        setLoading(false);
      }
    };

    // Listen for events from the backend
    const unlisten1 = listen<null>('http-server-started', () => {
      console.log('HTTP Server started event received');
      setServerRunning(true);
      setLoading(false);
      if (onServerStarted) onServerStarted();
    });

    const unlisten2 = listen<{ error: string }>('http-server-error', (event) => {
      console.error('HTTP Server error event:', event.payload);
      const errorMessage = event.payload.error || 'Unknown error';
      setError(`HTTP Server error: ${errorMessage}`);
      setLoading(false);
      if (onServerError) onServerError(errorMessage);
    });

    // Check the server status
    checkServer();

    // Cleanup listeners
    return () => {
      unlisten1.then(fn => fn());
      unlisten2.then(fn => fn());
    };
  }, [onServerStarted, onServerError]);

  // If loading, show a loading message
  if (loading) {
    return (
      <div className="http-server-loading">
        <p>Preparing streaming server...</p>
      </div>
    );
  }

  // If there's an error, show it
  if (error) {
    return (
      <div className="http-server-error">
        <p>Failed to start streaming server: {error}</p>
        <button onClick={() => window.location.reload()}>Retry</button>
      </div>
    );
  }

  // If the server is running, render children
  if (serverRunning) {
    return <>{children}</>;
  }

  // Default case - should not reach here
  return null;
};

export default EnsureHttpServer;