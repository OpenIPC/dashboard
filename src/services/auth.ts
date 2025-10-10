import { invoke } from '@tauri-apps/api/core';
import type { AuthUser, UserPermissions, UserRole } from '../types';

export interface LoginResponse {
  success: boolean;
  user?: AuthUser;
  error?: string;
}

export interface AutoLoginResponse {
  success: boolean;
  user?: AuthUser;
  error?: string;
}

export interface UsersResponse {
  success: boolean;
  users?: AuthUser[];
  error?: string;
}

export interface OperationResponse {
  success: boolean;
  error?: string;
}

// Проверяем, доступен ли Tauri (поддержка Tauri v1/v2)
let cachedTauriAvailability: boolean | null = null;

const isTauriAvailable = (): boolean => {
  if (cachedTauriAvailability !== null) {
    return cachedTauriAvailability;
  }

  if (typeof window === 'undefined') {
    cachedTauriAvailability = false;
    return cachedTauriAvailability;
  }

  const globalWindow = window as typeof window & {
    __TAURI__?: unknown;
    __TAURI_INTERNALS__?: unknown;
    __TAURI_METADATA__?: unknown;
  };

  const hasTauriGlobal =
    typeof globalWindow.__TAURI__ !== 'undefined' ||
    typeof globalWindow.__TAURI_INTERNALS__ !== 'undefined' ||
    typeof globalWindow.__TAURI_METADATA__ !== 'undefined';

  const hasTauriUserAgent =
    typeof navigator !== 'undefined' && /tauri/i.test(navigator.userAgent || '');

  cachedTauriAvailability = hasTauriGlobal || hasTauriUserAgent;
  return cachedTauriAvailability;
};

// Создаем mock user для fallback режима
const createMockUser = (): AuthUser => ({
  username: 'admin',
  role: 'admin' as UserRole,
  permissions: {
    viewCameras: true,
    manageCameras: true,
    viewRecordings: true,
    manageRecordings: true,
    viewAnalytics: true,
    manageAnalytics: true,
    manageUsers: true,
    manageSettings: true,
  } as UserPermissions,
});

export const login = async (username: string, password: string, rememberMe: boolean): Promise<LoginResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, using fallback mode');
    // Простая проверка для demo режима
    if (username === 'admin' && password === 'admin') {
      return {
        success: true,
        user: createMockUser(),
      };
    }
    return {
      success: false,
      error: 'Invalid credentials (demo mode)',
    };
  }

  try {
    return await invoke<LoginResponse>('login', {
      credentials: {
        username,
        password,
        rememberMe,
      },
    });
  } catch (error) {
    console.error('[Auth] Login invoke failed:', error);
    return {
      success: false,
      error: `Authentication service unavailable: ${error}`,
    };
  }
};

export const autoLogin = async (): Promise<AutoLoginResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, skipping auto-login');
    return {
      success: false,
      error: 'Tauri not available',
    };
  }

  try {
    return await invoke<AutoLoginResponse>('auto_login');
  } catch (error) {
    console.error('[Auth] Auto-login invoke failed:', error);
    return {
      success: false,
      error: `Auto-login service unavailable: ${error}`,
    };
  }
};

export const logout = async (): Promise<OperationResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, using fallback logout');
    return {
      success: true,
    };
  }

  try {
    return await invoke<OperationResponse>('logout');
  } catch (error) {
    console.error('[Auth] Logout invoke failed:', error);
    return {
      success: true, // Считаем logout успешным даже при ошибке
    };
  }
};

export const getUsers = async (): Promise<UsersResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, returning mock users');
    return {
      success: true,
      users: [createMockUser()],
    };
  }

  try {
    return await invoke<UsersResponse>('get_users');
  } catch (error) {
    console.error('[Auth] Get users invoke failed:', error);
    return {
      success: false,
      error: `User service unavailable: ${error}`,
    };
  }
};

export const addUser = async (
  username: string,
  password: string,
  role: UserRole,
  permissions?: UserPermissions,
): Promise<OperationResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, simulating add user');
    return {
      success: true,
    };
  }

  try {
    return await invoke<OperationResponse>('add_user', {
      user: {
        username,
        password,
        role,
        permissions,
      },
    });
  } catch (error) {
    console.error('[Auth] Add user invoke failed:', error);
    return {
      success: false,
      error: `User service unavailable: ${error}`,
    };
  }
};

export const updateUserPassword = async (
  username: string,
  password: string,
): Promise<OperationResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, simulating password update');
    return {
      success: true,
    };
  }

  try {
    return await invoke<OperationResponse>('update_user_password', {
      payload: {
        username,
        password,
      },
    });
  } catch (error) {
    console.error('[Auth] Update password invoke failed:', error);
    return {
      success: false,
      error: `User service unavailable: ${error}`,
    };
  }
};

export const updateUserRole = async (
  username: string,
  role: UserRole,
): Promise<OperationResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, simulating role update');
    return {
      success: true,
    };
  }

  try {
    return await invoke<OperationResponse>('update_user_role', {
      payload: {
        username,
        role,
      },
    });
  } catch (error) {
    console.error('[Auth] Update role invoke failed:', error);
    return {
      success: false,
      error: `User service unavailable: ${error}`,
    };
  }
};

export const updateUserPermissions = async (
  username: string,
  permissions: UserPermissions,
): Promise<OperationResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, simulating permissions update');
    return {
      success: true,
    };
  }

  try {
    return await invoke<OperationResponse>('update_user_permissions', {
      payload: {
        username,
        permissions,
      },
    });
  } catch (error) {
    console.error('[Auth] Update permissions invoke failed:', error);
    return {
      success: false,
      error: `User service unavailable: ${error}`,
    };
  }
};

export const deleteUser = async (username: string): Promise<OperationResponse> => {
  if (!isTauriAvailable()) {
    console.warn('[Auth] Tauri not available, simulating user deletion');
    return {
      success: true,
    };
  }

  try {
    return await invoke<OperationResponse>('delete_user', {
      payload: {
        username,
      },
    });
  } catch (error) {
    console.error('[Auth] Delete user invoke failed:', error);
    return {
      success: false,
      error: `User service unavailable: ${error}`,
    };
  }
};
