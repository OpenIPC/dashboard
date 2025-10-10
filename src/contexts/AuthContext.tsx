import React, {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
} from 'react';
import type { ReactNode } from 'react';
import type { AuthUser, UserPermissions } from '../types';
import {
  autoLogin as autoLoginRequest,
  login as loginRequest,
  logout as logoutRequest,
} from '../services/auth';

interface AuthContextValue {
  user: AuthUser | null;
  initializing: boolean;
  authenticating: boolean;
  error: string | null;
  login: (username: string, password: string, rememberMe: boolean) => Promise<void>;
  logout: () => Promise<void>;
  clearError: () => void;
  hasPermission: (permission: keyof UserPermissions) => boolean;
}

const AuthContext = createContext<AuthContextValue | undefined>(undefined);

const extractErrorMessage = (error: unknown): string => {
  if (!error) {
    return 'Unknown error';
  }
  if (typeof error === 'string') {
    return error;
  }
  if (error instanceof Error) {
    return error.message;
  }
  if (typeof error === 'object' && 'message' in (error as Record<string, unknown>)) {
    const message = (error as Record<string, unknown>).message;
    if (typeof message === 'string') {
      return message;
    }
  }
  return 'Unknown error';
};

export const AuthProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [user, setUser] = useState<AuthUser | null>(null);
  const [initializing, setInitializing] = useState(true);
  const [authenticating, setAuthenticating] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let isMounted = true;

    const attemptAutoLogin = async () => {
      try {
        const response = await autoLoginRequest();
        if (!isMounted) return;

        if (response.success && response.user) {
          setUser(response.user);
          setError(null);
        } else {
          setUser(null);
          if (response.error) {
            console.info('[Auth] Auto-login skipped:', response.error);
          }
        }
      } catch (err) {
        if (isMounted) {
          console.error('[Auth] Auto-login failed:', err);
        }
      } finally {
        if (isMounted) {
          setInitializing(false);
        }
      }
    };

    attemptAutoLogin();

    return () => {
      isMounted = false;
    };
  }, []);

  const login = useCallback(
    async (username: string, password: string, rememberMe: boolean) => {
      setAuthenticating(true);
      setError(null);
      try {
        const response = await loginRequest(username, password, rememberMe);
        if (response.success && response.user) {
          setUser(response.user);
          setError(null);
        } else {
          setError(response.error ?? 'Invalid username or password');
          setUser(null);
        }
      } catch (err) {
        const message = extractErrorMessage(err);
        setError(message);
        setUser(null);
      } finally {
        setAuthenticating(false);
      }
    },
    [],
  );

  const logout = useCallback(async () => {
    try {
      await logoutRequest();
    } catch (err) {
      console.error('[Auth] Logout error:', err);
    } finally {
      setUser(null);
      setError(null);
    }
  }, []);

  const clearError = useCallback(() => {
    setError(null);
  }, []);

  const hasPermission = useCallback(
    (permission: keyof UserPermissions) => {
      if (!user) {
        return false;
      }
      if (user.role === 'admin') {
        return true;
      }
      return Boolean(user.permissions && user.permissions[permission]);
    },
    [user],
  );

  const contextValue = useMemo<AuthContextValue>(
    () => ({
      user,
      initializing,
      authenticating,
      error,
      login,
      logout,
      clearError,
      hasPermission,
    }),
    [user, initializing, authenticating, error, login, logout, clearError, hasPermission],
  );

  return <AuthContext.Provider value={contextValue}>{children}</AuthContext.Provider>;
};

export const useAuth = (): AuthContextValue => {
  const context = useContext(AuthContext);
  if (!context) {
    throw new Error('useAuth must be used within an AuthProvider');
  }
  return context;
};
