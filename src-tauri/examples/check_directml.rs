use ort::execution_providers::{DirectMLExecutionProvider, ExecutionProvider};

fn main() {
    // Configure PATH to use bundled ONNX Runtime DLL
    #[cfg(windows)]
    {
        use std::env;
        if let Ok(exe_path) = env::current_exe() {
            if let Some(exe_dir) = exe_path.parent() {
                let binaries_dir = exe_dir.join("binaries");
                if binaries_dir.exists() {
                    let onnxruntime_dll = binaries_dir.join("onnxruntime.dll");
                    if onnxruntime_dll.exists() {
                        println!("📦 Using ONNX Runtime from: {}", onnxruntime_dll.display());
                        env::set_var("ORT_DYLIB_PATH", onnxruntime_dll);
                    }
                } else {
                    println!(
                        "⚠ Binaries directory not found at: {}",
                        binaries_dir.display()
                    );
                }
            }
        }
    }

    println!("=== DirectML Diagnostic Tool ===\n");

    // Check if DirectML is available
    println!("1. Checking DirectML availability...");
    let dml = DirectMLExecutionProvider::default();

    match dml.is_available() {
        Ok(true) => {
            println!("   ✓ DirectML is AVAILABLE");
            println!("   → Your system supports GPU acceleration!");
        }
        Ok(false) => {
            println!("   ✗ DirectML is NOT available");
            println!("   → Possible reasons:");
            println!("      - DirectML.dll not found in system");
            println!("      - GPU drivers outdated");
            println!("      - Windows version too old (need 1903+)");
            println!("      - DirectX 12 not supported by GPU");
        }
        Err(e) => {
            println!("   ✗ Error checking DirectML: {}", e);
            println!("   → ONNX Runtime may not be properly initialized");
        }
    }

    println!("\n2. System Information:");
    println!("   OS: {}", std::env::consts::OS);
    println!("   Architecture: {}", std::env::consts::ARCH);

    println!("\n3. Recommendations:");
    println!("   - Update GPU drivers to latest version");
    println!("   - Ensure Windows 10 1903+ or Windows 11");
    println!("   - Install DirectX 12 runtime");
    println!("   - Try setting analytics_provider to 'dml' instead of 'auto'");

    println!("\n=== End of Diagnostic ===");
}
