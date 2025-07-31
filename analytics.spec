# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['analytics.py'],
    pathex=[],
    binaries=[],
    # VVVV --- ИЗМЕНЕНИЕ ЗДЕСЬ --- VVVV
    # Добавляем всю папку cv2 из виртуального окружения в сборку.
    # Она будет помещена в папку 'cv2' внутри .exe файла.
    datas=[('yolov8n.onnx', '.'), 
           ('.venv/Lib/site-packages/cv2', 'cv2')],
    # ^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
)
pyz = PYZ(a.pure)

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
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)