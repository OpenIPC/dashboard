# build_analytics.py

import os
import subprocess
import sys
import venv
from pathlib import Path

BASE_DIR = Path(__file__).parent.resolve()
VENV_DIR = BASE_DIR / ".analytics_venvs"
SRC_DIR = BASE_DIR / "python_src"
SRC_FILE = SRC_DIR / "analytics.py"
MODEL_FILE = SRC_DIR / "yolov8n.onnx"
DIST_PATH = BASE_DIR / "extra" / "analytics"
REQUIREMENTS_DIR = BASE_DIR / "python_src" / "requirements"

BUILDS = {
    "cpu": "requirements_cpu.txt",
}

if sys.platform == "win32":
    BUILDS["dml"] = "requirements_dml.txt"

def run_command(command, shell=True, cwd=None):
    print(f"--- Running command: {' '.join(command) if isinstance(command, list) else command}")
    use_shell = isinstance(command, str) if sys.platform != "win32" else shell
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=use_shell, cwd=cwd, text=True, encoding='utf-8')
    for line in process.stdout:
        print(line, end='')
    process.wait()
    if process.returncode != 0:
        raise subprocess.CalledProcessError(process.returncode, command)

def get_onnx_libs_path(venv_path):
    if sys.platform == "win32":
        return venv_path / "Lib" / "site-packages" / "onnxruntime" / "capi"
    else:
        py_version = f"python{sys.version_info.major}.{sys.version_info.minor}"
        return venv_path / "lib" / py_version / "site-packages" / "onnxruntime" / "capi"

def create_and_build(name, req_file):
    print(f"\n{'='*20} Building: {name.upper()} {'='*20}")
    
    venv_path = VENV_DIR / name
    
    if sys.platform == "win32":
        python_executable = venv_path / "Scripts" / "python.exe"
        pip_executable = venv_path / "Scripts" / "pip.exe"
    else:
        python_executable = venv_path / "bin" / "python"
        pip_executable = venv_path / "bin" / "pip"

    if not venv_path.exists():
        print(f"Creating virtual environment for {name}...")
        venv.create(venv_path, with_pip=True)

    print(f"Installing dependencies for {name} from {req_file}...")
    run_command([str(pip_executable), "install", "-r", str(req_file)])

    print(f"Running PyInstaller for {name}...")
    
    pyinstaller_command = [
        str(python_executable), "-m", "PyInstaller",
        "--noconfirm", "--onefile",
        f"--name=analytics_{name}",
        f"--distpath={DIST_PATH}",
        f"--add-data={MODEL_FILE}{os.pathsep}.",
        # VVVVVV --- ФИНАЛЬНОЕ ИСПРАВЛЕНИЕ: Добавляем "скрытый" импорт --- VVVVVV
        "--hidden-import=numpy.core._multiarray_umath",
        # ^^^^^^ --- КОНЕЦ ФИНАЛЬНОГО ИСПРАВЛЕНИЯ --- ^^^^^^
    ]

    onnx_libs_path = get_onnx_libs_path(venv_path)
    binary_sep = os.pathsep

    if name == "dml" and onnx_libs_path.exists():
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
        
    VENV_DIR.mkdir(exist_ok=True)
    DIST_PATH.mkdir(exist_ok=True)
    
    for name, req_filename in BUILDS.items():
        create_and_build(name, REQUIREMENTS_DIR / req_filename)

    print(f"\n{'='*20} All builds for {sys.platform} completed! {'='*20}")
    print(f"Executables are located in: {DIST_PATH}")