import { createContext } from 'react';
import type { AuthUser, UserPermissions } from '../types';

export interface AuthContextValue {
  user: AuthUser | null;
  initializing: boolean;
  authenticating: boolean;
  error: string | null;
  login: (username: string, password: string, rememberMe: boolean) => Promise<void>;
  logout: () => Promise<void>;
  clearError: () => void;
  hasPermission: (permission: keyof UserPermissions) => boolean;
}

export const AuthContext = createContext<AuthContextValue | undefined>(undefined);
