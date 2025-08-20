// --- START OF FILE src/main/hls-server.js ---
// Файл: src/main/hls-server.js
const express = require('express');
const path = require('path');
const cors = require('cors');

let server = null;
let serverPort = null;

async function startHlsServer(servingPath) {
    if (server) {
        console.log('[HLS Server] Server already running.');
        return { port: serverPort };
    }

    // Динамический import() ESM-модуля get-port
    const { default: getPort } = await import('get-port');

    const app = express();
    app.use(cors());
    app.use('/hls', express.static(servingPath));

    // Исправленный вызов getPort. 
    // Библиотека сама найдет следующий свободный порт, если 9100 занят.
    const port = await getPort({ port: 9100 });
    serverPort = port;

    return new Promise((resolve, reject) => {
        server = app.listen(serverPort, () => {
            console.log(`[HLS Server] Started on port ${serverPort}, serving from ${servingPath}`);
            // Обновляем экспортируемое значение ТОЛЬКО после успешного запуска
            module.exports.serverPort = { port: serverPort };
            resolve({ port: serverPort });
        }).on('error', (err) => {
            console.error('[HLS Server] Failed to start:', err);
            reject(err);
        });
    });
}

function stopHlsServer() {
    if (server) {
        server.close(() => {
            console.log('[HLS Server] Stopped.');
            server = null;
            serverPort = null;
        });
    }
}

// Начальный экспорт. serverPort будет null, пока сервер не запустится.
module.exports = { startHlsServer, stopHlsServer, serverPort: null };
// --- END OF FILE src/main/hls-server.js ---