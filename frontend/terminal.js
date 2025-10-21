function resolveFitAddonClass() {
    if (window.FitAddon) {
        if (typeof window.FitAddon === 'function') return window.FitAddon;
        if (typeof window.FitAddon.FitAddon === 'function') return window.FitAddon.FitAddon;
    }
    return null;
}

function initializeTerminal() {
    const TerminalCtor = window.Terminal;
    const FitAddonCtor = resolveFitAddonClass();
    if (typeof TerminalCtor !== 'function' || typeof FitAddonCtor !== 'function') {
        console.error('[SSH Terminal] Xterm libraries are not available.', { TerminalCtor, FitAddonCtor });
        return;
    }

    const urlParams = new URLSearchParams(window.location.search);
    const camera = JSON.parse(urlParams.get('camera'));

    const term = new TerminalCtor({
        cursorBlink: true,
        fontFamily: 'Consolas, "Courier New", monospace',
        fontSize: 14,
        theme: {
            background: '#1e1e1e',
            foreground: '#d4d4d4',
            cursor: '#d4d4d4',
            selectionBackground: '#525252',
        }
    });

    const fitAddon = new FitAddonCtor();
    term.loadAddon(fitAddon);

    term.open(document.getElementById('terminal'));
    fitAddon.fit();

    window.addEventListener('resize', () => fitAddon.fit());

    term.writeln('Подключение к ' + camera.ip + '...');
    term.writeln('');
    term.writeln('\x1b[33mПодсказка: Используйте Ctrl+Shift+C для копирования и Ctrl+Shift+V для вставки.\x1b[0m');

    window.terminalApi.onData((data) => {
        term.write(data);
    });

    term.onData(data => {
        window.terminalApi.sendInput(camera.id, data);
    });

    window.terminalApi.onStatus((status) => {
        if (status.connected) {
            term.writeln('\r\n\x1b[32mСоединение установлено!\x1b[0m');
        } else {
            term.writeln(`\r\n\x1b[31m${status.message}\x1b[0m`);
        }
    });

    term.attachCustomKeyEventHandler(async (e) => {
        if (e.ctrlKey && e.shiftKey && (e.key === 'V' || e.key === 'Insert')) {
            e.preventDefault();
            const text = await window.terminalApi.readClipboard();
            term.write(text);
            return false;
        }
        if (e.ctrlKey && e.shiftKey && e.key === 'C') {
            e.preventDefault();
            const selection = term.getSelection();
            if (selection) {
                await window.terminalApi.writeClipboard(selection);
            }
            return false;
        }
        return true;
    });

    window.terminalApi.notifyReady(camera.id);
}

if (window.Terminal && resolveFitAddonClass()) {
    initializeTerminal();
} else {
    window.addEventListener('xterm-ready', () => {
        initializeTerminal();
    }, { once: true });
}