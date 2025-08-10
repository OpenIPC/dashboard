// --- ФАЙЛ: src/main/app-lifecycle.js ---

const { app } = require('electron');
const { initializeUsers } = require('./config-manager');
const { stopAllProcesses, getAllProcessesOfType } = require('./process-manager');
const { onvifConnectionManager, netipConnectionManager } = require('./camera-api');

let isShuttingDown = false;

function initializeApp() {
    if (process.platform === 'win32') {
        app.setAppUserModelId("com.vavol.openipcdashboard");
    }
    initializeUsers();
}

async function onAppWillQuit(event) {
    if (isShuttingDown) {
        return;
    }
    
    onvifConnectionManager.closeAll();
    netipConnectionManager.closeAll();

    const recordingProcs = getAllProcessesOfType('recording');

    if (recordingProcs.length > 0) {
        event.preventDefault();
        isShuttingDown = true;
        console.log(`[Shutdown] Gracefully stopping ${recordingProcs.length} recordings...`);

        const promises = recordingProcs.map(({ key, process }) => {
            return new Promise(resolve => {
                const timeout = setTimeout(() => {
                    if (!process.killed) process.kill('SIGKILL');
                    resolve();
                }, 4000);

                process.on('close', () => {
                    clearTimeout(timeout);
                    resolve();
                });

                try {
                    if (process.stdin.writable) {
                        process.stdin.write('q\n');
                    } else {
                        process.kill();
                    }
                } catch (e) {
                    if (!process.killed) process.kill('SIGKILL');
                    resolve();
                }
            });
        });

        await Promise.all(promises);
        console.log('[Shutdown] All recordings stopped. Quitting now.');
        app.quit();
    } else {
        console.log('[Shutdown] No active recordings. Stopping all other processes.');
        stopAllProcesses();
    }
}

module.exports = {
    initializeApp,
    onAppWillQuit,
};