# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['C:\\Users\\vavol\\openipc-dashboard\\python_src\\analytics.py'],
    pathex=[],
    binaries=[('C:\\Users\\vavol\\AppData\\Local\\Programs\\Python\\Python311\\Lib\\site-packages\\onnxruntime\\capi\\onnxruntime_providers_shared.dll', '.'), ('C:\\Users\\vavol\\AppData\\Local\\Programs\\Python\\Python311\\Lib\\site-packages\\onnxruntime\\capi\\DirectML.dll', '.')],
    datas=[('C:\\Users\\vavol\\openipc-dashboard\\python_src\\yolov8n.onnx', '.')],
    hiddenimports=['numpy.core._multiarray_umath'],
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
    name='analytics_dml',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=False,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
