use std::env;
use std::fs;
use std::path::Path;

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();

    if target_os == "windows" {
        println!("cargo:rustc-link-lib=Advapi32");
    }

    println!("cargo:rerun-if-changed=binaries/");

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

    // Copy the go2rtc binary matching the current target so runtime helpers can locate it.
    let (src_binary, dst_binary) = match target_os.as_str() {
        "windows" => ("binaries/windows/go2rtc.exe", "binaries/go2rtc.exe"),
        "linux" => ("binaries/linux/go2rtc", "binaries/go2rtc"),
        "macos" => ("binaries/macos/go2rtc", "binaries/go2rtc"),
        other => {
            println!(
                "cargo:warning=Unknown target OS: {}, skipping go2rtc bundling",
                other
            );
            tauri_build::build();
            return;
        }
    };

    if let Err(err) = fs::copy(src_binary, dst_binary) {
        println!(
            "cargo:warning=Failed to copy go2rtc binary from {} to {}: {}",
            src_binary, dst_binary, err
        );
    }

    tauri_build::build()
}
