# python_src/analytics.spec

# -*- mode: python ; coding: utf-8 -*-
import os
import sys
from PyInstaller.utils.hooks import collect_data_files, collect_dynamic_libs

# --- БЛОК 1: Определяем абсолютные пути ---

# VVVV --- ИЗМЕНЕНИЕ ЗДЕСЬ --- VVVV
# Используем переменную SPEC, предоставляемую PyInstaller, вместо __file__
SPEC_DIR = os.path.dirname(SPEC)
# ^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^

# --- БЛОК 2: Подготовка данных и библиотек ---

# Собираем данные: модель ONNX и все данные из ultralytics.
datas = [
    (os.path.join(SPEC_DIR, 'yolov8n.onnx'), '.'),
    *collect_data_files('ultralytics')
]

# Собираем бинарные файлы (.dll, .so) для onnxruntime и cv2.
binaries = []
binaries += collect_dynamic_libs('onnxruntime')
binaries += collect_dynamic_libs('cv2')

# Явно указываем импорты.
hiddenimports = [
    'numpy',
    'cv2',
    'onnxruntime',
    'scipy',
    'ultralytics',
    'ultralytics.engine.results',
    'PIL',
]

# Исключаем ненужные тяжелые библиотеки
excludes = [
    'torch',
    'torchvision',
    'tensorboard',
    'pandas',
    'matplotlib',
    'seaborn',
    'tkinter'
]

# --- БЛОК 3: Основная конфигурация Analysis ---

a = Analysis(
    [os.path.join(SPEC_DIR, 'analytics.py')], # Явно указываем абсолютный путь к скрипту
    pathex=[SPEC_DIR], # Указываем, где искать импорты
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    runtime_hooks=[],
    excludes=excludes,
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=None,
)

# --- БЛОК 4: Сборка исполняемого файла ---

pyz = PYZ(a.pure, a.zipped_data, cipher=None)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='analytics',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    runtime_tmpdir=None,
    console=True,
)