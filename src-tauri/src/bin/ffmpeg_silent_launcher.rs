#![windows_subsystem = "windows"]

// ffmpeg-silent-launcher
// Small helper that spawns ffmpeg-silent.exe hiding the console window.
// Used as the command go2rtc invokes for audio transcoding on Windows.

use std::env;
use std::path::PathBuf;
use std::process::{Command, Stdio};

#[cfg(windows)]
use std::os::windows::process::CommandExt;

#[cfg(windows)]
extern "system" {
    fn FreeConsole() -> i32;
}

fn locate_wrapper() -> Option<PathBuf> {
    let current_exe = env::current_exe().ok()?;
    let exe_dir = current_exe.parent()?;

    let candidates = [
        exe_dir.join("ffmpeg-silent.exe"),
        exe_dir.join("binaries").join("ffmpeg-silent.exe"),
    ];

    for candidate in candidates {
        if candidate.exists() {
            return Some(candidate);
        }
    }

    None
}

fn main() {
    #[cfg(windows)]
    unsafe {
        FreeConsole();
    }

    let Some(wrapper_path) = locate_wrapper() else {
        eprintln!("ffmpeg-silent.exe not found next to dashboard app");
        std::process::exit(1);
    };

    let mut cmd = Command::new(&wrapper_path);
    cmd.args(env::args().skip(1));
    cmd.stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit());

    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        cmd.creation_flags(CREATE_NO_WINDOW);
    }

    match cmd.spawn() {
        Ok(mut child) => {
            if let Err(err) = child.wait() {
                eprintln!("ffmpeg-silent.exe wait failed: {}", err);
                std::process::exit(1);
            }
        }
        Err(err) => {
            eprintln!("ffmpeg-silent.exe spawn failed: {}", err);
            std::process::exit(1);
        }
    }
}
