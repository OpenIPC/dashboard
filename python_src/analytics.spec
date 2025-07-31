# python_src/analytics.spec

# -*- mode: python ; coding: utf-8 -*-

# Собираем данные: нам нужна модель ONNX.
# Убедитесь, что файл 'yolov8n.onnx' лежит в той же папке, что и этот .spec файл.
datas = [('yolov8n.onnx', '.')]

# PyInstaller не всегда видит все зависимости. Явно указываем ему включить эти модули.
hiddenimports = [
    'numpy',
    'cv2',
    'onnxruntime',
    'ultralytics'
]

a = Analysis(
    ['analytics.py'],
    pathex=[],
    binaries=[],
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=None
)

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