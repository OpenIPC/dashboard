use std::env;
use std::fs;
use std::path::Path;

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();

    println!("cargo:rerun-if-changed=binaries/");
    println!("cargo:rerun-if-changed=mediamtx/");

    // Ensure the binaries directory exists
    let binaries_dir = Path::new("binaries");
    if !binaries_dir.exists() {
        fs::create_dir_all(binaries_dir).expect("Failed to create binaries directory");
    }

    // Guarantee the gstreamer resources folder has at least one visible file so the
    // Tauri resource glob does not fail on platforms where we skip bundling.
    let gstreamer_dir = Path::new("resources/gstreamer");
    if !gstreamer_dir.exists() {
        fs::create_dir_all(gstreamer_dir).expect("Failed to create gstreamer resources directory");
    }

    let mut has_visible_files = false;
    if let Ok(entries) = fs::read_dir(gstreamer_dir) {
        for entry in entries.flatten() {
            let name = entry.file_name();
            if let Ok(metadata) = entry.metadata() {
                if metadata.is_file() {
                    if let Some(name_str) = name.to_str() {
                        if !name_str.starts_with('.') {
                            has_visible_files = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if !has_visible_files {
        let placeholder = gstreamer_dir.join("README.txt");
        if !placeholder.exists() {
            fs::write(&placeholder, "Optional GStreamer runtime placeholder.\n")
                .expect("Failed to create gstreamer placeholder file");
        }
    }

    // Copy appropriate MediaMTX binary based on target platform
    let (src_binary, dst_binary) = match target_os.as_str() {
        "windows" => ("binaries/windows/mediamtx.exe", "binaries/mediamtx.exe"),
        "linux" => ("binaries/linux/mediamtx", "binaries/mediamtx"),
        "macos" => ("binaries/macos/mediamtx", "binaries/mediamtx"),
        _ => {
            println!(
                "cargo:warning=Unknown target OS: {}, skipping binary copy",
                target_os
            );
            tauri_build::build();
            return;
        }
    };

    // Copy binary if source exists
    if Path::new(src_binary).exists() {
        if let Err(e) = fs::copy(src_binary, dst_binary) {
            println!("cargo:warning=Failed to copy MediaMTX binary: {}", e);
        } else {
            println!("cargo:rustc-env=MEDIAMTX_BINARY_PATH={}", dst_binary);
        }
    } else {
        println!(
            "cargo:warning=MediaMTX binary not found for {}: {}",
            target_os, src_binary
        );
        println!("cargo:warning=Run 'npm run download-mediamtx' to download required binaries");
    }

    tauri_build::build()
}
