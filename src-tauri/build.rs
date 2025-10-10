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
    
    // Copy appropriate MediaMTX binary based on target platform
    let (src_binary, dst_binary) = match target_os.as_str() {
        "windows" => ("binaries/windows/mediamtx.exe", "binaries/mediamtx.exe"),
        "linux" => ("binaries/linux/mediamtx", "binaries/mediamtx"),
        "macos" => ("binaries/macos/mediamtx", "binaries/mediamtx"),
        _ => {
            println!("cargo:warning=Unknown target OS: {}, skipping binary copy", target_os);
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
        println!("cargo:warning=MediaMTX binary not found for {}: {}", target_os, src_binary);
        println!("cargo:warning=Run 'npm run download-mediamtx' to download required binaries");
    }

    tauri_build::build()
}
