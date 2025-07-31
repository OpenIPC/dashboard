# python_src/analytics.spec

# -*- mode: python ; coding: utf-8 -*-
import os
import sys

# --- БЛОК 1: Ручной поиск и сбор данных ---

# Функция для поиска пути к пакету
def get_site_packages_path(package_name):
    import importlib.util
    try:
        spec = importlib.util.find_spec(package_name)
        if spec and spec.origin:
            # Путь к __init__.py -> путь к папке пакета
            return os.path.dirname(spec.origin)
    except:
        pass
    # Резервный метод
    for path in sys.path:
        if package_name in path and 'site-packages' in path:
            return path
    return None

# Собираем данные: модель ONNX.
datas = [('yolov8n.onnx', '.')]

# Собираем бинарные файлы и данные вручную
binaries = []
package_data_to_include = [
    'onnxruntime',
    'cv2',
    'ultralytics',
    'numpy',
    'PIL' # Pillow
]

for package in package_data_to_include:
    path = get_site_packages_path(package)
    if path:
        print(f"INFO: Including data from '{package}' at path: {path}")
        datas.append((path, package))
    else:
        print(f"WARNING: Could not find path for package '{package}'")

# Явно указываем импорты, которые могут быть пропущены
hiddenimports = [
    'ultralytics.engine.results',
    'scipy'
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
    cipher=None,
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
    runtime_tmpdir=None,
    console=True,
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