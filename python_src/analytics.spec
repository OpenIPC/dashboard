# -*- mode: python ; coding: utf-8 -*-

# --- НАЧАЛО БЛОКА КОНФИГУРАЦИИ ---

# Ваш путь к папке site-packages
site_packages_path = 'C:\\Users\\vavol\\AppData\\Roaming\\Python\\Python313\\site-packages'

# Список "скрытых" импортов
hidden_imports_list = [
    'ultralytics', 'ultralytics.engine.results', 'ultralytics.utils',
    'torch', 'torchvision', 'cv2', 'numpy', 'yaml', 'tqdm',
    'pandas', 'seaborn', 'matplotlib', 'scipy', 'psutil'
]

# Список файлов с данными (если нужны)
datas_list = []

# --- КОНЕЦ БЛОКА КОНФИГУРАЦИИ ---


a = Analysis(
    ['analytics.py'],
    pathex=[site_packages_path],
    binaries=[],
    datas=datas_list,
    hiddenimports=hidden_imports_list,
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