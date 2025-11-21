import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

// https://vite.dev/config/
export default defineConfig({
  base: './',
  plugins: [react()],
  build: {
    outDir: 'dist',
    assetsDir: 'assets',
    sourcemap: true,
    // Use Terser instead of the default esbuild minifier to avoid identifier
    // redeclaration collisions that surfaced in the production bundle.
    minify: 'terser',
    terserOptions: {
      module: true,
      mangle: false,
      compress: {
        passes: 2
      },
      format: {
        comments: false
      }
    },
    rollupOptions: {
      output: {
        manualChunks: undefined
      }
    }
  },
  server: {
    strictPort: true,
    port: 5173
  },
  // Обеспечиваем совместимость с Tauri
  clearScreen: false,
  envPrefix: ['VITE_', 'TAURI_PLATFORM', 'TAURI_ARCH', 'TAURI_FAMILY', 'TAURI_PLATFORM_VERSION', 'TAURI_PLATFORM_TYPE']
})
