import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import {
  Typography,
  Box,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  CircularProgress,
} from '@mui/material';
import { AnsiUp } from 'ansi_up';
import type { Camera } from '../types';

const escapeHtml = (value: string): string =>
  value
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');

interface TerminalProps {
  open: boolean;
  camera: Camera | null;
  onClose: () => void;
}

type SshShellOpenResponse = {
  sessionId: string;
  output: string;
};

type SshShellDataResponse = {
  output: string;
};

const TerminalComponent: React.FC<TerminalProps> = ({ open, camera, onClose }) => {
  const [sessionId, setSessionId] = useState<string | null>(null);
  const sessionIdRef = useRef<string | null>(null);
  const [chunks, setChunks] = useState<string[]>([]);
  const [currentInput, setCurrentInput] = useState('');
  const currentInputRef = useRef('');
  const inputCaptureRef = useRef<HTMLTextAreaElement | null>(null);
  const [isConnecting, setIsConnecting] = useState(false);
  const [isExecuting, setIsExecuting] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const outputRef = useRef<HTMLDivElement | null>(null);
  const stickToBottomRef = useRef(true);

  const ansi = useMemo(() => new AnsiUp(), []);

  const updateSessionId = useCallback((value: string | null) => {
    sessionIdRef.current = value;
    setSessionId(value);
  }, []);

  useEffect(() => {
    currentInputRef.current = currentInput;
  }, [currentInput]);

  const connectionSummary = useMemo(() => {
    if (!camera) {
      return '';
    }
    const username = camera.user?.trim() || 'root';
    return `${username}@${camera.ip}`;
  }, [camera]);

  const terminateSession = useCallback(async (explicitId?: string | null) => {
    const activeId = explicitId ?? sessionIdRef.current;
    if (!activeId) {
      return;
    }

    try {
      await invoke<boolean>('camera_ssh_shell_close', { sessionId: activeId });
    } catch (closeError) {
      console.warn('Failed to close SSH shell session', closeError);
    } finally {
      if (sessionIdRef.current === activeId) {
        updateSessionId(null);
      }
    }
  }, [updateSessionId]);

  const connect = useCallback(async () => {
    if (!camera) {
      return;
    }

    setIsConnecting(true);
    setError(null);
    setChunks([]);
    setCurrentInput('');
    stickToBottomRef.current = true;

    await terminateSession();

    try {
      const response = await invoke<SshShellOpenResponse>('camera_ssh_shell_open', {
        host: camera.ip,
        username: camera.user || null,
        passwordEnc: camera.pass_enc || null,
        passwordPlain: camera.pass || null,
        port:
          camera.port && camera.port !== 0 && camera.port !== 554 ? camera.port : null,
      });

      updateSessionId(response.sessionId);
      if (response.output) {
        setChunks([response.output]);
      }
    } catch (connectError) {
      const message = connectError instanceof Error ? connectError.message : String(connectError);
      setError(message);
      updateSessionId(null);
    } finally {
      setIsConnecting(false);
    }
  }, [camera, terminateSession, updateSessionId]);

  useEffect(() => {
    if (open && camera) {
      void connect();
    }

    if (!open) {
      void terminateSession();
      setChunks([]);
      setCurrentInput('');
      setIsExecuting(false);
      setIsConnecting(false);
      setError(null);
      stickToBottomRef.current = true;
    }
  }, [open, camera, connect, terminateSession]);

  useEffect(() => {
    return () => {
      void terminateSession();
    };
  }, [terminateSession]);

  const scrollToBottomIfNeeded = useCallback(() => {
    const node = outputRef.current;
    if (!node || !stickToBottomRef.current) {
      return;
    }

    node.scrollTop = node.scrollHeight;
  }, []);

  useEffect(() => {
    scrollToBottomIfNeeded();
  }, [chunks, scrollToBottomIfNeeded]);

  useEffect(() => {
    scrollToBottomIfNeeded();
  }, [currentInput, scrollToBottomIfNeeded]);

  const handleScroll = useCallback(() => {
    const node = outputRef.current;
    if (!node) {
      return;
    }

    const distanceFromBottom = node.scrollHeight - (node.scrollTop + node.clientHeight);
    stickToBottomRef.current = distanceFromBottom <= 32;
  }, []);

  const focusCapture = useCallback(() => {
    inputCaptureRef.current?.focus();
  }, []);

  useEffect(() => {
    if (open && sessionId && !isConnecting) {
      const timer = window.setTimeout(() => {
        focusCapture();
      }, 0);

      return () => {
        window.clearTimeout(timer);
      };
    }

    return undefined;
  }, [open, sessionId, isConnecting, focusCapture]);

  const runCommand = useCallback(async () => {
    const activeId = sessionIdRef.current;
    if (!activeId) {
      return;
    }

    const inputValue = currentInputRef.current;
    stickToBottomRef.current = true;
    setCurrentInput('');

    if (inputValue.length === 0) {
      try {
        const response = await invoke<SshShellDataResponse>('camera_ssh_shell_send', {
          sessionId: activeId,
          command: '',
        });
        if (response.output) {
          setChunks(prev => [...prev, response.output]);
        }
      } catch (executeError) {
        const message = executeError instanceof Error ? executeError.message : String(executeError);
        setChunks(prev => [...prev, `\u001b[31mError:\u001b[0m ${message}`]);
      }
      return;
    }

    setIsExecuting(true);

    try {
      const response = await invoke<SshShellDataResponse>('camera_ssh_shell_send', {
        sessionId: activeId,
        command: inputValue,
      });

      if (response.output) {
        setChunks(prev => [...prev, response.output]);
      }
    } catch (executeError) {
      const message = executeError instanceof Error ? executeError.message : String(executeError);
      setChunks(prev => [...prev, `\u001b[31mError:\u001b[0m ${message}`]);
    } finally {
      setIsExecuting(false);
    }
  }, []);

  const handleKeyDown = (event: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (event.key === 'Enter' && !event.shiftKey) {
      event.preventDefault();
      void runCommand();
    }

    if (event.key === 'Tab') {
      event.preventDefault();
      const { selectionStart, selectionEnd, value } = event.currentTarget;
      const updated = `${value.slice(0, selectionStart)}\t${value.slice(selectionEnd)}`;
      setCurrentInput(updated);

      requestAnimationFrame(() => {
        const node = inputCaptureRef.current;
        if (node) {
          const caret = selectionStart + 1;
          node.selectionStart = caret;
          node.selectionEnd = caret;
        }
      });
    }
  };

  const handleInputChange = (event: React.ChangeEvent<HTMLTextAreaElement>) => {
    stickToBottomRef.current = true;
    setCurrentInput(event.target.value);
  };

  const handleClose = useCallback(() => {
    if (!isExecuting) {
      void terminateSession();
      onClose();
    }
  }, [isExecuting, onClose, terminateSession]);

  const renderTerminalContent = () => {
    if (isConnecting) {
      return (
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          <CircularProgress size={18} />
          <Typography variant="body2" color="grey.400">
            Establishing SSH session...
          </Typography>
        </Box>
      );
    }

    if (error) {
      return (
        <Typography variant="body2" color="error">
          {error}
        </Typography>
      );
    }

    if (!sessionId && !chunks.length) {
      return (
        <Typography variant="body2" color="grey.400">
          Waiting for session...
        </Typography>
      );
    }

    const caretHtml = '<span class="terminal-caret">&nbsp;</span>';
    const inputHtml = currentInput.length > 0 ? escapeHtml(currentInput) : '';

    return (
      <>
        {chunks.map((chunk, index) => (
          <Box
            component="div"
            // eslint-disable-next-line react/no-danger
            dangerouslySetInnerHTML={{ __html: ansi.ansi_to_html(chunk || ' ') }}
            key={`terminal-chunk-${index}`}
          />
        ))}
        <Box
          component="span"
          // eslint-disable-next-line react/no-danger
          dangerouslySetInnerHTML={{ __html: `${inputHtml}${caretHtml}` }}
          className="terminal-input-line"
          sx={{ display: 'inline' }}
        />
      </>
    );
  };

  return (
    <Dialog open={open} onClose={handleClose} maxWidth="md" fullWidth>
      <DialogTitle
        sx={{
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between',
          py: 0.75,
          px: 2,
          typography: 'subtitle2',
          fontWeight: 500,
        }}
      >
        <Typography component="span" variant="subtitle2" sx={{ fontSize: '0.9rem' }}>
          SSH Terminal{' '}
          {connectionSummary && (
            <Typography component="span" variant="subtitle2" sx={{ fontSize: '0.75rem', opacity: 0.7 }}>
              ({connectionSummary})
            </Typography>
          )}
        </Typography>
        <Button
          onClick={handleClose}
          color="inherit"
          size="small"
          sx={{ minWidth: 26, fontSize: '0.95rem', height: 22, lineHeight: 1 }}
        >
          ×
        </Button>
      </DialogTitle>
      <DialogContent dividers>
        <Box
          ref={outputRef}
          sx={{
            position: 'relative',
            backgroundColor: '#000',
            color: '#f1f1f1',
            p: 2,
            borderRadius: 1,
            fontFamily: 'monospace',
            fontSize: '0.9rem',
            height: 540,
            overflowY: 'auto',
            mb: 2,
            whiteSpace: 'pre-wrap',
            cursor: sessionId ? 'text' : 'default',
            '& .terminal-input-line': {
              color: '#f1f1f1',
            },
            '& .terminal-caret': {
              display: 'inline-block',
              width: '0.6ch',
              height: '1em',
              backgroundColor: '#f1f1f1',
              marginLeft: '-0.1ch',
              animation: 'terminalCaretBlink 1.2s steps(1) infinite',
            },
            '@keyframes terminalCaretBlink': {
              '0%,50%': { opacity: 1 },
              '50%,100%': { opacity: 0 },
            },
          }}
          onClick={focusCapture}
          onScroll={handleScroll}
        >
          {renderTerminalContent()}
          <textarea
            ref={inputCaptureRef}
            value={currentInput}
            onKeyDown={handleKeyDown}
            onChange={handleInputChange}
            spellCheck={false}
            style={{
              position: 'fixed',
              opacity: 0,
              pointerEvents: 'none',
              width: '1px',
              height: '1px',
              top: -9999,
              left: -9999,
              fontFamily: 'monospace',
              fontSize: '1rem',
              background: 'transparent',
              border: 'none',
              color: 'transparent',
              caretColor: 'transparent',
              resize: 'none',
            }}
            tabIndex={-1}
            aria-hidden={sessionId ? 'false' : 'true'}
            disabled={!sessionId || isConnecting}
          />
        </Box>
      </DialogContent>
      <Box sx={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', px: 3, py: 1.5 }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
          {(isExecuting || isConnecting) && (
            <>
              <CircularProgress size={18} />
              <Typography variant="caption" color="text.secondary">
                {isConnecting ? 'Connecting...' : 'Running command...'}
              </Typography>
            </>
          )}
        </Box>
        {!sessionId && !isConnecting && (
          <Button onClick={() => void connect()} color="inherit">
            Reconnect
          </Button>
        )}
      </Box>
    </Dialog>
  );
};

export default TerminalComponent;