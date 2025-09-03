// web-server.js
const express = require('express');
const path = require('path');
const app = express();
const PORT = 8080;

// Отдаём статические файлы интерфейса (index.html, js, css)
app.use(express.static(path.join(__dirname)));
// Middleware: проверка авторизации для главной страницы
app.get('/', (req, res, next) => {
  // Примитивная проверка: если нет username в localStorage, редирект на login
  // На сервере нет доступа к localStorage, поэтому делаем проверку на клиенте
  res.sendFile(path.join(__dirname, 'index.html'));
});

// Роут для login.html
app.get('/login', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'login.html'));
});

// Пример API: статус системы

// Пример: список камер (заглушка)
const configManager = require('./src/main/config-manager.js');
const bodyParser = require('body-parser');
const authManager = require('./src/main/auth-manager.js');

app.use(bodyParser.json());

app.get('/api/cameras', async (req, res) => {
  try {
    const config = await configManager.loadConfiguration();
    res.json(config.cameras || []);
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});

// Пример: аналитика (заглушка)
app.get('/api/analytics', async (req, res) => {
  try {
    // Получаем все события за сегодня
    const today = new Date().toISOString().split('T')[0];
    const events = await configManager.getEventsForDate({ date: today });
    res.json({ events });
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});

// Пример: настройки (заглушка)
app.get('/api/settings', async (req, res) => {
// Login API
app.post('/api/login', async (req, res) => {
  try {
    const { username, password } = req.body;
    if (!username || !password) {
      return res.status(400).json({ error: 'Username and password required' });
    }
    const users = await authManager.getUsers();
    const user = users.users.find(u => u.username === username);
    if (!user) {
      return res.status(401).json({ error: 'Invalid username or password' });
    }
    const valid = authManager.verifyPassword(password, user.salt, user.hashedPassword);
    if (!valid) {
      return res.status(401).json({ error: 'Invalid username or password' });
    }
    // Можно добавить генерацию JWT или сессионного токена
    res.json({ success: true, username: user.username, role: user.role });
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});
  try {
    const settings = await configManager.getAppSettings();
    res.json(settings);
  } catch (e) {
    res.status(500).json({ error: e.message });
  }
});

// TODO: Добавить остальные API для управления камерами, аналитикой, настройками

app.listen(PORT, () => {
  console.log(`Web server running at http://localhost:${PORT}`);
});
