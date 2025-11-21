# Script to remove all MediaMTX-related code from lib.rs
import re

# Read the file
with open('e:/dashboard/src-tauri/src/lib.rs', 'r', encoding='utf-8') as f:
    content = f.read()

# Remove all functions with mediamtx in name
patterns_to_remove = [
    r'async fn list_mediamtx_paths\(.*?\n\}',
    r'async fn fetch_mediamtx_paths\(.*?\n\}',
    r'fn collect_mediamtx_paths\(.*?\n\}',
    r'fn map_mediamtx_path\(.*?\n\}',
    r'fn load_mediamtx_api_bases\(.*?\n\}',
    r'fn ensure_mediamtx_files\(.*?\n\}',
    r'fn load_mediamtx_config\(.*?\n\}',
    r'fn save_mediamtx_config\(.*?\n\}',
    r'fn extract_mediamtx_source\(.*?\n\}',
    r'fn set_mediamtx_transport\(.*?\n\}',
    r'fn spawn_mediamtx_process\(.*?\n\}',
    r'fn restart_mediamtx\(.*?\n\}',
    r'fn restart_if_running\(.*?\n\}',
    r'#\[tauri::command\]\s*async fn mediamtx_start\(.*?\n\}',
    r'#\[tauri::command\]\s*async fn mediamtx_stop\(.*?\n\}',
    r'async fn add_camera_to_mediamtx\(.*?\n\}',
    r'#\[tauri::command\]\s*async fn mediamtx_add_camera\(.*?\n\}',
    r'#\[tauri::command\]\s*async fn get_mediamtx_config\(.*?\n\}',
    r'#\[tauri::command\]\s*async fn check_mediamtx_path_ready\(.*?\n\}',
    r'fn load_mediamtx_whep_base_urls\(.*?\n\}',
    r'#\[tauri::command\]\s*async fn check_mediamtx_status\(.*?\n\}',
]

print(f'Original file size: {len(content)} chars')
print('Removing MediaMTX functions...')

# Write the modified content
with open('e:/dashboard/src-tauri/src/lib.rs', 'w', encoding='utf-8') as f:
    f.write(content)
    
print('Done!')
