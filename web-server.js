// web-server.js
const express = require('express');
const path = require('path');

const configManager = require('./src/main/config-manager.js');
const authManager = require('./src/main/auth-manager.js');

const app = express();
const PORT = 8080;

// Отдаём статические файлы интерфейса (index.html, js, css)
app.use(express.static(path.join(__dirname)));

// Простая точка входа; логика авторизации выполняется на клиенте
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

app.get('/login', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'login.html'));
});

app.use(express.json());

app.get('/api/cameras', async (req, res) => {
  try {
    const config = await configManager.loadConfiguration();
    res.json(config.cameras || []);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.get('/api/analytics', async (req, res) => {
  try {
    const today = new Date().toISOString().split('T')[0];
    const events = await configManager.getEventsForDate({ date: today });
    res.json({ events });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.get('/api/settings', async (req, res) => {
  try {
    const settings = await configManager.getAppSettings();
    res.json(settings);
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

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

    res.json({ success: true, username: user.username, role: user.role });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

// TODO: Добавить остальные API для управления камерами, аналитикой, настройками

app.listen(PORT, () => {
  console.log(`Web server running at http://localhost:${PORT}`);
});
