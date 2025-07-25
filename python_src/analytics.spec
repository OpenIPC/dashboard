# -*- mode: python ; coding: utf-8 -*-

# --- НАЧАЛО БЛОКА КОНФИГУРАЦИИ ---

# 1. Ваш путь к папке site-packages, определенный командой 'pip show ultralytics'.
#    Двойные обратные слэши обязательны.
site_packages_path = 'C:\\Users\\vavol\\AppData\\Roaming\\Python\\Python313\\site-packages'

# 2. Список "скрытых" импортов, которые PyInstaller может пропустить.
#    Это помогает включить все необходимые зависимости.
hidden_imports_list = [
    'ultralytics',
    'ultralytics.engine.results',
    'ultralytics.utils',
    'torch',
    'torchvision',
    'cv2',
    'numpy',
    'yaml',
    'tqdm',
    'pandas',
    'seaborn',
    'matplotlib',
    'scipy',
    'psutil'
]

# 3. Список файлов с данными (например, веса моделей), которые нужно включить в .exe
#    Если ultralytics сама скачивает веса, этот список можно оставить пустым.
datas_list = []

# --- КОНЕЦ БЛОКА КОНФИГУРАЦИИ ---


a = Analysis(
    ['analytics.py'],
    pathex=[site_packages_path],  # <--- Используем ваш путь
    binaries=[],
    datas=datas_list,             # <--- Используем ваш список данных
    hiddenimports=hidden_imports_list, # <--- Используем ваш список импортов
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

# Главная секция для сборки в ОДИН ФАЙЛ
exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='analytics',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=True, # Оставляем True для отладки
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)