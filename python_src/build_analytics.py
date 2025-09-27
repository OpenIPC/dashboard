# build_analytics.py

import os
import subprocess
import sys
from pathlib import Path
import site
import shutil

# Disable UPX to prevent double compression
# upx_exe = shutil.which('upx')
# if upx_exe:
#     os.rename(upx_exe, upx_exe + '.disabled')
#     def restore_upx():
#         if os.path.exists(upx_exe + '.disabled'):
#             os.rename(upx_exe + '.disabled', upx_exe)
#     import atexit
#     atexit.register(restore_upx)

BASE_DIR = Path(__file__).parent.resolve()

BASE_DIR = Path(__file__).parent.resolve()
# VENV_DIR больше не нужен
SRC_DIR = BASE_DIR
SRC_FILE = SRC_DIR / "analytics.py"
MODEL_FILE = SRC_DIR / "yolov8n.onnx"
DIST_PATH = BASE_DIR.parent / "extra" / "analytics"
REQUIREMENTS_DIR = BASE_DIR / "requirements"

BUILDS = {
    "cpu": "requirements_cpu.txt",
}

if sys.platform == "win32":
    BUILDS["dml"] = "requirements_dml.txt"

def run_command(command, cwd=None):
    """Runs a command as a list of arguments, which is safer."""
    print(f"--- Running command: {' '.join(command)}")
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        cwd=cwd,
        text=True,
        encoding='utf-8'
    )
    for line in process.stdout:
        print(line, end='')
    process.wait()
    if process.returncode != 0:
        raise subprocess.CalledProcessError(process.returncode, command)

# VVVVVV --- ИЗМЕНЕНИЕ: Ищем библиотеки в системном site-packages --- VVVVVV
def get_onnx_libs_path():
    """Finds the onnxruntime/capi path in the main Python environment."""
    # site.getsitepackages() возвращает список путей, обычно один
    for site_path in site.getsitepackages():
        potential_path = Path(site_path) / "onnxruntime" / "capi"
        if potential_path.exists():
            return potential_path
    # Если не нашли, возвращаем None
    return None
# ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ --- ^^^^^^



def create_and_build(name, req_file):
    print(f"\n{'='*20} Building: {name.upper()} {'='*20}")
    python_executable = sys.executable

    print(f"Installing dependencies for {name} from {req_file}...")
    install_command = [
        python_executable,
        "-m", "pip",
        "install",
        "-r", str(req_file)
    ]
    run_command(install_command)

    print(f"Running PyInstaller for {name}...")
    pyinstaller_command = [
        python_executable, "-m", "PyInstaller",
        "--noconfirm", "--onefile", "--noupx", "--upx-dir=/tmp/no_upx",
        f"--name=analytics_{name}",
        f"--distpath={DIST_PATH}",
        f"--add-data={MODEL_FILE}{os.pathsep}.",
        "--hidden-import=numpy.core._multiarray_umath",
    ]
    onnx_libs_path = get_onnx_libs_path()
    binary_sep = os.pathsep
    if name == "dml" and onnx_libs_path and onnx_libs_path.exists():
        print("Adding DirectML provider binaries...")
        for lib in ["onnxruntime_providers_shared.dll", "onnxruntime_providers_dml.dll", "DirectML.dll"]:
            if (onnx_libs_path / lib).exists():
                pyinstaller_command.append(f"--add-binary={(onnx_libs_path / lib)}{binary_sep}.")
    pyinstaller_command.append(str(SRC_FILE))
    run_command(pyinstaller_command)
    print(f"--- Successfully built {name} version! ---")


if __name__ == "__main__":
    if not SRC_FILE.exists():
        print(f"Error: Source file not found at {SRC_FILE}")
        sys.exit(1)

    if not MODEL_FILE.exists():
        print(f"Error: Model file not found at {MODEL_FILE}")
        print("Please make sure 'yolov8n.onnx' is placed in the 'python_src' directory.")
        sys.exit(1)
        
    # VENV_DIR.mkdir(exist_ok=True) <-- Больше не нужно
    DIST_PATH.mkdir(parents=True, exist_ok=True)
    
    for name, req_filename in BUILDS.items():
        create_and_build(name, REQUIREMENTS_DIR / req_filename)

    print(f"\n{'='*20} All builds for {sys.platform} completed! {'='*20}")
    print(f"Executables are located in: {DIST_PATH}")