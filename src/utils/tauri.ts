export const isTauriAvailable = (): boolean => {
  if (typeof window === 'undefined') {
    return false;
  }

  const globalWindow = window as typeof window & {
    __TAURI__?: unknown;
    __TAURI_INTERNALS__?: unknown;
    __TAURI_METADATA__?: unknown;
  };

  if (
    typeof globalWindow.__TAURI__ !== 'undefined' ||
    typeof globalWindow.__TAURI_INTERNALS__ !== 'undefined' ||
    typeof globalWindow.__TAURI_METADATA__ !== 'undefined'
  ) {
    return true;
  }

  if (typeof navigator !== 'undefined' && /tauri/i.test(navigator.userAgent || '')) {
    return true;
  }

  return false;
};
