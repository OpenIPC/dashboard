document.addEventListener('DOMContentLoaded', () => {
    // Проверяем, загрузился ли preload-скрипт
    if (!window.scpApi) {
        document.body.innerHTML = `<div style="color: #ff6b6b; background-color: #1e1e1e; font-family: sans-serif; padding: 20px; height: 100%; box-sizing: border-box;">
            <h1>Критическая ошибка</h1>
            <p>Не удалось загрузить скрипт для связи с основным процессом (preload-скрипт).</p>
            <p>Возможные причины:</p>
            <ul>
                <li>Файл <strong>fm-preload.js</strong> отсутствует или переименован.</li>
                <li>В файле <strong>main.js</strong> указан неверный путь к preload-скрипту в функции <strong>createFileManagerWindow</strong>.</li>
            </ul>
        </div>`;
        return;
    }
    
    const urlParams = new URLSearchParams(window.location.search);
    const camera = JSON.parse(urlParams.get('camera'));

    let localPath = '';
    let remotePath = '/';
    let selectedLocalFile = null;
    let selectedRemoteFile = null;
    let isBusy = false;

    const localPane = document.getElementById('local-pane');
    const remotePane = document.getElementById('remote-pane');
    const localFileListEl = document.getElementById('local-file-list');
    const remoteFileListEl = document.getElementById('remote-file-list');
    const localPathInput = document.getElementById('local-path');
    const remotePathInput = document.getElementById('remote-path');
    
    const btnUpload = document.getElementById('btn-upload');
    const btnDownload = document.getElementById('btn-download');
    const btnNewFolder = document.getElementById('btn-new-folder');
    const btnDelete = document.getElementById('btn-delete');
    const btnRefresh = document.getElementById('btn-refresh');

    const statusTextEl = document.getElementById('status-text');
    const progressBarContainer = document.getElementById('progress-bar-container');
    const progressBarFill = document.getElementById('progress-bar-fill');
    const pathModule = {
        sep: (navigator.appVersion.indexOf("Win")!=-1) ? '\\' : '/',
        join: (...args) => args.join((navigator.appVersion.indexOf("Win")!=-1) ? '\\' : '/'),
        dirname: (p) => p.substring(0, p.lastIndexOf((navigator.appVersion.indexOf("Win")!=-1) ? '\\' : '/')),
        posix: {
            join: (...args) => args.filter(p => p).join('/'),
            dirname: (p) => p === '/' ? '/' : p.substring(0, p.lastIndexOf('/')) || '/',
        }
    };

    document.querySelector('#camera-name').textContent = camera.name;
    
    function setBusy(busyState, text = "Выполнение операции...") {
        isBusy = busyState;
        if (isBusy) {
            setStatus(text);
        } else {
            setStatus("Готов");
        }
    }

    function setStatus(text, isError = false) {
        statusTextEl.textContent = text;
        statusTextEl.style.color = isError ? '#ff6b6b' : 'var(--text-color)';
        if (!isBusy) hideProgress();
    }

    function showProgress() {
        progressBarContainer.style.display = 'block';
        statusTextEl.style.display = 'none';
        progressBarFill.style.width = '0%';
        progressBarFill.classList.remove('pulse');
    }
    
    // VVV ИЗМЕНЕНИЕ: Новая функция для неопределенного прогресса VVV
    function showIndeterminateProgress() {
        progressBarContainer.style.display = 'block';
        statusTextEl.style.display = 'none';
        progressBarFill.style.width = '100%';
        progressBarFill.classList.add('pulse');
    }
    // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^

    function hideProgress() {
        progressBarContainer.style.display = 'none';
        statusTextEl.style.display = 'inline';
    }

    function updateProgress(progress) {
        showProgress();
        progressBarFill.style.width = `${progress}%`;
    }

    function renderFileList(element, files, isLocal) {
        element.innerHTML = '';
        files.sort((a, b) => {
            if (a.isDirectory !== b.isDirectory) return a.isDirectory ? -1 : 1;
            return a.name.localeCompare(b.name, undefined, { numeric: true, sensitivity: 'base' });
        });
        files.forEach(file => element.appendChild(createFileLI(file, isLocal)));
    }

    function createFileLI(file, isLocal) {
        const li = document.createElement('li');
        li.dataset.name = file.name;
        li.dataset.isdir = file.isDirectory;
        li.innerHTML = `
            <span class="icon">${file.isDirectory ? '📁' : '📄'}</span>
            <span class="file-name">${file.name}</span>
            ${!file.isDirectory ? `<span class="file-size">${formatBytes(file.size)}</span>` : ''}
        `;
        
        li.addEventListener('click', () => {
            if (isBusy) return;
            const pane = isLocal ? localPane : remotePane;
            pane.querySelectorAll('.selected').forEach(el => el.classList.remove('selected'));
            li.classList.add('selected');
            if (isLocal) {
                selectedLocalFile = file;
                selectedRemoteFile = null;
            } else {
                selectedRemoteFile = file;
                selectedLocalFile = null;
            }
            updateButtonStates();
        });

        li.addEventListener('dblclick', async () => {
            if (!file.isDirectory || isBusy) return;
            setBusy(true);
            try {
                if (isLocal) {
                    let newPath = (file.name === '..') ? pathModule.dirname(localPath) : pathModule.join(localPath, file.name);
                    await listLocalFiles(newPath);
                } else {
                    let newPath = (file.name === '..') ? pathModule.posix.dirname(remotePath) : pathModule.posix.join(remotePath, file.name);
                    await listRemoteFiles(newPath);
                }
            } finally {
                setBusy(false);
            }
        });

        return li;
    }

    function formatBytes(bytes, decimals = 2) {
        if (!bytes || bytes === 0) return '';
        const k = 1024;
        const dm = decimals < 0 ? 0 : decimals;
        const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
    }

    function updateButtonStates() {
        btnDownload.disabled = !selectedRemoteFile || selectedRemoteFile.isDirectory;
        btnUpload.disabled = !selectedLocalFile || selectedLocalFile.isDirectory;
        btnDelete.disabled = !selectedLocalFile && !selectedRemoteFile;
    }

    async function ensureConnection() {
        try {
            await window.scpApi.connect(camera);
            return true;
        } catch(e) {
            setStatus(`Ошибка подключения: ${e.message}`, true);
            return false;
        }
    }

    async function listLocalFiles(newPath) {
        setBusy(true, 'Загрузка локальных файлов...');
        try {
            const files = await window.scpApi.listLocal(newPath);
            localPath = newPath;
            localPathInput.value = localPath;
            const isRoot = (pathModule.sep === '\\' && !localPath.includes(pathModule.sep)) || (pathModule.sep === '/' && localPath === '/');
            if (!isRoot) {
                files.unshift({ name: '..', isDirectory: true });
            }
            renderFileList(localFileListEl, files, true);
        } catch (e) {
            setStatus(`Ошибка локально: ${e.message}`, true);
        } finally {
            setBusy(false);
        }
    }
    
    async function listRemoteFiles(newPath) {
        if (!await ensureConnection()) return;
        setBusy(true, `Загрузка ${newPath} с камеры...`);
        try {
            const files = await window.scpApi.list(camera.id, newPath);
            remotePath = newPath;
            remotePathInput.value = remotePath;
            if (remotePath !== '/') {
                 files.unshift({ name: '..', isDirectory: true });
            }
            renderFileList(remoteFileListEl, files, false);
        } catch (e) {
            setStatus(`Ошибка камеры: ${e.message}`, true);
        } finally {
            setBusy(false);
        }
    }

    async function initialize() {
        await listRemoteFiles('/');
        const disks = await window.scpApi.getLocalDiskList();
        await listLocalFiles(disks[0]);
    }
    
    btnRefresh.addEventListener('click', async () => {
        if(isBusy) return;
        await listLocalFiles(localPath);
        await listRemoteFiles(remotePath);
    });

    btnDownload.addEventListener('click', async () => {
        if (!selectedRemoteFile || selectedRemoteFile.isDirectory || !await ensureConnection()) return;
        setBusy(true, `Скачивание ${selectedRemoteFile.name}...`);
        // VVV ИЗМЕНЕНИЕ: Используем неопределенный индикатор VVV
        showIndeterminateProgress();
        // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^

        const remoteFilePath = pathModule.posix.join(remotePath, selectedRemoteFile.name);
        try {
            const result = await window.scpApi.download(camera.id, remoteFilePath);
            if (result.success) {
                setStatus(`Файл ${selectedRemoteFile.name} успешно скачан.`);
                await listLocalFiles(localPath);
            } else if (result.error) {
                setStatus(`Ошибка скачивания: ${result.error}`, true);
            }
        } catch(e) {
            setStatus(`Ошибка скачивания: ${e.message}`, true);
        } finally {
            setBusy(false);
        }
    });

    btnUpload.addEventListener('click', async () => {
        if (!selectedLocalFile || selectedLocalFile.isDirectory || !await ensureConnection()) return;
        setBusy(true, `Загрузка ${selectedLocalFile.name}...`);
        // VVV ИЗМЕНЕНИЕ: Используем неопределенный индикатор VVV
        showIndeterminateProgress();
        // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^
        try {
            const result = await window.scpApi.upload(camera.id, remotePath);
             if (result.success) {
                setStatus(`Файл успешно загружен.`);
                await listRemoteFiles(remotePath);
            } else if(result.error) {
                setStatus(`Ошибка загрузки: ${result.error}`, true);
            }
        } catch (e) {
            setStatus(`Ошибка загрузки: ${e.message}`, true);
        } finally {
            setBusy(false);
        }
    });

    btnDelete.addEventListener('click', async () => {
        const fileToDelete = selectedLocalFile || selectedRemoteFile;
        if (!fileToDelete || !await ensureConnection()) return;
        const isRemote = !!selectedRemoteFile;

        if (!confirm(`Вы уверены, что хотите удалить "${fileToDelete.name}"? Это действие необратимо.`)) return;
        setBusy(true, `Удаление ${fileToDelete.name}...`);

        try {
            let result;
            if (isRemote) {
                const pathToDelete = pathModule.posix.join(remotePath, fileToDelete.name);
                if (fileToDelete.isDirectory) {
                    result = await window.scpApi.deleteDirectory(camera.id, pathToDelete);
                } else {
                    result = await window.scpApi.deleteFile(camera.id, pathToDelete);
                }
            } else {
                setStatus("Локальное удаление не поддерживается.", true);
                setBusy(false);
                return;
            }

            if (result.success) {
                setStatus(`${fileToDelete.name} успешно удален.`);
                if (isRemote) await listRemoteFiles(remotePath);
            } else {
                setStatus(`Ошибка удаления: ${result.error}`, true);
            }
        } catch (e) {
             setStatus(`Ошибка удаления: ${e.message}`, true);
        } finally {
            setBusy(false);
        }
    });
    
    btnNewFolder.addEventListener('click', async () => {
        if (isBusy) return;
        const folderName = prompt('Введите имя новой папки:');
        if (!folderName || !folderName.trim() || !await ensureConnection()) return;
        
        const isRemote = (selectedRemoteFile != null) || (selectedLocalFile == null && document.activeElement.closest('#remote-pane'));
        if (isRemote) {
            setBusy(true, `Создание папки ${folderName}...`);
            const newDirPath = pathModule.posix.join(remotePath, folderName);
            try {
                const result = await window.scpApi.createDirectory(camera.id, newDirPath);
                 if (result.success) {
                    setStatus(`Папка ${folderName} создана.`);
                    await listRemoteFiles(remotePath);
                } else {
                    setStatus(`Ошибка создания папки: ${result.error}`, true);
                }
            } catch (e) {
                 setStatus(`Ошибка: ${e.message}`, true);
            } finally {
                setBusy(false);
            }
        } else {
            setStatus("Создание локальных папок не поддерживается.", true);
        }
    });
    
    remotePathInput.addEventListener('keydown', async e => {
        if(e.key === 'Enter') await listRemoteFiles(remotePathInput.value);
    });
    localPathInput.addEventListener('keydown', async e => {
        if(e.key === 'Enter') await listLocalFiles(localPathInput.value);
    });

    // SCP не возвращает прогресс, поэтому этот обработчик больше не будет вызываться
    window.scpApi.onProgress(updateProgress); 
    window.scpApi.onClose(() => {
        setStatus('Соединение с камерой закрыто.', true);
        remoteFileListEl.innerHTML = '';
    });
    
    initialize();
});