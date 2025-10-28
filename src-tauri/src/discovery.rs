#![cfg_attr(not(feature = "device-discovery"), allow(dead_code))]

use std::collections::HashSet;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;

use futures_util::stream::{self, StreamExt};
use get_if_addrs::{self, IfAddr};
use serde_json::json;
use tauri::Emitter;
use tokio::net::TcpStream;
use tokio::sync::Mutex;
use tokio::time::{timeout, Duration};

const PORTS_TO_SCAN: &[u16] = &[554, 8554, 7447, 80, 8000, 8080, 8899, 2020];
const TCP_TIMEOUT_MS: u64 = 250;
const MAX_CONCURRENT_SCANS: usize = 96;

#[tauri::command]
pub async fn discover_cameras(app_handle: tauri::AppHandle) -> Result<(), String> {
    println!("[VMS] discover_cameras: start");

    let app_handle_clone = app_handle.clone();

    tokio::spawn(async move {
        let interfaces = match get_if_addrs::get_if_addrs() {
            Ok(interfaces) => interfaces,
            Err(e) => {
                println!("[VMS] Error getting interfaces: {}", e);
                let _ = app_handle_clone.emit(
                    "device-discovery-finished",
                    json!({ "status": "error", "reason": "interfaces", "details": e.to_string() }),
                );
                return;
            }
        };

        let mut targets: Vec<String> = Vec::new();

        for iface in interfaces {
            if let IfAddr::V4(ipv4) = iface.addr {
                if ipv4.ip.is_loopback() {
                    continue;
                }

                println!("[VMS] Scanning subnet via interface {}", ipv4.ip);

                let octets = ipv4.ip.octets();
                for host in 1u8..=254 {
                    if host == octets[3] {
                        continue;
                    }
                    let ip = format!("{}.{}.{}.{}", octets[0], octets[1], octets[2], host);
                    targets.push(ip);
                }
            }
        }

        if targets.is_empty() {
            println!("[VMS] No network targets available for discovery");
            let _ = app_handle_clone.emit(
                "device-discovery-finished",
                json!({ "status": "no-targets" }),
            );
            return;
        }

        let total_targets = targets.len();
        println!(
            "[VMS] discover_cameras: scanning {} addresses",
            total_targets
        );

        let found_cameras = Arc::new(Mutex::new(HashSet::<String>::new()));
        let progress_counter = Arc::new(AtomicUsize::new(0));
        let http_client = match reqwest::Client::builder()
            .timeout(Duration::from_millis(700))
            .danger_accept_invalid_certs(true)
            .build()
        {
            Ok(client) => Some(Arc::new(client)),
            Err(e) => {
                println!(
                    "[VMS] Failed to build HTTP client for discovery metadata: {}",
                    e
                );
                None
            }
        };

        let _ = app_handle_clone.emit(
            "device-discovery-progress",
            json!({ "scanned": 0, "total": total_targets }),
        );

        let app_handle_for_tasks = app_handle_clone.clone();
        let found_cameras_for_tasks = found_cameras.clone();
        let progress_counter_for_tasks = progress_counter.clone();
        let http_client_for_tasks = http_client.clone();

        stream::iter(targets.into_iter())
            .for_each_concurrent(MAX_CONCURRENT_SCANS, move |ip| {
                let app_handle = app_handle_for_tasks.clone();
                let found_cameras = found_cameras_for_tasks.clone();
                let progress_counter = progress_counter_for_tasks.clone();
                let http_client = http_client_for_tasks.clone();

                async move {
                    scan_ip(
                        ip.clone(),
                        app_handle.clone(),
                        found_cameras.clone(),
                        http_client.clone(),
                    )
                    .await;

                    let scanned = progress_counter.fetch_add(1, Ordering::Relaxed) + 1;
                    if scanned % 10 == 0 || scanned == total_targets {
                        let _ = app_handle.emit(
                            "device-discovery-progress",
                            json!({ "scanned": scanned, "total": total_targets }),
                        );
                    }
                }
            })
            .await;

        println!("[VMS] discover_cameras: scan completed");
        let found_total = found_cameras.lock().await.len();
        let _ = app_handle_clone.emit(
            "device-discovery-finished",
            json!({ "status": "completed", "found": found_total, "scanned": total_targets }),
        );
    });

    println!("[VMS] discover_cameras: task started");
    Ok(())
}

async fn scan_ip(
    ip: String,
    app_handle: tauri::AppHandle,
    found_cameras: Arc<Mutex<HashSet<String>>>,
    http_client: Option<Arc<reqwest::Client>>,
) {
    for &port in PORTS_TO_SCAN {
        let address = format!("{}:{}", ip, port);

        match timeout(
            Duration::from_millis(TCP_TIMEOUT_MS),
            TcpStream::connect(&address),
        )
        .await
        {
            Ok(Ok(stream)) => {
                drop(stream);

                let already_known = {
                    let mut guard = found_cameras.lock().await;
                    !guard.insert(ip.clone())
                };

                if already_known {
                    return;
                }

                let friendly_name = identify_camera(&ip, port, http_client.clone())
                    .await
                    .unwrap_or_else(|| format!("Camera {}", ip));

                println!("[VMS] Found camera candidate {} on port {}", ip, port);

                let payload = json!({
                    "ip": ip,
                    "name": friendly_name,
                    "protocol": "onvif",
                    "onvifPort": infer_onvif_port(port),
                    "port": infer_rtsp_port(port),
                    "detectedPort": port
                });

                if let Err(e) = app_handle.emit("device-found", payload) {
                    println!("[VMS] Error sending device-found event: {}", e);
                }

                return;
            }
            Ok(Err(e)) => {
                if e.kind() != std::io::ErrorKind::ConnectionRefused {
                    println!("[VMS] {}:{} connection error: {}", ip, port, e);
                }
            }
            Err(_) => {
                // timeout
            }
        }
    }
}

async fn identify_camera(
    ip: &str,
    port: u16,
    http_client: Option<Arc<reqwest::Client>>,
) -> Option<String> {
    let client = http_client?;
    let mut urls = vec![format!("http://{}:{}/", ip, port)];

    if port != 80 {
        urls.push(format!("http://{}:{}/onvif/device_service", ip, port));
    } else {
        urls.push(format!("http://{}/onvif/device_service", ip));
    }

    for url in urls {
        match client.get(&url).send().await {
            Ok(response) => {
                if let Some(server_header) = response.headers().get(reqwest::header::SERVER) {
                    if let Ok(server) = server_header.to_str() {
                        if !server.trim().is_empty() {
                            return Some(server.trim().to_string());
                        }
                    }
                }

                if response.status() == reqwest::StatusCode::UNAUTHORIZED {
                    if let Some(auth_header) =
                        response.headers().get(reqwest::header::WWW_AUTHENTICATE)
                    {
                        if let Ok(auth) = auth_header.to_str() {
                            if let Some(name) = extract_realm(auth) {
                                return Some(name);
                            }
                        }
                    }
                }

                if let Ok(body) = response.text().await {
                    if let Some(name) = extract_model_from_body(&body) {
                        return Some(name);
                    }
                }
            }
            Err(e) => {
                if e.is_timeout() {
                    continue;
                }
            }
        }
    }

    None
}

fn extract_realm(header: &str) -> Option<String> {
    header
        .split("realm=")
        .nth(1)
        .and_then(|rest| rest.trim_start_matches('"').split('"').next())
        .map(|s| s.trim().to_string())
}

fn extract_model_from_body(body: &str) -> Option<String> {
    for marker in ["<td>Model</td>", "<model>", "<td>Device Type</td>"] {
        if let Some(pos) = body.find(marker) {
            let snippet = &body[pos..].chars().take(200).collect::<String>();
            let mut cleaned = snippet.replace(marker, "");
            for token in ["<td>", "</td>", "<th>", "</th>", "<model>", "</model>"] {
                cleaned = cleaned.replace(token, " ");
            }
            let text = cleaned
                .split_whitespace()
                .take(5)
                .collect::<Vec<_>>()
                .join(" ");
            if !text.is_empty() {
                return Some(text.trim().to_string());
            }
        }
    }
    None
}

fn infer_onvif_port(port: u16) -> u16 {
    match port {
        80 | 8000 | 8080 | 8899 | 2020 => port,
        _ => 80,
    }
}

fn infer_rtsp_port(port: u16) -> u16 {
    match port {
        554 | 8554 | 7447 => port,
        _ => 554,
    }
}
