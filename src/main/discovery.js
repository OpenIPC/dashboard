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
    // Common ports to probe for ONVIF/HTTP/RTSP/HTTPS interfaces on cameras
    const COMMON_ONVIF_PORTS = [80, 443, 554, 8554, 8899, 8000, 8001, 8080, 8443, 2020];
    const interfaces = os.networkInterfaces();
    const subnets = new Set();

    // Diagnostic: dump interfaces to help troubleshooting when discovery returns no devices
    try {
        console.log('[Scanner IP-Scan] Network interfaces:');
        for (const name of Object.keys(interfaces)) {
            for (const iface of interfaces[name]) {
                if (iface.family === 'IPv4') {
                    console.log(`  - ${name}: ${iface.address} / ${iface.netmask} (${iface.cidr})${iface.internal ? ' [internal]' : ''}`);
                }
            }
        }
    } catch (e) { /* ignore in best-effort logging */ }

    // Helpers: convert dotted IPv4 <-> int
    function ipv4ToInt(ip) {
        return ip.split('.').reduce((acc, octet) => (acc << 8) + parseInt(octet, 10), 0) >>> 0;
    }
    function intToIpv4(int) {
        return [(int >>> 24) & 0xFF, (int >>> 16) & 0xFF, (int >>> 8) & 0xFF, int & 0xFF].join('.');
    }
    function netmaskToInt(mask) {
        return mask.split('.').reduce((acc, octet) => (acc << 8) + parseInt(octet, 10), 0) >>> 0;
    }

    // Собираем все локальные подсети (например, 192.168.1.) на основе адреса + маски
    for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
            if (iface.family === 'IPv4' && !iface.internal) {
                try {
                    let mask = iface.netmask || (iface.cidr ? iface.cidr.split('/')[1] : null);
                    let prefix24;
                    if (typeof mask === 'string' && mask.indexOf('.') !== -1) {
                        // mask like '255.255.255.0'
                        const addrInt = ipv4ToInt(iface.address);
                        const maskInt = netmaskToInt(mask);
                        const networkInt = addrInt & maskInt;
                        // Derive the /24 containing this interface (safe default)
                        const network24Int = networkInt & netmaskToInt('255.255.255.0');
                        prefix24 = intToIpv4(network24Int).substring(0, intToIpv4(network24Int).lastIndexOf('.') + 1);
                    } else if (mask !== null) {
                        // mask is CIDR length like '24'
                        const cidr = parseInt(mask, 10);
                        const addrInt = ipv4ToInt(iface.address);
                        const maskInt = cidr >= 32 ? 0xFFFFFFFF : cidr <= 0 ? 0 : (~((1 << (32 - cidr)) - 1)) >>> 0;
                        const networkInt = addrInt & maskInt;
                        const network24Int = networkInt & netmaskToInt('255.255.255.0');
                        prefix24 = intToIpv4(network24Int).substring(0, intToIpv4(network24Int).lastIndexOf('.') + 1);
                    } else {
                        // Fallback: use first three octets of address
                        prefix24 = iface.address.substring(0, iface.address.lastIndexOf('.') + 1);
                    }

                    if (prefix24) {
                        subnets.add(prefix24);
                    }
                } catch (e) {
                    console.warn('[Scanner IP-Scan] Failed to compute subnet for iface', iface.address, iface.netmask, e && e.message);
                    subnets.add(iface.address.substring(0, iface.address.lastIndexOf('.') + 1));
                }
            }
        }
    }

    // If we didn't discover any subnets from interfaces (rare), add a small set of common private subnets as a fallback
    if (subnets.size === 0) {
        const fallback = ['192.168.0.', '192.168.1.', '10.0.0.', '172.16.0.'];
        console.log('[Scanner IP-Scan] No non-internal interfaces found; falling back to common private subnets:', fallback);
        fallback.forEach(s => subnets.add(s));
    }

    const scanPromises = [];
    for (const subnet of subnets) {
        console.log(`[Scanner IP-Scan] Scanning subnet: ${subnet}0/24...`);
        for (let i = 1; i < 255; i++) {
            const ip = subnet + i;
            if (foundDevices.has(ip)) continue;

            const promise = (async () => {
                for (const port of COMMON_ONVIF_PORTS) {
                    // Use slightly longer timeout for slower networks/devices
                    const portOpen = await checkPort(ip, port, 800);
                    if (portOpen) {
                        try {
                            // Prefer ONVIF init on HTTP/ONVIF ports
                            const tryOnvif = [80, 8080, 8899, 8000, 8001, 2020, 443, 8443];
                            if (tryOnvif.includes(port)) {
                                const device = new onvif.OnvifDevice({ xaddr: `http://${ip}:${port}/onvif/device_service` });
                                const initPromise = device.init();
                                const timeoutPromise = new Promise((_, reject) => setTimeout(() => reject(new Error('timeout')), 3000));
                                await Promise.race([initPromise, timeoutPromise]);

                                sendDeviceFound(mainWindow, { ip }, 'ONVIF');
                                break; // Found and reported
                            }

                            // If it's an RTSP port (554, 8554) — ONVIF may not be available, but RTSP stream likely is.
                            if ([554, 8554].includes(port)) {
                                console.log(`[Scanner IP-Scan] RTSP port open at ${ip}:${port} — reporting as RTSP candidate`);
                                sendDeviceFound(mainWindow, { ip }, 'RTSP');
                                break;
                            }
                        } catch (error) {
                            // If ONVIF init fails on an ONVIF-capable port, ignore and continue scanning other ports.
                            // For RTSP ports, we already report above.
                            // Log at debug level for diagnostics.
                            console.debug('[Scanner IP-Scan] Probe failed for', ip, port, error && error.message);
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