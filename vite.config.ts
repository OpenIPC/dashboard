import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))

// https://vite.dev/config/
export default defineConfig({
  base: './',
  plugins: [react()],
  resolve: {
    alias: {
      '@emotion/react': path.resolve(__dirname, 'node_modules/@emotion/react'),
      '@emotion/styled': path.resolve(__dirname, 'node_modules/@emotion/styled')
    },
    dedupe: ['react', 'react-dom', '@emotion/react', '@emotion/styled']
  },
  optimizeDeps: {
    entries: ['src/main.tsx'],
    exclude: [],
    esbuildOptions: {
      absWorkingDir: __dirname
    }
  },
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
    port: 5173,
    watch: {
      ignored: [
        '**/playground/**',
        '**/release-anpr/**',
        '**/src-tauri/target/**',
        '**/artifacts/**'
      ]
    }
  },
  // Обеспечиваем совместимость с Tauri
  clearScreen: false,
  envPrefix: ['VITE_', 'TAURI_PLATFORM', 'TAURI_ARCH', 'TAURI_FAMILY', 'TAURI_PLATFORM_VERSION', 'TAURI_PLATFORM_TYPE']
})
