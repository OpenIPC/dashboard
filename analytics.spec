# python_src/analytics.spec

# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_data_files, collect_dynamic_libs

# --- БЛОК 1: Подготовка данных и библиотек ---

# Собираем данные: модель ONNX.
datas = [('yolov8n.onnx', '.')]

# Собираем бинарные файлы (.dll, .so) для onnxruntime и cv2.
binaries = []
binaries += collect_dynamic_libs('onnxruntime')
binaries += collect_dynamic_libs('cv2')

# Собираем дополнительные файлы данных, которые могут понадобиться.
datas += collect_data_files('onnxruntime')
datas += collect_data_files('cv2')
datas += collect_data_files('ultralytics')

# Явно указываем необходимые импорты.
hiddenimports = [
    'numpy',
    'cv2',
    'onnxruntime',
    'scipy',
    'ultralytics',
    'ultralytics.engine.results',
    'PIL',
]

# ИСКЛЮЧАЕМ НЕНУЖНЫЕ ТЯЖЕЛЫЕ БИБЛИОТЕКИ, которые тянет ultralytics
excludes = [
    'torch',
    'torchvision',
    'tensorboard',
    'pandas',
    'matplotlib',
    'seaborn',
    'tkinter'
]

# --- БЛОК 2: Основная конфигурация Analysis ---

a = Analysis(
    ['analytics.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    runtime_hooks=[],
    excludes=excludes,
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=None
)


# --- БЛОК 3: Сборка исполняемого файла ---

pyz = PYZ(a.pure, a.zipped_data, cipher=None)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='analytics',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_console=True,
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None
)

coll = COLLECT(
    exe,
    a.binaries,
    a.zipfiles,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name='analytics'
)