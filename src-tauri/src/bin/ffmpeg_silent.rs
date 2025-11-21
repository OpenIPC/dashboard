#![windows_subsystem = "windows"]

// FFmpeg Silent Wrapper
// Launches ffmpeg without showing a console window on Windows.

use std::env;
use std::path::PathBuf;
use std::process::{Command, Stdio};

#[cfg(windows)]
use std::os::windows::process::CommandExt;

#[cfg(windows)]
extern "system" {
    fn FreeConsole() -> i32;
}

fn main() {
    #[cfg(windows)]
    unsafe {
        FreeConsole();
    }

    let real_ffmpeg = resolve_ffmpeg_path().unwrap_or_else(|| {
        eprintln!("ffmpeg.exe not found in bundled locations");
        std::process::exit(1);
    });

    let mut cmd = Command::new(&real_ffmpeg);
    cmd.args(env::args().skip(1))
        .stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit());

    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        cmd.creation_flags(CREATE_NO_WINDOW);
    }

    match cmd.status() {
        Ok(status) => {
            if !status.success() {
                std::process::exit(status.code().unwrap_or(1));
            }
        }
        Err(err) => {
            eprintln!("ffmpeg.exe execution failed: {}", err);
            std::process::exit(1);
        }
    }
}

fn resolve_ffmpeg_path() -> Option<PathBuf> {
    if let Ok(path) = env::var("REAL_FFMPEG_PATH") {
        let buf = PathBuf::from(path);
        if buf.exists() {
            return Some(buf);
        }
    }

    let current_exe = env::current_exe().ok()?;
    let exe_dir = current_exe.parent()?;

    let local_candidates = [
        exe_dir.join("ffmpeg.exe"),
        exe_dir.join("binaries").join("ffmpeg.exe"),
    ];

    for path in &local_candidates {
        if path.exists() {
            return Some(path.clone());
        }
    }

    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        let mut where_cmd = Command::new("where");
        where_cmd
            .arg("ffmpeg.exe")
            .stdout(Stdio::piped())
            .stderr(Stdio::null())
            .creation_flags(CREATE_NO_WINDOW);

        if let Ok(output) = where_cmd.output() {
            if output.status.success() {
                if let Some(first_line) = String::from_utf8_lossy(&output.stdout).lines().next() {
                    let trimmed = first_line.trim();
                    if !trimmed.is_empty() {
                        return Some(PathBuf::from(trimmed));
                    }
                }
            }
        }
    }

    None
}
