import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.tsx'

// StrictMode отключен для избежания двойного монтирования компонентов
// и дублирования WebRTC соединений в dev режиме
createRoot(document.getElementById('root')!).render(
  <App />
)
