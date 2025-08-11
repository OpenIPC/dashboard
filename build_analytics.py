# build_analytics.py

import os
import subprocess
import sys
import venv
from pathlib import Path

BASE_DIR = Path(__file__).parent.resolve()
VENV_DIR = BASE_DIR / ".analytics_venvs"
# VVVVVV --- ИЗМЕНЕНИЕ 1: Определяем пути к папке с исходниками и к модели --- VVVVVV
# Это делает код чище и позволяет легко ссылаться на модель
SRC_DIR = BASE_DIR / "python_src"
SRC_FILE = SRC_DIR / "analytics.py"
MODEL_FILE = SRC_DIR / "yolov8n.onnx"
# ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ 1 --- ^^^^^^
DIST_PATH = BASE_DIR / "extra" / "analytics"
REQUIREMENTS_DIR = BASE_DIR / "python_src" / "requirements"

BUILDS = {
    "cpu": "requirements_cpu.txt",
    "cuda": "requirements_cuda.txt",
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
    # VVVVVV --- ИЗМЕНЕНИЕ 2: Добавляем флаг --add-data в команду PyInstaller --- VVVVVV
    # Эта строка говорит PyInstaller включить файл модели в исполняемый файл.
    # "MODEL_FILE:." означает: взять файл модели и положить его в корень (.) сборки.
    # os.pathsep используется для кросс-платформенной совместимости (';' для Windows, ':' для Linux).
    pyinstaller_command = [
        str(python_executable), "-m", "PyInstaller",
        "--noconfirm", "--onefile",
        f"--name=analytics_{name}",
        f"--distpath={DIST_PATH}",
        f"--add-data={MODEL_FILE}{os.pathsep}.", # <--- КЛЮЧЕВОЕ ИЗМЕНЕНИЕ
        str(SRC_FILE)
    ]
    # ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ 2 --- ^^^^^^
    run_command(pyinstaller_command)
    print(f"--- Successfully built {name} version! ---")


if __name__ == "__main__":
    if not SRC_FILE.exists():
        print(f"Error: Source file not found at {SRC_FILE}")
        sys.exit(1)

    # VVVVVV --- ИЗМЕНЕНИЕ 3: Добавляем проверку наличия файла модели --- VVVVVV
    if not MODEL_FILE.exists():
        print(f"Error: Model file not found at {MODEL_FILE}")
        print("Please make sure 'yolov8n.onnx' is placed in the 'python_src' directory.")
        sys.exit(1)
    # ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЯ 3 --- ^^^^^^
        
    VENV_DIR.mkdir(exist_ok=True)
    DIST_PATH.mkdir(exist_ok=True)
    
    for name, req_filename in BUILDS.items():
        create_and_build(name, REQUIREMENTS_DIR / req_filename)

    print(f"\n{'='*20} All builds for {sys.platform} completed! {'='*20}")
    print(f"Executables are located in: {DIST_PATH}")