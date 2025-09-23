const urlParams = new URLSearchParams(window.location.search);
const camera = JSON.parse(urlParams.get('camera'));
// document.title больше не нужен, так как у нас свой заголовок
// document.title = `SSH: ${camera.name} (${camera.ip})`;

const term = new Terminal({
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

const fitAddon = new FitAddon.FitAddon();
term.loadAddon(fitAddon);

// VVV ИЗМЕНЕНИЕ: Открываем терминал в правильном контейнере VVV
term.open(document.getElementById('terminal'));
// ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^
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