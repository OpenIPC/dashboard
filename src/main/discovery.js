// --- ФАЙЛ: src/main/discovery.js ---

const onvif = require('node-onvif');
const net = require('net');
const os = require('os');

// Используем Map для отслеживания уже найденных IP, чтобы избежать дубликатов
const foundDevices = new Map();

/**
 * Отправляет информацию о найденном устройстве в рендер-процесс.
 */
function sendDeviceFound(mainWindow, device, protocol) {
    if (!device || !device.ip) return;
    // Если уже нашли по любому протоколу, не дублируем
    if (foundDevices.has(device.ip)) {
        return;
    }
    
    foundDevices.set(device.ip, true);
    console.log(`[Scanner ${protocol.toUpperCase()}] Found potential device at ${device.ip}`);
    
    if (mainWindow && !mainWindow.isDestroyed()) {
        mainWindow.webContents.send('device-found', {
            ip: device.ip,
            name: `${protocol.toUpperCase()} Camera`, // Даем общее имя, пользователь уточнит
            protocol: 'onvif' // Упрощаем, считая все ONVIF для добавления
        });
    }
}

/**
 * Быстрый поиск через стандартный ONVIF UDP-зонд.
 */
async function fastDiscoveryTask(mainWindow) {
    console.log('[Scanner ONVIF-Probe] Starting fast discovery via UDP probe...');
    try {
        const devices = await onvif.startProbe();
        devices.forEach(device => {
            try {
                const url = new URL(device.xaddrs[0]);
                const ip = url.hostname;
                if (ip) {
                    sendDeviceFound(mainWindow, { ip }, 'ONVIF');
                }
            } catch (e) {
                console.error('[Scanner ONVIF-Probe] Could not parse device address:', device.xaddrs[0], e.message);
            }
        });
        console.log('[Scanner ONVIF-Probe] Fast discovery probe finished.');
    } catch (error) {
        console.error('[Scanner ONVIF-Probe] Error during probe:', error.message);
    }
}


/**
 * Проверяет, открыт ли порт на указанном IP.
 */
function checkPort(ip, port, timeout = 1000) {
    return new Promise((resolve) => {
        const socket = new net.Socket();
        socket.setTimeout(timeout);
        const onError = () => {
            socket.destroy();
            resolve(false);
        };
        socket.on('connect', () => {
            socket.destroy();
            resolve(true);
        });
        socket.on('timeout', onError);
        socket.on('error', onError);
        socket.connect(port, ip);
    });
}

/**
 * Глубокое сканирование всех локальных подсетей по стандартным портам.
 */
async function ipScanDiscoveryTask(mainWindow) {
    console.log('[Scanner IP-Scan] Starting robust TCP port scan...');
    const COMMON_ONVIF_PORTS = [80, 8899, 8080, 2020];
    const interfaces = os.networkInterfaces();
    const subnets = new Set();

    // Собираем все локальные подсети (например, 192.168.1.)
    for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
            if (iface.family === 'IPv4' && !iface.internal) {
                subnets.add(iface.address.substring(0, iface.address.lastIndexOf('.') + 1));
            }
        }
    }

    const scanPromises = [];
    for (const subnet of subnets) {
        console.log(`[Scanner IP-Scan] Scanning subnet: ${subnet}0/24...`);
        for (let i = 1; i < 255; i++) {
            const ip = subnet + i;
            if (foundDevices.has(ip)) continue;

            const promise = (async () => {
                for (const port of COMMON_ONVIF_PORTS) {
                    if (await checkPort(ip, port, 500)) { 
                        try {
                            const device = new onvif.OnvifDevice({
                                xaddr: `http://${ip}:${port}/onvif/device_service`,
                            });
                            
                            // Пытаемся инициализировать с таймаутом, чтобы не зависать надолго
                            const initPromise = device.init();
                            const timeoutPromise = new Promise((_, reject) => setTimeout(() => reject(new Error('timeout')), 2000));
                            await Promise.race([initPromise, timeoutPromise]);

                            sendDeviceFound(mainWindow, { ip }, 'ONVIF');
                            break; // Нашли на одном порту, переходим к следующему IP
                        } catch (error) {
                            // Игнорируем ошибки, это нормально для устройств, не являющихся камерами
                        }
                    }
                }
            })();
            scanPromises.push(promise);
        }
    }
    
    await Promise.allSettled(scanPromises);
    console.log('[Scanner IP-Scan] TCP port scan finished.');
    return { success: true };
}


/**
 * Точка входа: запускает оба метода поиска одновременно.
 */
async function discoverDevices(mainWindow) {
    console.log('[Scanner] Starting comprehensive discovery...');
    foundDevices.clear();
    
    // Запускаем оба метода параллельно для максимальной скорости
    fastDiscoveryTask(mainWindow);
    ipScanDiscoveryTask(mainWindow);

    return { success: true, message: 'Comprehensive discovery started' };
}

module.exports = {
    discoverDevices,
};