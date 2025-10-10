declare global {
  interface Window {
    __VMS_CAMERAS?: any[];
    setCellCamera?: (cam: any) => void;
  }
}
export {};
declare module '@tauri-apps/api/window' {
  export const appWindow: {
    minimize: () => Promise<void>;
    toggleMaximize: () => Promise<void>;
    close: () => Promise<void>;
  };
}
