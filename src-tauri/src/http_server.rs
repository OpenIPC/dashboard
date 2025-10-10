use once_cell::sync::Lazy;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use tauri::command;
use tokio::io::{AsyncReadExt, AsyncSeekExt, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio::task;

// Global state to track if HTTP server is running
static HTTP_SERVER_RUNNING: Lazy<Arc<AtomicBool>> = Lazy::new(|| Arc::new(AtomicBool::new(false)));

// Simple HTTP server using tokio
#[command]
pub async fn start_http_server() -> Result<bool, String> {
    // Check if server is already running
    if HTTP_SERVER_RUNNING.load(Ordering::Relaxed) {
        println!("HTTP server already running");

        // Test if server is actually responding by checking port
        if let Ok(_) = tokio::net::TcpStream::connect("127.0.0.1:8080").await {
            return Ok(true);
        } else {
            println!("HTTP server marked as running but not responding, restarting...");
            HTTP_SERVER_RUNNING.store(false, Ordering::Relaxed);
        }
    }

    // Get the path to the recordings directory directly from settings
    let settings = crate::get_app_settings_internal()
        .await
        .map_err(|e| format!("Failed to get settings: {}", e))?;
    let recordings_path = settings
        .get("recordingsPath")
        .and_then(|v| v.as_str())
        .unwrap_or("E:\\VMS")
        .to_string();

    println!(
        "Starting HTTP server to serve recordings from: {}",
        recordings_path
    );

    // Create the recordings directory if it doesn't exist
    if !Path::new(&recordings_path).exists() {
        if let Err(e) = std::fs::create_dir_all(&recordings_path) {
            return Err(format!("Failed to create recordings directory: {}", e));
        }
    }

    // Verify the directory exists and has files
    match std::fs::read_dir(&recordings_path) {
        Ok(entries) => {
            let file_count = entries
                .filter_map(|e| e.ok())
                .filter(|e| e.path().is_file())
                .count();
            println!(
                "Directory {} contains {} files",
                recordings_path, file_count
            );
        }
        Err(e) => {
            return Err(format!(
                "Cannot read recordings directory {}: {}",
                recordings_path, e
            ));
        }
    }

    // Start simple HTTP server in background
    let recordings_path_clone = recordings_path.clone();
    let server_running = HTTP_SERVER_RUNNING.clone();

    let _server_handle = task::spawn(async move {
        println!(
            "Starting HTTP server on 127.0.0.1:8080 serving {}",
            recordings_path_clone
        );

        // Mark server as running before starting
        server_running.store(true, Ordering::Relaxed);

        match TcpListener::bind("127.0.0.1:8080").await {
            Ok(listener) => {
                println!("HTTP server bound successfully to port 8080");

                loop {
                    match listener.accept().await {
                        Ok((mut socket, addr)) => {
                            println!("Connection from: {}", addr);
                            let recordings_path = recordings_path_clone.clone();

                            tokio::spawn(async move {
                                if let Err(e) =
                                    handle_http_request(&mut socket, &recordings_path).await
                                {
                                    eprintln!("Error handling request: {}", e);
                                }
                            });
                        }
                        Err(e) => {
                            eprintln!("Failed to accept connection: {}", e);
                        }
                    }
                }
            }
            Err(e) => {
                eprintln!("Failed to bind HTTP server to port 8080: {}", e);
                server_running.store(false, Ordering::Relaxed);
            }
        }
    });

    // Store the handle to prevent the server from being dropped
    std::mem::forget(_server_handle);

    // Give the server more time to start
    tokio::time::sleep(tokio::time::Duration::from_millis(1000)).await;

    // Verify server is actually listening
    match tokio::net::TcpStream::connect("127.0.0.1:8080").await {
        Ok(_) => {
            println!("HTTP server started successfully and is responding");
            Ok(true)
        }
        Err(e) => {
            HTTP_SERVER_RUNNING.store(false, Ordering::Relaxed);
            Err(format!(
                "HTTP server failed to start or is not responding: {}",
                e
            ))
        }
    }
}

async fn handle_http_request(
    socket: &mut tokio::net::TcpStream,
    recordings_path: &str,
) -> Result<(), Box<dyn std::error::Error>> {
    let mut buffer = [0; 4096]; // Увеличиваем буфер для заголовков
    let bytes_read = socket.read(&mut buffer).await?;

    let request = String::from_utf8_lossy(&buffer[..bytes_read]);
    let lines: Vec<&str> = request.split("\r\n").collect();

    if let Some(first_line) = lines.first() {
        let parts: Vec<&str> = first_line.split(' ').collect();
        if parts.len() >= 2 {
            let method = parts[0];
            let path = parts[1];

            println!("HTTP {} {}", method, path);

            // Парсим заголовки
            let mut range_header: Option<&str> = None;
            for line in &lines[1..] {
                if line.to_lowercase().starts_with("range:") {
                    range_header = Some(line);
                    break;
                }
            }

            if method == "GET" || method == "HEAD" {
                if path == "/" {
                    // Root path - return simple message
                    let response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\n\r\nHTTP Server for VMS recordings is running";
                    socket.write_all(response.as_bytes()).await?;
                } else {
                    // File request
                    let file_path = path.trim_start_matches('/');
                    let full_path = PathBuf::from(recordings_path).join(file_path);

                    if full_path.exists() && full_path.is_file() {
                        serve_file_with_range(
                            socket,
                            &full_path,
                            file_path,
                            range_header,
                            method == "HEAD",
                        )
                        .await?;
                    } else {
                        println!("File not found: {}", file_path);
                        let response = "HTTP/1.1 404 Not Found\r\nAccess-Control-Allow-Origin: *\r\n\r\nFile not found";
                        socket.write_all(response.as_bytes()).await?;
                    }
                }
            } else if method == "OPTIONS" {
                // CORS preflight
                let response = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Range\r\nAccess-Control-Allow-Methods: GET, HEAD, OPTIONS\r\nAccept-Ranges: bytes\r\n\r\n";
                socket.write_all(response.as_bytes()).await?;
            }
        }
    }

    Ok(())
}

async fn serve_file_with_range(
    socket: &mut tokio::net::TcpStream,
    file_path: &PathBuf,
    filename: &str,
    range_header: Option<&str>,
    head_only: bool,
) -> Result<(), Box<dyn std::error::Error>> {
    use tokio::fs::File;
    use tokio::io::AsyncSeekExt;

    let mut file = File::open(file_path).await?;
    let file_size = file.metadata().await?.len();

    let mime_type = if filename.ends_with(".mp4") {
        "video/mp4"
    } else {
        "application/octet-stream"
    };

    if let Some(range_str) = range_header {
        // Парсим Range заголовок
        if let Some(range_part) = range_str.split(':').nth(1) {
            let range_part = range_part.trim();
            if range_part.starts_with("bytes=") {
                let bytes_part = &range_part[6..]; // убираем "bytes="

                if let Some((start_str, end_str)) = bytes_part.split_once('-') {
                    let start: u64 = start_str.parse().unwrap_or(0);
                    let end: u64 = if end_str.is_empty() {
                        file_size - 1
                    } else {
                        end_str.parse().unwrap_or(file_size - 1).min(file_size - 1)
                    };

                    let content_length = end - start + 1;

                    // Отправляем 206 Partial Content
                    let response = format!(
                        "HTTP/1.1 206 Partial Content\r\nContent-Type: {}\r\nContent-Length: {}\r\nContent-Range: bytes {}-{}/{}\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Range\r\nAccess-Control-Expose-Headers: Content-Range\r\nAccept-Ranges: bytes\r\n\r\n",
                        mime_type, content_length, start, end, file_size
                    );

                    socket.write_all(response.as_bytes()).await?;

                    if !head_only {
                        // Читаем и отправляем нужный диапазон
                        file.seek(std::io::SeekFrom::Start(start)).await?;
                        let mut remaining = content_length;
                        let mut buffer = vec![0u8; 65536]; // 64KB буфер

                        while remaining > 0 {
                            let to_read = (remaining as usize).min(buffer.len());
                            let bytes_read = file.read(&mut buffer[..to_read]).await?;
                            if bytes_read == 0 {
                                break;
                            }
                            socket.write_all(&buffer[..bytes_read]).await?;
                            remaining -= bytes_read as u64;
                        }
                    }

                    println!(
                        "Served range {}-{} of file: {} ({} bytes)",
                        start, end, filename, content_length
                    );
                    return Ok(());
                }
            }
        }
    }

    // Обычный запрос без Range
    let response = format!(
        "HTTP/1.1 200 OK\r\nContent-Type: {}\r\nContent-Length: {}\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Range\r\nAccept-Ranges: bytes\r\n\r\n",
        mime_type, file_size
    );

    socket.write_all(response.as_bytes()).await?;

    if !head_only {
        // Отправляем весь файл
        let mut buffer = vec![0u8; 65536]; // 64KB буфер
        loop {
            let bytes_read = file.read(&mut buffer).await?;
            if bytes_read == 0 {
                break;
            }
            socket.write_all(&buffer[..bytes_read]).await?;
        }
    }

    println!("Served full file: {} ({} bytes)", filename, file_size);
    Ok(())
}

// Helper function to check if HTTP server is running without starting it
#[command]
pub async fn check_http_server() -> Result<bool, String> {
    // Check if server is running
    let is_running = HTTP_SERVER_RUNNING.load(Ordering::Relaxed);

    if is_running {
        // Also test if server is actually responding
        match tokio::net::TcpStream::connect("127.0.0.1:8080").await {
            Ok(_) => Ok(true),
            Err(_) => {
                HTTP_SERVER_RUNNING.store(false, Ordering::Relaxed);
                Ok(false)
            }
        }
    } else {
        Ok(false)
    }
}
