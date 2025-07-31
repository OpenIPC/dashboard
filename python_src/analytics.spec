# python_src/analytics.spec

# -*- mode: python ; coding: utf-8 -*-

from PyInstaller.utils.hooks import collect_data_files

# Собираем данные: модель ONNX и все данные из ultralytics.
# Это самый надежный способ включить все, что нужно.
datas = [
    ('yolov8n.onnx', '.'),
    *collect_data_files('ultralytics')
]

# Исключаем тяжелые и ненужные пакеты, чтобы уменьшить размер
excludes = [
    'torch',
    'torchvision',
    'tensorboard',
    'pandas',
    'matplotlib',
    'seaborn',
    'tkinter'
]

a = Analysis(
    ['analytics.py'],
    pathex=[],
    binaries=[],
    datas=datas,
    hiddenimports=[], # Хуки PyInstaller для ultralytics должны справиться сами
    hookspath=[],
    runtime_hooks=[],
    excludes=excludes,
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=None,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=None)

# Собираем все в один исполняемый файл. Без COLLECT.
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
    runtime_tmpdir=None,
    console=True,
)