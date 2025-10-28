}

fn ensure_mediamtx_files(state: &mut MediaMtxState) -> Result<(), String> {
                let body = response
                    .text()
                    .await
                    .unwrap_or_else(|_| String::from("<empty>"));
                last_error = Some(format!(
                    "MediaMTX API {} responded with status {} and body: {}",
                    url, status, body
                ));
                continue;
            }

            let payload: serde_json::Value = response
                .json()
                .await
                .map_err(|e| format!("Failed to parse MediaMTX response from {}: {}", url, e))?;

            let mut result: Vec<StreamPathStatus> = Vec::new();

            if let Some(items) = payload.get("items").and_then(|v| v.as_array()) {
                for item in items {
                    let Some(name) = item.get("name").and_then(|v| v.as_str()) else {
                        continue;
                    };

                    let ready = item.get("ready").and_then(|v| v.as_bool()).unwrap_or(false);

                    let mut protocols: BTreeSet<String> = BTreeSet::new();
                    let mut reader_count = 0usize;

                    if let Some(readers) = item.get("readers").and_then(|v| v.as_array()) {
                        reader_count = readers.len();
                        for reader in readers {
                            if let Some(proto) = reader.get("type").and_then(|v| v.as_str()) {
                                protocols.insert(proto.to_string());
                            } else if let Some(proto) =
                                reader.get("protocol").and_then(|v| v.as_str())
                            {
                                protocols.insert(proto.to_string());
                            }
                        }
                    }

                    let publisher_kind = item
                        .get("source")
                        .and_then(|v| v.get("type").or_else(|| v.get("kind")))
                        .and_then(|v| v.as_str())
                        .map(|s| s.to_string());

                    let source_url = item
                        .get("source")
                        .and_then(|v| v.get("url"))
                        .and_then(|v| v.as_str())
                        .map(|s| s.to_string());

                    let on_demand = item
                        .get("source")
                        if !exe_path.exists() {
                            let mut copied = false;

                            let resource_candidates: &[&str] = &[
                                #[cfg(windows)]
                                "go2rtc/go2rtc.exe",
                                #[cfg(not(windows))]
                                "go2rtc/go2rtc",
                                #[cfg(windows)]
                                "binaries/go2rtc.exe",
                                #[cfg(windows)]
                                "binaries/go2rtc-x86_64-pc-windows-msvc.exe",
                                #[cfg(not(windows))]
                                "binaries/go2rtc",
                            ];

                            if let Ok(resource_root) = app_handle.path().resource_dir() {
                                for candidate in resource_candidates {
                                    let path = resource_root.join(candidate);
                                    if path.exists() {
                                        match fs::copy(&path, &exe_path) {
                                            Ok(_) => {
                                                println!(
                                                    "Copied go2rtc binary from {:?} to {:?}",
                                                    path, exe_path
                                                );
                                                copied = true;
                                                break;
                                            }
                                            Err(err) => {
                                                println!(
                                                    "Failed to copy go2rtc binary from {:?}: {}",
                                                    path, err
                                                );
                                            }
                                        }
                                    }
                                }
                            }

                            if !copied {
                                if let Ok(cur) = std::env::current_dir() {
                                    let bundled = cur
                                        .join("src-tauri")
                                        .join("go2rtc")
                                        .join(GO2RTC_BINARY_NAME);
                                    if bundled.exists() {
                                        if let Err(err) = fs::copy(&bundled, &exe_path) {
                                            println!(
                                                "Failed to copy go2rtc binary from {:?}: {}",
                                                bundled, err
                                            );
                                        } else {
                                            println!(
                                                "Copied go2rtc binary from {:?} to {:?}",
                                                bundled, exe_path
                                            );
                                            copied = true;
                                        }
                                    }
                                }
                            }

                            if !copied {
                                match Self::download_binary(&exe_path) {
                                    Ok(_) => {
                                        println!("Downloaded go2rtc binary to {:?}", exe_path);
                                        copied = true;
                                    }
                                    Err(err) => {
                                        println!(
                                            "Failed to prepare go2rtc binary automatically: {}",
                                            err
                                        );
                                    }
                                }
                            }

                            if !copied {
                                println!(
                                    "go2rtc binary was not found in bundled resources; expected at {:?}",
                                    exe_path
                                );
                            }
                        }
    }

    Err(last_error.unwrap_or_else(|| {
        "go2rtc control API is unavailable".to_string()
    }))
}

fn collect_consumer_info(value: &serde_json::Value, protocols: &mut BTreeSet<String>) -> usize {
    if let Some(map) = value.as_object() {
        for consumer in map.values() {
            if let Some(proto) = consumer
                .get("protocol")
                .or_else(|| consumer.get("proto"))
                .or_else(|| consumer.get("type"))
                .and_then(|v| v.as_str())
            {
                protocols.insert(proto.to_string());
            }
        }
        map.len()
    } else if let Some(array) = value.as_array() {
        for consumer in array {
            if let Some(proto) = consumer
                .get("protocol")
                .or_else(|| consumer.get("proto"))
                .or_else(|| consumer.get("type"))
                .and_then(|v| v.as_str())
            {
                protocols.insert(proto.to_string());
            } else if let Some(proto) = consumer.as_str() {
                if !proto.is_empty() {
                    protocols.insert(proto.to_string());
                }
            }
        }
        array.len()
    } else if let Some(proto) = value.as_str() {
        if !proto.is_empty() {
            protocols.insert(proto.to_string());
            1
        } else {
            0
        }
    } else {
        0
    }
}

fn extract_url_from_value(value: &serde_json::Value) -> Option<String> {
    if let Some(s) = value.as_str() {
        if !s.is_empty() {
            return Some(s.to_string());
        }
    }

    if let Some(array) = value.as_array() {
        for entry in array {
            if let Some(url) = extract_url_from_value(entry) {
                return Some(url);
            }
        }
    }

    if let Some(map) = value.as_object() {
        for key in ["url", "source", "input", "stream", "uri"] {
            if let Some(url) = map
                .get(key)
                .and_then(|v| v.as_str())
                .map(|s| s.to_string())
            {
                return Some(url);
            }
        }

        for entry in map.values() {
            if let Some(url) = extract_url_from_value(entry) {
                return Some(url);
            }
        }
    }

    None
}

fn map_go2rtc_stream(
    name: &str,
    entry: &serde_json::Value,
    settings: &StreamingSettings,
) -> StreamPathStatus {
    let ready = entry
        .get("status")
        .and_then(|v| v.as_str())
        .map(|s| matches!(s.to_ascii_lowercase().as_str(), "online" | "ready" | "active"))
        .or_else(|| entry.get("state").and_then(|v| v.as_str()).map(|s| {
            matches!(s.to_ascii_lowercase().as_str(), "ok" | "online" | "ready")
        }))
        .or_else(|| entry.get("ready").and_then(|v| v.as_bool()))
        .or_else(|| entry.get("online").and_then(|v| v.as_bool()))
        .unwrap_or(false);

    let mut protocols: BTreeSet<String> = BTreeSet::new();
    let mut reader_count = 0usize;

    if let Some(consumers) = entry.get("consumers") {
        reader_count += collect_consumer_info(consumers, &mut protocols);
    }

    if let Some(clients) = entry.get("clients") {
        reader_count += collect_consumer_info(clients, &mut protocols);
    }

    if reader_count == 0 {
        if let Some(viewers) = entry.get("viewers") {
            reader_count += collect_consumer_info(viewers, &mut protocols);
        }
    }

    let publisher_kind = entry
        .get("producer")
        .and_then(|v| v.as_str())
        .map(|s| s.to_string())
        .or_else(|| {
            entry
                .get("producers")
                .and_then(|v| v.as_object())
                .and_then(|map| map.keys().next().map(|k| k.to_string()))
        });

    let source_url = entry
        .get("source")
        .and_then(extract_url_from_value)
        .or_else(|| entry.get("url").and_then(|v| v.as_str()).map(|s| s.to_string()))
        .or_else(|| {
            entry
                .get("producers")
                .and_then(|v| v.as_object())
                .and_then(|map| {
                    map.values()
                        .find_map(|producer| extract_url_from_value(producer))
                })
        });

    let on_demand = entry
        .get("on_demand")
        .or_else(|| entry.get("onDemand"))
        .and_then(|v| v.as_bool())
        .unwrap_or(settings.enable_on_demand);

    StreamPathStatus {
        name: name.to_string(),
        ready,
        reader_count,
        active_protocols: protocols.into_iter().collect(),
        publisher_kind,
        source_url,
        on_demand,
    }
}
#[tauri::command]
async fn list_mediamtx_paths(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<Vec<StreamPathStatus>, String> {
    let settings = load_streaming_settings().await?;
    match settings.provider {
        StreamingProvider::MediaMtx => fetch_mediamtx_paths(&mediamtx_state).await,
        StreamingProvider::Go2Rtc => fetch_go2rtc_paths(&go2rtc_state, &settings).await,
    }
}

fn load_mediamtx_api_bases(
    state: &State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Vec<String> {
    const DEFAULTS: &[&str] = &["http://127.0.0.1:8889", "http://127.0.0.1:9997"];

    let mut bases: BTreeSet<String> = DEFAULTS.iter().map(|s| s.to_string()).collect();

    let state_guard = match state.lock() {
        Ok(guard) => guard,
        Err(err) => {
            println!(
                "[load_mediamtx_api_bases] Failed to lock MediaMTX state: {}",
                err
            );
            return bases.into_iter().collect();
        }
    };

    let config_path = state_guard.config_path.clone();
    drop(state_guard);

    let config_raw = match fs::read_to_string(&config_path) {
        Ok(content) => content,
        Err(err) => {
            println!(
                "[load_mediamtx_api_bases] Failed to read MediaMTX config {:?}: {}",
                config_path, err
            );
            return bases.into_iter().collect();
        }
    };

    let yaml: serde_yaml::Value = match serde_yaml::from_str(&config_raw) {
        Ok(value) => value,
        Err(err) => {
            println!(
                "[load_mediamtx_api_bases] Failed to parse MediaMTX config {:?}: {}",
                config_path, err
            );
            return bases.into_iter().collect();
        }
    };

    if let Some(api_addr) = yaml.get("apiAddress").and_then(|v| v.as_str()) {
        if let Some(base) = parse_address_to_base(api_addr) {
            bases.insert(base);
        }
    }

    if let Some(api_addresses) = yaml.get("apiAddresses").and_then(|v| v.as_sequence()) {
        for entry in api_addresses.iter().filter_map(|v| v.as_str()) {
            if let Some(base) = parse_address_to_base(entry) {
                bases.insert(base);
            }
        }
    }

    bases.into_iter().collect()
}

fn load_go2rtc_api_bases(
    state: &State<'_, Arc<StdMutex<Go2RtcState>>>,
    settings: &StreamingSettings,
) -> Vec<String> {
    let mut bases: BTreeSet<String> = settings
        .go2rtc_api_bases()
        .into_iter()
        .filter(|s| !s.is_empty())
        .collect();

    if bases.is_empty() {
        bases.insert("http://127.0.0.1:1984".to_string());
    }

    let state_guard = match state.lock() {
        Ok(guard) => guard,
        Err(err) => {
            println!("[go2rtc] Failed to lock state: {}", err);
            return bases.into_iter().collect();
        }
    };
    let config_path = state_guard.config_path.clone();
    drop(state_guard);

    let config_raw = match fs::read_to_string(&config_path) {
        Ok(content) => content,
        Err(err) => {
            println!(
                "[go2rtc] Failed to read config {:?}: {}",
                config_path, err
            );
            return bases.into_iter().collect();
        }
    };

    let yaml: serde_yaml::Value = match serde_yaml::from_str(&config_raw) {
        Ok(value) => value,
        Err(err) => {
            println!(
                "[go2rtc] Failed to parse config {:?}: {}",
                config_path, err
            );
            return bases.into_iter().collect();
        }
    };

    if let Some(api_node) = yaml.get("api") {
        if let Some(addr) = api_node.as_str() {
            if let Some(base) = parse_address_to_base(addr) {
                bases.insert(base);
            }
        } else if let Some(map) = api_node.as_mapping() {
            for key in ["listen", "address", "addr"] {
                if let Some(value) = map
                    .get(&serde_yaml::Value::String(key.to_string()))
                    .and_then(|v| v.as_str())
                {
                    if let Some(base) = parse_address_to_base(value) {
                        bases.insert(base);
                    }
                }
            }
        }
    }

    if let Some(http_node) = yaml.get("http") {
        if let Some(addr) = http_node.as_str() {
            if let Some(base) = parse_address_to_base(addr) {
                bases.insert(base);
            }
        } else if let Some(map) = http_node.as_mapping() {
            for key in ["listen", "address", "addr"] {
                if let Some(value) = map
                    .get(&serde_yaml::Value::String(key.to_string()))
                    .and_then(|v| v.as_str())
                {
                    if let Some(base) = parse_address_to_base(value) {
                        bases.insert(base);
                    }
                }
            }
        }
    }

    let mut list: Vec<String> = bases.into_iter().collect();
    list.sort();
    list
}

#[cfg(target_os = "linux")]
fn configure_gstreamer_environment() {
    const SYSTEM_SCANNER_CANDIDATES: &[&str] = &[
        "/usr/libexec/gstreamer-1.0/gst-plugin-scanner",
        "/usr/libexec/gstreamer1.0/gst-plugin-scanner",
        "/usr/lib/x86_64-linux-gnu/gstreamer-1.0/gst-plugin-scanner",
        "/usr/lib/aarch64-linux-gnu/gstreamer-1.0/gst-plugin-scanner",
    ];

    let mut plugin_paths: Vec<PathBuf> = Vec::new();
    let mut library_paths: Vec<PathBuf> = Vec::new();

    if let Ok(app_dir) = env::var("APPDIR") {
        let app_dir_path = PathBuf::from(app_dir);

        for rel in [
            "resources/gstreamer/lib/gstreamer-1.0",
            "resources/gstreamer/lib64/gstreamer-1.0",
            "resources/gstreamer/lib/x86_64-linux-gnu/gstreamer-1.0",
        ] {
            let candidate = app_dir_path.join(rel);
            if candidate.exists() {
                plugin_paths.push(candidate);
            }
        }

        for rel in ["lib", "lib64", "resources/gstreamer/lib"] {
            let candidate = app_dir_path.join(rel);
            if candidate.exists() {
                library_paths.push(candidate);
            }
        }

        let registry_path = app_dir_path.join("resources/gstreamer/cache/registry.bin");
        if let Some(parent) = registry_path.parent() {
            let _ = fs::create_dir_all(parent);
        }
        env::set_var("GST_REGISTRY_1_0", &registry_path);

        let gstreamer_bin = app_dir_path.join("resources/gstreamer/bin");
        prepend_to_path(&gstreamer_bin);

        if env::var("GST_PLUGIN_SCANNER").is_err() {
            for rel in [
                "resources/gstreamer/libexec/gstreamer-1.0/gst-plugin-scanner",
                "resources/gstreamer/bin/gst-plugin-scanner",
                "usr/libexec/gstreamer-1.0/gst-plugin-scanner",
                "usr/libexec/gstreamer1.0/gst-plugin-scanner",
            ] {
                let candidate = app_dir_path.join(rel);
                if candidate.exists() {
                    env::set_var("GST_PLUGIN_SCANNER", &candidate);
                    break;
                }
            }
        }
    }

    if env::var("GST_PLUGIN_SCANNER").is_err() {
        for candidate in SYSTEM_SCANNER_CANDIDATES {
            let path = PathBuf::from(candidate);
            if path.exists() {
                env::set_var("GST_PLUGIN_SCANNER", &path);
                break;
            }
        }
    }

    if !plugin_paths.is_empty() {
        if let Ok(joined) = env::join_paths(plugin_paths.iter().map(|p| p.as_os_str())) {
            env::set_var("GST_PLUGIN_PATH", &joined);
            if env::var("GST_PLUGIN_SYSTEM_PATH_1_0").is_err() {
                env::set_var("GST_PLUGIN_SYSTEM_PATH_1_0", &joined);
            }
        }
    }

    if !library_paths.is_empty() {
        merge_library_paths(&library_paths);
    }

    env::set_var("GST_VAAPI_DISABLE", "1");
    env::set_var("GST_VAAPI_ALL_DRIVERS", "0");
    env::set_var("GST_PLUGIN_FEATURE_RANK", "vaapi*:0");

    println!(
        "Configured GStreamer env: GST_PLUGIN_PATH={:?}, GST_PLUGIN_SCANNER={:?}, LD_LIBRARY_PATH={:?}",
        env::var("GST_PLUGIN_PATH").ok(),
        env::var("GST_PLUGIN_SCANNER").ok(),
        env::var("LD_LIBRARY_PATH").ok()
    );
}

#[cfg(target_os = "linux")]
fn prepend_to_path(dir: &Path) {
    if !dir.exists() {
        return;
    }

    let current = env::var_os("PATH").unwrap_or_default();
    let mut entries: Vec<PathBuf> = env::split_paths(&current).collect();
    if entries.iter().any(|p| p == dir) {
        return;
    }
    entries.insert(0, dir.to_path_buf());
    if let Ok(joined) = env::join_paths(entries) {
        env::set_var("PATH", joined);
    }
}

#[cfg(target_os = "linux")]
fn merge_library_paths(paths: &[PathBuf]) {
    let mut existing: Vec<PathBuf> = env::var_os("LD_LIBRARY_PATH")
        .map(|val| env::split_paths(&val).collect())
        .unwrap_or_else(Vec::new);

    for path in paths {
        if path.exists() && !existing.iter().any(|p| p == path) {
            existing.insert(0, path.clone());
        }
    }

    if let Ok(joined) = env::join_paths(existing.iter().map(|p| p.as_os_str())) {
        env::set_var("LD_LIBRARY_PATH", joined);
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct EventInfo {
    pub id: String,
    pub timestamp: String,
    pub event_type: String,
    pub camera_id: String,
    pub description: String,
}

#[derive(Clone, Debug, Serialize)]
pub struct RecordingInfo {
    pub filename: String,
    pub start_time: String,
    pub end_time: Option<String>,
    pub size: u64,
    pub duration: Option<f64>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum StreamingProvider {
    MediaMtx,
    Go2Rtc,
}

impl StreamingProvider {
    fn from_str(value: &str) -> Self {
        match value.trim().to_ascii_lowercase().as_str() {
            "go2rtc" => StreamingProvider::Go2Rtc,
            _ => StreamingProvider::MediaMtx,
        }
    }
}

impl Default for StreamingProvider {
    fn default() -> Self {
        StreamingProvider::MediaMtx
    }
}

#[derive(Debug, Clone)]
struct StreamingSettings {
    provider: StreamingProvider,
    enable_on_demand: bool,
    restart_on_config_change: bool,
    go2rtc_api_addresses: Vec<String>,
}

impl Default for StreamingSettings {
    fn default() -> Self {
        Self {
            provider: StreamingProvider::MediaMtx,
            enable_on_demand: true,
            restart_on_config_change: true,
            go2rtc_api_addresses: vec!["http://127.0.0.1:1984".to_string()],
        }
    }
}

impl StreamingSettings {
    fn from_value(value: &serde_json::Value) -> Self {
        let mut settings = StreamingSettings::default();

        if let Some(streaming) = value.get("streaming") {
            if let Some(provider) = streaming.get("provider").and_then(|v| v.as_str()) {
                settings.provider = StreamingProvider::from_str(provider);
            }

            if let Some(val) = streaming.get("enableOnDemand").and_then(|v| v.as_bool()) {
                settings.enable_on_demand = val;
            }

            if let Some(val) = streaming
                .get("restartOnConfigChange")
                .and_then(|v| v.as_bool())
            {
                settings.restart_on_config_change = val;
            }

            if let Some(go2rtc) = streaming.get("go2rtc") {
                let mut addresses: Vec<String> = Vec::new();

                if let Some(list) = go2rtc.get("apiAddresses").and_then(|v| v.as_array()) {
                    for entry in list {
                        if let Some(addr) = entry.as_str() {
                            let normalized = addr.trim().trim_end_matches('/');
                            if !normalized.is_empty() {
                                addresses.push(normalized.to_string());
                            }
                        }
                    }
                }

                if addresses.is_empty() {
                    if let Some(addr) = go2rtc.get("apiAddress").and_then(|v| v.as_str()) {
                        let normalized = addr.trim().trim_end_matches('/');
                        if !normalized.is_empty() {
                            addresses.push(normalized.to_string());
                        }
                    }
                }

                if addresses.is_empty() {
                    if let Some(addr) = go2rtc.get("baseUrl").and_then(|v| v.as_str()) {
                        let normalized = addr.trim().trim_end_matches('/');
                        if !normalized.is_empty() {
                            addresses.push(normalized.to_string());
                        }
                    }
                }

                if !addresses.is_empty() {
                    addresses.sort();
                    addresses.dedup();
                    settings.go2rtc_api_addresses = addresses;
                }
            }
        }

        settings
    }

    fn go2rtc_api_bases(&self) -> Vec<String> {
        if self.go2rtc_api_addresses.is_empty() {
            vec!["http://127.0.0.1:1984".to_string()]
        } else {
            self.go2rtc_api_addresses.clone()
        }
    }
}

async fn load_streaming_settings() -> Result<StreamingSettings, String> {
    let value = get_app_settings_internal().await?;
    Ok(StreamingSettings::from_value(&value))
}

struct MediaMtxState {
    child: Option<StdChild>,
    mediamtx_dir: PathBuf,
    config_path: PathBuf,
    exe_path: PathBuf,
}

impl MediaMtxState {
    fn new(app_handle: &AppHandle) -> Self {
        let base = app_handle
            .path()
            .app_local_data_dir()
            .unwrap_or_else(|_| std::env::current_dir().unwrap_or_else(|_| PathBuf::from(".")));

        let mediamtx_dir = base.join("mediamtx");
        let config_path = mediamtx_dir.join("mediamtx.yml");
        let exe_path = mediamtx_dir.join(MEDIAMTX_BINARY_NAME);

        if !mediamtx_dir.exists() {
            let _ = fs::create_dir_all(&mediamtx_dir);
        }

        if !exe_path.exists() {
            let mut copied = false;

            let resource_candidates: &[&str] = &[
                #[cfg(windows)]
                "mediamtx/mediamtx.exe",
                #[cfg(not(windows))]
                "mediamtx/mediamtx",
                #[cfg(windows)]
                "binaries/mediamtx.exe",
                #[cfg(windows)]
                "binaries/mediamtx-x86_64-pc-windows-msvc.exe",
                #[cfg(not(windows))]
                "binaries/mediamtx",
            ];

            if let Ok(resource_root) = app_handle.path().resource_dir() {
                for candidate in resource_candidates {
                    let path = resource_root.join(candidate);
                    if path.exists() {
                        match fs::copy(&path, &exe_path) {
                            Ok(_) => {
                                println!(
                                    "Copied MediaMTX binary from {:?} to {:?}",
                                    path, exe_path
                                );
                                copied = true;
                                break;
                            }
                            Err(err) => {
                                println!("Failed to copy MediaMTX binary from {:?}: {}", path, err);
                            }
                        }
                    }
                }
            }

            if !copied {
                if let Ok(cur) = std::env::current_dir() {
                    let bundled = cur
                        .join("src-tauri")
                        .join("mediamtx")
                        .join(MEDIAMTX_BINARY_NAME);
                    if bundled.exists() {
                        if let Err(err) = fs::copy(&bundled, &exe_path) {
                            println!("Failed to copy MediaMTX binary from {:?}: {}", bundled, err);
                        } else {
                            println!(
                                "Copied MediaMTX binary from {:?} to {:?}",
                                bundled, exe_path
                            );
                            copied = true;
                        }
                    }
                }
            }

            if !copied {
                println!(
                    "MediaMTX binary was not found in bundled resources; expected at {:?}",
                    exe_path
                );
            }
        }

        #[cfg(windows)]
        {
            let _ = StdCommand::new("taskkill")
                .args(["/IM", MEDIAMTX_BINARY_NAME, "/F"])
                .stdout(Stdio::null())
                .stderr(Stdio::null())
                .spawn();
        }

        Self {
            child: None,
            mediamtx_dir,
            config_path,
            exe_path,
        }
    }
}

struct Go2RtcState {
    child: Option<StdChild>,
    go2rtc_dir: PathBuf,
    config_path: PathBuf,
    exe_path: PathBuf,
}

impl Go2RtcState {
    fn new(app_handle: &AppHandle) -> Self {
        let base = app_handle
            .path()
            .app_local_data_dir()
            .unwrap_or_else(|_| std::env::current_dir().unwrap_or_else(|_| PathBuf::from(".")));

        let go2rtc_dir = base.join("go2rtc");
        let config_path = go2rtc_dir.join("go2rtc.yaml");
        let exe_path = go2rtc_dir.join(GO2RTC_BINARY_NAME);

        if !go2rtc_dir.exists() {
            let _ = fs::create_dir_all(&go2rtc_dir);
        }

        if !exe_path.exists() {
            let mut copied = false;

            let resource_candidates: &[&str] = &[
                #[cfg(windows)]
                "go2rtc/go2rtc.exe",
                #[cfg(not(windows))]
                "go2rtc/go2rtc",
                #[cfg(windows)]
                "binaries/go2rtc.exe",
                #[cfg(windows)]
                "binaries/go2rtc-x86_64-pc-windows-msvc.exe",
                #[cfg(not(windows))]
                "binaries/go2rtc",
            ];

            if let Ok(resource_root) = app_handle.path().resource_dir() {
                for candidate in resource_candidates {
                    let path = resource_root.join(candidate);
                    if path.exists() {
                        match fs::copy(&path, &exe_path) {
                            Ok(_) => {
                                println!(
                                    "Copied go2rtc binary from {:?} to {:?}",
                                    path, exe_path
                                );
                                copied = true;
                                break;
                            }
                            Err(err) => {
                                println!(
                                    "Failed to copy go2rtc binary from {:?}: {}",
                                    path, err
                                );
                            }
                        }
                    }
                }
            }

            if !copied {
                if let Ok(cur) = std::env::current_dir() {
                    let bundled = cur
                        .join("src-tauri")
                        .join("go2rtc")
                        .join(GO2RTC_BINARY_NAME);
                    if bundled.exists() {
                        if let Err(err) = fs::copy(&bundled, &exe_path) {
                            println!(
                                "Failed to copy go2rtc binary from {:?}: {}",
                                bundled, err
                            );
                        } else {
                            println!(
                                "Copied go2rtc binary from {:?} to {:?}",
                                bundled, exe_path
                            );
                            copied = true;
                        }
                    }
                }
            }

            if !copied {
                match Self::download_binary(&exe_path) {
                    Ok(_) => {
                        println!("Downloaded go2rtc binary to {:?}", exe_path);
                        copied = true;
                    }
                    Err(err) => {
                        println!(
                            "Failed to prepare go2rtc binary automatically: {}",
                            err
                        );
                    }
                }
            }

            if !copied {
                println!(
                    "go2rtc binary was not found in bundled resources; expected at {:?}",
                    exe_path
                );
            }
        }

        #[cfg(windows)]
        {
            let _ = StdCommand::new("taskkill")
                .args(["/IM", GO2RTC_BINARY_NAME, "/F"])
                .stdout(Stdio::null())
                .stderr(Stdio::null())
                .spawn();
        }

        Self {
            child: None,
            go2rtc_dir,
            config_path,
            exe_path,
        }
    }

    #[cfg(all(windows, target_arch = "x86_64"))]
    fn download_binary(destination: &Path) -> Result<(), String> {
        use std::fs::File;
        use std::io::Write;
        use zip::ZipArchive;

        let response = reqwest::blocking::get(GO2RTC_DOWNLOAD_URL)
            .map_err(|e| format!("Failed to download go2rtc archive: {}", e))?;

        if !response.status().is_success() {
            return Err(format!(
                "go2rtc archive download failed with HTTP status {}",
                response.status()
            ));
        }

        let bytes = response
            .bytes()
            .map_err(|e| format!("Failed to read go2rtc archive bytes: {}", e))?;

        let tmp_path = destination.with_extension("zip.part");
        {
            let mut tmp_file = File::create(&tmp_path).map_err(|e| {
                format!(
                    "Failed to create temporary go2rtc archive {:?}: {}",
                    tmp_path, e
                )
            })?;
            tmp_file
                .write_all(&bytes)
                .map_err(|e| format!("Failed to write go2rtc archive {:?}: {}", tmp_path, e))?;
        }

        let file = File::open(&tmp_path)
            .map_err(|e| format!("Failed to reopen go2rtc archive {:?}: {}", tmp_path, e))?;
        let mut archive = ZipArchive::new(file)
            .map_err(|e| format!("Failed to read go2rtc archive {:?}: {}", tmp_path, e))?;

        let mut extracted = false;
        for index in 0..archive.len() {
            let mut entry = archive
                .by_index(index)
                .map_err(|e| format!("Failed to access go2rtc archive entry {}: {}", index, e))?;
            let entry_name = entry.name().to_string();
            if entry_name.ends_with("go2rtc.exe") {
                if let Some(parent) = destination.parent() {
                    if !parent.exists() {
                        fs::create_dir_all(parent).map_err(|e| {
                            format!("Failed to create go2rtc directory {:?}: {}", parent, e)
                        })?;
                    }
                }

                let mut out_file = File::create(destination).map_err(|e| {
                    format!("Failed to create go2rtc binary {:?}: {}", destination, e)
                })?;
                std::io::copy(&mut entry, &mut out_file)
                    .map_err(|e| format!("Failed to extract go2rtc binary: {}", e))?;
                extracted = true;
                break;
            }
        }

        if let Err(err) = fs::remove_file(&tmp_path) {
            println!(
                "Warning: failed to remove temporary go2rtc archive {:?}: {}",
                tmp_path, err
            );
        }

        if !extracted {
            return Err("go2rtc archive did not contain go2rtc.exe".into());
        }

        Ok(())
    }

    #[cfg(not(all(windows, target_arch = "x86_64")))]
    fn download_binary(_destination: &Path) -> Result<(), String> {
        Err("Automatic go2rtc download is not implemented for this platform".into())
    }
}

struct RecordingProcess {
    child: StdChild,
    camera_name: String,
    stream_path: String,
    quality_label: String,
    output_file: PathBuf,
    segment_duration: u64, // Duration in seconds (default 600 = 10 minutes)
    current_segment: u32,
    recordings_dir: PathBuf,
    rtsp_url: String,
}

struct RecordingsState {
    active_recordings: HashMap<String, RecordingProcess>,
    segment_handles: HashMap<String, AsyncJoinHandle>,
}

#[derive(Clone, Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct AppResourceUsage {
    cpu_usage: f32,
    memory_bytes: u64,
    timestamp: i64,
}

impl RecordingsState {
    fn new(recordings_dir: PathBuf) -> Self {
        if !recordings_dir.exists() {
            let _ = fs::create_dir_all(&recordings_dir);
        }

        Self {
            active_recordings: HashMap::new(),
            segment_handles: HashMap::new(),
        }
    }
}

impl Default for RecordingsState {
    fn default() -> Self {
        Self::new(default_recordings_dir())
    }
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct RecordingCommandArgs {
    camera_id: Option<u32>,
    camera_name: Option<String>,
    stream_path: String,
    quality: Option<String>,
    directory: Option<String>,
    duration_seconds: Option<u64>,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct StopRecordingArgs {
    stream_path: String,
}

#[derive(Deserialize)]
#[serde(rename_all = "camelCase")]
struct ScreenshotArgs {
    image_data: String,
    directory: Option<String>,
    camera_name: Option<String>,
    stream_name: Option<String>,
    quality: Option<String>,
}

pub(crate) fn settings_root_dir() -> PathBuf {
    if let Some(dir) = dirs_next::config_dir() {
        dir.join("com.openipc.dashboard")
    } else {
        std::env::current_dir()
            .unwrap_or_else(|_| PathBuf::from("."))
            .join("settings")
    }
}

fn settings_file_path() -> PathBuf {
    settings_root_dir().join("settings.json")
}

fn load_settings_from_disk() -> Option<serde_json::Value> {
    let path = settings_file_path();
    fs::read_to_string(path)
        .ok()
        .and_then(|content| serde_json::from_str(&content).ok())
}

fn resolve_setting_path(key: &str, default: PathBuf) -> PathBuf {
    load_settings_from_disk()
        .and_then(|settings| {
            settings
                .get(key)
                .and_then(|v| v.as_str())
                .map(|s| s.to_string())
        })
        .map(|value| PathBuf::from(value.replace("\\\\", "\\")))
        .filter(|path| !path.as_os_str().is_empty())
        .unwrap_or(default)
}

fn sanitize_filename(input: &str) -> String {
    let trimmed = input.trim();
    if trimmed.is_empty() {
        return "item".to_string();
    }

    let mut sanitized = String::with_capacity(trimmed.len());
    for ch in trimmed.chars() {
        match ch {
            '<' | '>' | ':' | '"' | '/' | '\\' | '|' | '?' | '*' => sanitized.push('_'),
            _ => sanitized.push(ch),
        }
    }
    sanitized
}

fn fallback_recordings_dir() -> PathBuf {
    let base_dir = dirs_next::video_dir()
        .or_else(|| dirs_next::data_dir())
        .or_else(|| dirs_next::home_dir())
        .unwrap_or_else(|| PathBuf::from("."));

    base_dir.join(APP_STORAGE_DIR).join(RECORDINGS_SUBDIR)
}

fn fallback_screenshots_dir() -> PathBuf {
    let base_dir = dirs_next::picture_dir()
        .or_else(|| dirs_next::data_dir())
        .or_else(|| dirs_next::home_dir())
        .unwrap_or_else(|| PathBuf::from("."));

    base_dir.join(APP_STORAGE_DIR).join(SCREENSHOTS_SUBDIR)
}

pub(crate) fn default_recordings_dir() -> PathBuf {
    resolve_setting_path("recordingsPath", fallback_recordings_dir())
}

fn default_screenshots_dir() -> PathBuf {
    resolve_setting_path("screenshotsPath", fallback_screenshots_dir())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    #[cfg(target_os = "linux")]
    configure_gstreamer_environment();

    tauri::Builder::default()
        .setup(|app| {
            let app_handle = app.handle();

            let mediamtx_state = Arc::new(StdMutex::new(MediaMtxState::new(&app_handle)));
            app.manage(mediamtx_state);

            let go2rtc_state = Arc::new(StdMutex::new(Go2RtcState::new(&app_handle)));
            app.manage(go2rtc_state);

            let recordings_state = Arc::new(StdMutex::new(RecordingsState::new(
                default_recordings_dir(),
            )));
            app.manage(recordings_state);

            let auth_state = Arc::new(auth::AuthState::new(&app_handle));
            auth_state.initialize();
            app.manage(auth_state);

            let system_state = Arc::new(StdMutex::new(System::new_all()));
            app.manage(system_state);

            Ok(())
        })
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![
            ffmpeg::start_stream,
            ffmpeg::stop_stream,
            ffmpeg::play_recording,
            mediamtx_start,
            mediamtx_stop,
            add_camera_to_mediamtx,
            mediamtx_add_camera,
            get_mediamtx_config,
            check_mediamtx_path_ready,
            list_mediamtx_paths,
            add_camera_streams,
            check_stream_status,
            check_camera_http,
            whep_play,
            get_whep_endpoints,
            ensure_path_ready,
            camera_store::save_cameras,
            camera_store::load_cameras,
            camera_store::save_groups,
            camera_store::load_groups,
            camera_store::encrypt_password,
            camera_store::decrypt_password,
            camera_store::remove_camera,
            discovery::discover_cameras,
            onvif::get_rtsp_url,
            play_direct_rtsp,
            save_config_file,
            check_mediamtx_status,
            check_rtsp_stream,
            start_recording,
            stop_recording,
            get_recordings_for_date,
            get_events_for_date,
            prepare_archive_for_playback,
            read_video_file,
            get_archive_file_url,
            get_video_info,
            export_archive_clip,
            get_stream_stats,
            get_app_settings,
            save_app_settings,
            save_screenshot,
            http_server::start_http_server,
            http_server::check_http_server,
            get_app_resource_usage,
            auth::login,
            auth::auto_login,
            auth::logout,
            auth::get_users,
            auth::add_user,
            auth::update_user_password,
            auth::update_user_role,
            auth::update_user_permissions,
            auth::delete_user
        ])
        .on_window_event(|window, event| {
            if let tauri::WindowEvent::CloseRequested { .. } = event {
                let app_handle = window.app_handle();
                let mediamtx_state = app_handle.state::<Arc<StdMutex<MediaMtxState>>>();
                let mediamtx_arc = mediamtx_state.inner().clone();
                drop(mediamtx_state);

                let mut mediamtx_child = match mediamtx_arc.lock() {
                    Ok(mut guard) => guard.child.take(),
                    Err(err) => {
                        println!(
                            "[shutdown] Failed to lock MediaMTX state for termination: {}",
                            err
                        );
                        None
                    }
                };

                if let Some(mut child) = mediamtx_child.take() {
                    let _ = child.kill();
                    let _ = child.wait();
                }

                let go2rtc_state = app_handle.state::<Arc<StdMutex<Go2RtcState>>>();
                let go2rtc_arc = go2rtc_state.inner().clone();
                drop(go2rtc_state);

                let mut go2rtc_child = match go2rtc_arc.lock() {
                    Ok(mut guard) => guard.child.take(),
                    Err(err) => {
                        println!(
                            "[shutdown] Failed to lock go2rtc state for termination: {}",
                            err
                        );
                        None
                    }
                };

                if let Some(mut child) = go2rtc_child.take() {
                    let _ = child.kill();
                    let _ = child.wait();
                }
            }
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

fn ensure_mediamtx_files(state: &mut MediaMtxState) -> Result<(), String> {
    if !state.mediamtx_dir.exists() {
        fs::create_dir_all(&state.mediamtx_dir)
            .map_err(|e| format!("Failed to create mediamtx directory: {}", e))?;
    }

    if !state.config_path.exists() {
        let default_cfg = r#"logLevel: info
logDestinations: [stdout]
rtspAddress: :8554
hlsAddress: :8888
webrtcAddress: :8889
api: true
apiAddress: :9997

paths: {}
"#;

        fs::write(&state.config_path, default_cfg)
            .map_err(|e| format!("Failed to write default mediamtx config: {}", e))?;
    }

    Ok(())
}

fn load_mediamtx_config(state: &MediaMtxState) -> Result<serde_yaml::Value, String> {
    let content = fs::read_to_string(&state.config_path)
        .map_err(|e| format!("Failed to read mediamtx config: {}", e))?;
    serde_yaml::from_str(&content).map_err(|e| format!("Failed to parse mediamtx config: {}", e))
}

fn save_mediamtx_config(state: &MediaMtxState, value: &serde_yaml::Value) -> Result<(), String> {
    let content = serde_yaml::to_string(value)
        .map_err(|e| format!("Failed to serialize mediamtx config: {}", e))?;
    fs::write(&state.config_path, content)
        .map_err(|e| format!("Failed to write mediamtx config: {}", e))
}

#[tauri::command]
fn get_app_resource_usage(
    system_state: State<'_, Arc<StdMutex<System>>>,
) -> Result<AppResourceUsage, String> {
    let mut system = system_state
        .lock()
        .map_err(|_| "Failed to lock system information state".to_string())?;

    system.refresh_cpu_usage();
    system.refresh_processes();

    let pid = std::process::id();
    let process = system
        .process(sysinfo::Pid::from_u32(pid))
        .ok_or_else(|| "Process information unavailable".to_string())?;

    let cpu_usage = process.cpu_usage();
    let memory_bytes = process.memory();

    Ok(AppResourceUsage {
        cpu_usage,
        memory_bytes,
        timestamp: Utc::now().timestamp_millis(),
    })
}

fn sanitize_stream_key(name: &str) -> String {
    sanitize_filename(name).to_lowercase()
}

fn ensure_paths_mapping<'a>(root: &'a mut serde_yaml::Value) -> &'a mut serde_yaml::Mapping {
    if !root.is_mapping() {
        *root = serde_yaml::Value::Mapping(serde_yaml::Mapping::new());
    }
    root.as_mapping_mut().unwrap()
}

fn ensure_stream_path(map: &mut serde_yaml::Value, stream_name: &str, url: &str) -> bool {
    use serde_yaml::Value;

    let root_map = ensure_paths_mapping(map);
    let paths_entry = root_map
        .entry(Value::String("paths".into()))
        .or_insert(Value::Mapping(serde_yaml::Mapping::new()));

    let paths_map = ensure_paths_mapping(paths_entry);
    let key = Value::String(stream_name.to_string());

    let desired_value = {
        let mut stream_map = serde_yaml::Mapping::new();
        stream_map.insert(
            Value::String("source".into()),
            Value::String(url.to_string()),
        );
        stream_map.insert(Value::String("sourceOnDemand".into()), Value::Bool(true));
        stream_map.insert(
            Value::String("rtspTransport".into()),
            Value::String("tcp".into()),
        );
        stream_map.insert(
            Value::String("disablePublisherOverride".into()),
            Value::Bool(false),
        );
        Value::Mapping(stream_map)
    };

    let needs_update = match paths_map.get(&key) {
        Some(existing) if existing == &desired_value => false,
        _ => true,
    };

    if needs_update {
        paths_map.insert(key, desired_value);
    }

    needs_update
}

fn ensure_go2rtc_files(state: &mut Go2RtcState) -> Result<(), String> {
    if !state.go2rtc_dir.exists() {
        fs::create_dir_all(&state.go2rtc_dir)
            .map_err(|e| format!("Failed to create go2rtc directory: {}", e))?;
    }

    if !state.config_path.exists() {
        let default_cfg = r#"api:
  listen: ":1984"
rtsp:
  listen: ":8554"
webrtc:
  listen: ":8555"
streams: {}
"#;

        fs::write(&state.config_path, default_cfg)
            .map_err(|e| format!("Failed to write default go2rtc config: {}", e))?;
    }

    Ok(())
}

fn load_go2rtc_config(state: &Go2RtcState) -> Result<serde_yaml::Value, String> {
    let content = fs::read_to_string(&state.config_path)
        .map_err(|e| format!("Failed to read go2rtc config: {}", e))?;
    serde_yaml::from_str(&content).map_err(|e| format!("Failed to parse go2rtc config: {}", e))
}

fn save_go2rtc_config(state: &Go2RtcState, value: &serde_yaml::Value) -> Result<(), String> {
    let content = serde_yaml::to_string(value)
        .map_err(|e| format!("Failed to serialize go2rtc config: {}", e))?;
    fs::write(&state.config_path, content)
        .map_err(|e| format!("Failed to write go2rtc config: {}", e))
}

fn ensure_go2rtc_stream(map: &mut serde_yaml::Value, stream_name: &str, url: &str) -> bool {
    use serde_yaml::Value;

    if !map.is_mapping() {
        *map = Value::Mapping(serde_yaml::Mapping::new());
    }

    let root = map.as_mapping_mut().unwrap();
    let streams_entry = root
        .entry(Value::String("streams".into()))
        .or_insert(Value::Mapping(serde_yaml::Mapping::new()));

    if !streams_entry.is_mapping() {
        *streams_entry = Value::Mapping(serde_yaml::Mapping::new());
    }

    let streams_map = streams_entry.as_mapping_mut().unwrap();
    let key = Value::String(stream_name.to_string());
    let desired = Value::Sequence(vec![Value::String(url.to_string())]);

    let needs_update = match streams_map.get(&key) {
        Some(existing) if existing == &desired => false,
        _ => true,
    };

    if needs_update {
        streams_map.insert(key, desired);
    }

    needs_update
}

fn spawn_go2rtc_process(state: &mut Go2RtcState) -> Result<(), String> {
    ensure_go2rtc_files(state)?;

    let mut cmd = StdCommand::new(&state.exe_path);
    cmd.arg("-c")
        .arg(&state.config_path)
        .current_dir(&state.go2rtc_dir)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped());

    #[cfg(windows)]
    {
        cmd.creation_flags(0x08000000);
    }

    let mut child = cmd
        .spawn()
        .map_err(|e| format!("Failed to start go2rtc: {}", e))?;

    if let Some(stdout) = child.stdout.take() {
        std::thread::spawn(move || {
            use std::io::{BufRead, BufReader};
            let reader = BufReader::new(stdout);
            for line in reader.lines().flatten() {
                println!("[go2rtc][out] {}", line);
            }
        });
    }

    if let Some(stderr) = child.stderr.take() {
        std::thread::spawn(move || {
            use std::io::{BufRead, BufReader};
            let reader = BufReader::new(stderr);
            for line in reader.lines().flatten() {
                eprintln!("[go2rtc][err] {}", line);
            }
        });
    }

    state.child = Some(child);
    Ok(())
}

fn restart_go2rtc(state: &mut Go2RtcState) -> Result<(), String> {
    if let Some(mut child) = state.child.take() {
        let _ = child.kill();
        let _ = child.wait();
    }
    spawn_go2rtc_process(state)
}

fn restart_go2rtc_if_running(state: &mut Go2RtcState) -> Result<(), String> {
    if state.child.is_some() {
        restart_go2rtc(state)?;
    }
    Ok(())
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct StreamPathStatus {
    name: String,
    ready: bool,
    reader_count: usize,
    active_protocols: Vec<String>,
    publisher_kind: Option<String>,
    source_url: Option<String>,
    on_demand: bool,
}

fn spawn_mediamtx_process(state: &mut MediaMtxState) -> Result<(), String> {
    ensure_mediamtx_files(state)?;

    let mut cmd = StdCommand::new(&state.exe_path);
    cmd.arg(&state.config_path)
        .current_dir(&state.mediamtx_dir)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped());

    #[cfg(windows)]
    {
        cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW
    }

    let mut child = cmd
        .spawn()
        .map_err(|e| format!("Failed to start mediamtx: {}", e))?;

    if let Some(stdout) = child.stdout.take() {
        std::thread::spawn(move || {
            use std::io::{BufRead, BufReader};
            let reader = BufReader::new(stdout);
            for line in reader.lines().flatten() {
                println!("[mediamtx][out] {}", line);
            }
        });
    }

    if let Some(stderr) = child.stderr.take() {
        std::thread::spawn(move || {
            use std::io::{BufRead, BufReader};
            let reader = BufReader::new(stderr);
            for line in reader.lines().flatten() {
                eprintln!("[mediamtx][err] {}", line);
            }
        });
    }

    state.child = Some(child);
    Ok(())
}

fn restart_mediamtx(state: &mut MediaMtxState) -> Result<(), String> {
    if let Some(mut child) = state.child.take() {
        let _ = child.kill();
        let _ = child.wait();
    }
    spawn_mediamtx_process(state)
}

#[tauri::command]
async fn mediamtx_start(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<String, String> {
    let settings = load_streaming_settings().await?;

    match settings.provider {
        StreamingProvider::MediaMtx => {
            {
                let mut guard = mediamtx_state
                    .lock()
                    .map_err(|_| "Failed to lock MediaMTX state".to_string())?;

                if let Some(child) = guard.child.as_mut() {
                    match child.try_wait() {
                        Ok(None) => return Ok("already running".into()),
                        Ok(Some(_)) | Err(_) => {
                            guard.child = None;
                        }
                    }
                }

                spawn_mediamtx_process(&mut guard)?;
            }

            if let Ok(mut other) = go2rtc_state.lock() {
                if let Some(mut child) = other.child.take() {
                    let _ = child.kill();
                    let _ = child.wait();
                    println!("Stopped go2rtc backend to prioritize MediaMTX");
                }
            }

            Ok("started".into())
        }
        StreamingProvider::Go2Rtc => {
            {
                let mut guard = go2rtc_state
                    .lock()
                    .map_err(|_| "Failed to lock go2rtc state".to_string())?;

                if let Some(child) = guard.child.as_mut() {
                    match child.try_wait() {
                        Ok(None) => return Ok("already running".into()),
                        Ok(Some(_)) | Err(_) => {
                            guard.child = None;
                        }
                    }
                }

                spawn_go2rtc_process(&mut guard)?;
            }

            if let Ok(mut other) = mediamtx_state.lock() {
                if let Some(mut child) = other.child.take() {
                    let _ = child.kill();
                    let _ = child.wait();
                    println!("Stopped MediaMTX backend to prioritize go2rtc");
                }
            }

            Ok("started".into())
        }
    }
}

#[tauri::command]
async fn mediamtx_stop(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<String, String> {
    let mut stopped = false;

    {
        let mut guard = mediamtx_state
            .lock()
            .map_err(|_| "Failed to lock MediaMTX state".to_string())?;
        if let Some(mut child) = guard.child.take() {
            let _ = child.kill();
            let _ = child.wait();
            stopped = true;
        }
    }

    match go2rtc_state.lock() {
        Ok(mut guard) => {
            if let Some(mut child) = guard.child.take() {
                let _ = child.kill();
                let _ = child.wait();
                stopped = true;
            }
        }
        Err(err) => {
            println!("Failed to lock go2rtc state for stop: {}", err);
        }
    }

    if stopped {
        Ok("stopped".into())
    } else {
        Ok("not running".into())
    }
}

fn upsert_single_stream(
    state: &mut MediaMtxState,
    stream_name: &str,
    url: &str,
) -> Result<bool, String> {
    ensure_mediamtx_files(state)?;
    let mut config = load_mediamtx_config(state)?;
    let updated = ensure_stream_path(&mut config, stream_name, url);
    if updated {
        save_mediamtx_config(state, &config)?;
    }
    Ok(updated)
}

fn restart_if_running(state: &mut MediaMtxState) -> Result<(), String> {
    if state.child.is_some() {
        restart_mediamtx(state)?;
    }
    Ok(())
}

#[tauri::command]
async fn add_camera_to_mediamtx(
    name: String,
    url: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let settings = load_streaming_settings().await?;
    let stream_name = sanitize_stream_key(&name);

    match settings.provider {
        StreamingProvider::MediaMtx => {
            let mut guard = mediamtx_state
                .lock()
                .map_err(|_| "Failed to lock MediaMTX state".to_string())?;

            ensure_mediamtx_files(&mut guard)?;
            let updated = upsert_single_stream(&mut guard, &stream_name, &url)?;
            if updated {
                restart_if_running(&mut guard)?;
            } else {
                println!(
                    "MediaMTX stream '{}' already configured, skipping restart",
                    stream_name
                );
            }
            Ok(true)
        }
        StreamingProvider::Go2Rtc => {
            let mut guard = go2rtc_state
                .lock()
                .map_err(|_| "Failed to lock go2rtc state".to_string())?;

            ensure_go2rtc_files(&mut guard)?;
            let mut config = load_go2rtc_config(&guard)?;
            let updated = ensure_go2rtc_stream(&mut config, &stream_name, &url);
            if updated {
                save_go2rtc_config(&guard, &config)?;
                restart_go2rtc_if_running(&mut guard)?;
            } else {
                println!(
                    "go2rtc stream '{}' already configured, skipping restart",
                    stream_name
                );
            }
            Ok(true)
        }
    }
}

#[tauri::command]
async fn mediamtx_add_camera(
    name: String,
    rtsp: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    add_camera_to_mediamtx(name, rtsp, mediamtx_state, go2rtc_state).await
}

#[tauri::command]
async fn add_camera_streams(
    camera_id: u32,
    hd_url: String,
    sd_url: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let settings = load_streaming_settings().await?;

    match settings.provider {
        StreamingProvider::MediaMtx => {
            let mut guard = mediamtx_state
                .lock()
                .map_err(|_| "Failed to lock MediaMTX state".to_string())?;

            ensure_mediamtx_files(&mut guard)?;

            let hd_stream = format!("cam{}_0", camera_id);
            let sd_stream = format!("cam{}_1", camera_id);

            let mut config = load_mediamtx_config(&guard)?;
            let mut updated = false;

            if ensure_stream_path(&mut config, &hd_stream, hd_url.as_str()) {
                updated = true;
            }

            if ensure_stream_path(&mut config, &sd_stream, sd_url.as_str()) {
                updated = true;
            }

            if updated {
                save_mediamtx_config(&guard, &config)?;
                restart_if_running(&mut guard)?;
            } else {
                println!(
                    "MediaMTX streams '{}' and '{}' already configured, skipping restart",
                    hd_stream, sd_stream
                );
            }
            Ok(true)
        }
        StreamingProvider::Go2Rtc => {
            let mut guard = go2rtc_state
                .lock()
                .map_err(|_| "Failed to lock go2rtc state".to_string())?;

            ensure_go2rtc_files(&mut guard)?;

            let hd_stream = format!("cam{}_0", camera_id);
            let sd_stream = format!("cam{}_1", camera_id);

            let mut config = load_go2rtc_config(&guard)?;
            let mut updated = false;

            if ensure_go2rtc_stream(&mut config, &hd_stream, &hd_url) {
                updated = true;
            }
            if ensure_go2rtc_stream(&mut config, &sd_stream, &sd_url) {
                updated = true;
            }

            if updated {
                save_go2rtc_config(&guard, &config)?;
                restart_go2rtc_if_running(&mut guard)?;
            } else {
                println!(
                    "go2rtc streams '{}' and '{}' already configured, skipping restart",
                    hd_stream, sd_stream
                );
            }
            Ok(true)
        }
    }
}

#[tauri::command]
async fn get_mediamtx_config(
    state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<String, String> {
    let mut guard = state
        .lock()
        .map_err(|_| "Failed to lock MediaMTX state".to_string())?;
    ensure_mediamtx_files(&mut guard)?;

    fs::read_to_string(&guard.config_path)
        .map_err(|e| format!("Failed to read mediamtx config: {}", e))
}

#[tauri::command]
async fn check_mediamtx_path_ready(
    path_name: String,
    _mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let path = path_name.trim().to_string();
    if path.is_empty() {
        return Ok(false);
    }

    let settings = load_streaming_settings().await?;

    match settings.provider {
        StreamingProvider::MediaMtx => {
            let path_clone = path.clone();
            let result = spawn_blocking(move || {
                let client = reqwest::blocking::Client::builder()
                    .timeout(std::time::Duration::from_secs(3))
                    .build()
                    .map_err(|e| format!("Failed to build HTTP client: {}", e))?;

                let url = format!("http://127.0.0.1:8888/{}/index.m3u8", path_clone);
                match client.get(&url).send() {
                    Ok(response) if response.status().is_success() => Ok(true),
                    Ok(_) => Ok(false),
                    Err(e) => Err(format!("Failed to query MediaMTX: {}", e)),
                }
            })
            .await
            .map_err(|e| format!("Task join error: {}", e))?;

            result
        }
        StreamingProvider::Go2Rtc => {
            let statuses = fetch_go2rtc_paths(&go2rtc_state, &settings).await?;
            Ok(statuses
                .iter()
                .any(|status| status.name == path && status.ready))
        }
    }
}

fn parse_address_to_base(addr: &str) -> Option<String> {
    let mut addr = addr.trim().trim_matches('"');
    if let Some(stripped) = addr.strip_prefix("http://") {
        addr = stripped;
    } else if let Some(stripped) = addr.strip_prefix("https://") {
        addr = stripped;
    }
    addr = addr.trim_end_matches('/');
    if addr.is_empty() {
        return None;
    }

    let (host, port) = if addr.starts_with(':') {
        let port = addr.trim_start_matches(':').trim();
        if port.is_empty() {
            return None;
        }
        ("127.0.0.1".to_string(), port.to_string())
    } else if let Some((host_part, port_part)) = addr.rsplit_once(':') {
        let port = port_part.trim();
        if port.is_empty() {
            return None;
        }
        let host_part = host_part.trim().trim_matches('"');
        let host = match host_part {
            "" | "0.0.0.0" | "::" | "[::]" => "127.0.0.1".to_string(),
            _ => host_part.trim_matches(|c| c == '[' || c == ']').to_string(),
        };
        (host, port.to_string())
    } else {
        return None;
    };

    if port.parse::<u16>().is_err() {
        return None;
    }

    Some(format!("http://{}:{}", host, port))
}

fn load_whep_base_urls(state: &State<'_, Arc<StdMutex<MediaMtxState>>>) -> Vec<String> {
    let fallback = vec![
        "http://127.0.0.1:8889".to_string(),
        "http://127.0.0.1:9997".to_string(),
    ];

    let state_guard = match state.lock() {
        Ok(guard) => guard,
        Err(e) => {
            println!("[whep_play] Failed to lock MediaMTX state: {}", e);
            return fallback;
        }
    };
    let config_path = state_guard.config_path.clone();
    drop(state_guard);

    let config_str = match std::fs::read_to_string(&config_path) {
        Ok(content) => content,
        Err(e) => {
            println!(
                "[whep_play] Failed to read MediaMTX config {:?}: {}",
                config_path, e
            );
            return fallback;
        }
    };

    let yaml: serde_yaml::Value = match serde_yaml::from_str(&config_str) {
        Ok(doc) => doc,
        Err(e) => {
            println!("[whep_play] Failed to parse MediaMTX config YAML: {}", e);
            return fallback;
        }
    };

    let mut bases = HashSet::new();

    if let Some(api_addr) = yaml.get("apiAddress").and_then(|v| v.as_str()) {
        if let Some(base) = parse_address_to_base(api_addr) {
            bases.insert(base);
        }
    }

    if let Some(webrtc_addr) = yaml.get("webrtcAddress").and_then(|v| v.as_str()) {
        if let Some(base) = parse_address_to_base(webrtc_addr) {
            bases.insert(base);
        }
    }

    if bases.is_empty() {
        println!("[whep_play] No WHEP addresses found in config, using defaults");
        return fallback;
    }

    let mut list: Vec<String> = bases.into_iter().collect();
    list.sort();
    list
}

#[tauri::command]
async fn whep_play(
    path: String,
    offer_sdp: String,
    state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<String, String> {
    println!("[whep_play] Preparing WHEP endpoints for path '{}'", path);
    let base_urls = load_whep_base_urls(&state);
    println!("[whep_play] Base URLs: {:?}", base_urls);

    let mut endpoints: Vec<String> = base_urls
        .iter()
        .flat_map(|base| {
            let normalized = base.trim_end_matches('/');
            vec![
                format!("{}/whep/{}", normalized, path),
                format!("{}/{}/whep", normalized, path),
            ]
        })
        .collect();

    if endpoints.is_empty() {
        endpoints = vec![
            format!("http://127.0.0.1:8889/whep/{}", path),
            format!("http://127.0.0.1:8889/{}/whep", path),
            format!("http://127.0.0.1:9997/whep/{}", path),
            format!("http://127.0.0.1:9997/{}/whep", path),
        ];
    }

    let res = std::thread::spawn(move || -> Result<String, String> {
        let client = reqwest::blocking::Client::builder()
            .timeout(std::time::Duration::from_secs(5))
            .build()
            .map_err(|e| format!("client build error: {}", e))?;

        let mut errors: Vec<String> = Vec::new();

        for url in endpoints {
            println!(
                "[whep_play] Trying endpoint {} ({} bytes)",
                url,
                offer_sdp.len()
            );
            match client
                .post(&url)
                .header("Content-Type", "application/sdp")
                .body(offer_sdp.clone())
                .send()
            {
                Ok(resp) => {
                    let status = resp.status();
                    println!("[whep_play] Response status from {}: {}", url, status);
                    if status == 200 || status == 201 {
                        let answer = resp.text().map_err(|e| format!("read body error: {}", e))?;
                        println!(
                            "[whep_play] Success via {}! Answer SDP ({} bytes)",
                            url,
                            answer.len()
                        );
                        return Ok(answer);
                    } else {
                        let error_body = resp.text().unwrap_or_else(|_| "no body".to_string());
                        println!(
                            "[whep_play] Endpoint {} returned {} with body: {}",
                            url, status, error_body
                        );
                        errors.push(format!("{} -> HTTP {}: {}", url, status, error_body));
                    }
                }
                Err(e) => {
                    println!("[whep_play] Request error on {}: {}", url, e);
                    errors.push(format!("{} -> request error: {}", url, e));
                }
            }
        }

        Err(format!("all WHEP endpoints failed: {}", errors.join(" | ")))
    })
    .join()
    .map_err(|_| "thread join error".to_string())?;
    res
}

#[tauri::command]
async fn get_whep_endpoints(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<Vec<String>, String> {
    let settings = load_streaming_settings().await?;

    let bases = match settings.provider {
        StreamingProvider::MediaMtx => load_whep_base_urls(&mediamtx_state),
        StreamingProvider::Go2Rtc => load_go2rtc_api_bases(&go2rtc_state, &settings),
    };

    Ok(bases)
}

#[tauri::command]
async fn check_stream_status(url: String) -> Result<bool, String> {
    // This is a placeholder. In a real app, you might try to connect to the stream
    // or check the MediaMTX API to see if the stream is active.
    println!("Checking stream status for: {}", url);
    // For now, we'll just assume if the URL is valid, the stream is "active"
    Ok(url.starts_with("rtsp://"))
}

#[tauri::command]
async fn play_direct_rtsp(sdp: String) -> Result<String, String> {
    let url = sdp;
    println!("Processing RTSP URL for direct playback");

    // Use the new utility to fix common issues with RTSP URLs
    match rtsp_utils::fix_rtsp_url(&url) {
        Ok(fixed_url) => {
            // Log the fixed URL with password masked for security
            if let Some(auth_end) = fixed_url.find('@') {
                let _scheme_len = "rtsp://".len();
                let masked_url = format!("rtsp://***:***@{}", &fixed_url[auth_end + 1..]);
                println!("Fixed RTSP URL: {}", masked_url);
            } else {
                println!("Fixed RTSP URL: {}", fixed_url);
            }

            // Важно: для FFmpeg нам нужно предотвратить двойное кодирование символа @
            // Если URL содержит %40, который мог быть закодирован из @, мы должны декодировать его
            // для корректной передачи в FFmpeg и браузеру
            let decoded_url = if fixed_url.contains('%') {
                // Только один раз декодируем URL, чтобы избежать проблем с двойным кодированием
                match urlencoding::decode(&fixed_url) {
                    Ok(decoded) => {
                        println!("Decoded URL for FFmpeg (credentials masked)");
                        decoded.to_string()
                    }
                    Err(_) => {
                        println!("Failed to decode URL, using fixed version");
                        fixed_url
                    }
                }
            } else {
                fixed_url
            };

            println!("Returning URL for processing (credentials masked)");
            Ok(decoded_url)
        }
        Err(e) => {
            println!("Error fixing RTSP URL: {}", e);
            // Return the original URL if we can't fix it
            Ok(url)
        }
    }
}

#[tauri::command]
async fn check_camera_http(
    ip: String,
    user: String,
    pass: String,
    port: Option<u16>,
) -> Result<bool, String> {
    // Простая проверка доступности HTTP интерфейса камеры и валидности базовой авторизации
    // Считаем online при 200, offline при таймауте/ошибке DNS/сетевой, unauthorized при 401 (вернём false, но в лог выведем причину)
    let url = format!("http://{}:{}/", ip, port.unwrap_or(80));
    println!("[check_camera_http] GET {} as {}", url, user);
    let res = std::thread::spawn(move || {
        let client = match reqwest::blocking::Client::builder()
            .timeout(std::time::Duration::from_secs(3))
            .build()
        {
            Ok(c) => c,
            Err(e) => return Err(format!("client build error: {}", e)),
        };
        let req = client.get(&url).basic_auth(user, Some(pass));
        match req.send() {
            Ok(resp) => {
                let status = resp.status();
                println!("[check_camera_http] status {}", status);
                if status.is_success() {
                    Ok(true)
                } else if status.as_u16() == 401 {
                    // Доступен, но неверные учётные данные
                    Ok(false)
                } else {
                    Ok(false)
                }
            }
            Err(e) => {
                println!("[check_camera_http] error {}", e);
                Ok(false)
            }
        }
    })
    .join()
    .map_err(|_| "thread join error".to_string())?;
    res
}

#[tauri::command]
async fn ensure_path_ready(path: String, timeout_ms: Option<u64>) -> Result<bool, String> {
    use tauri::async_runtime::spawn_blocking;
    let deadline =
        std::time::Instant::now() + std::time::Duration::from_millis(timeout_ms.unwrap_or(15000));

    // First poke HLS to trigger on-demand (do it once before loop)
    let _ = spawn_blocking({
        let p = path.clone();
        move || {
            let _ = reqwest::blocking::Client::builder()
                .timeout(std::time::Duration::from_secs(3))
                .build()
                .and_then(|c| {
                    c.get(format!("http://127.0.0.1:8888/{}/index.m3u8", p))
                        .send()
                });
        }
    })
    .await;

    while std::time::Instant::now() < deadline {
        let target = path.clone();
        let ready = spawn_blocking(move || {
            // Try a quick OPTIONS to the RTSP URL if present in config to encourage source initialization
            // (best-effort, ignored on errors)
            let _ = (|| -> Result<(), ()> {
                // Read current config to get the source URL for this path
                // This is lightweight and avoids persistent state passing.
                let cfg_path = {
                    // Locate mediamtx.yml beside running binary in app data
                    let base = dirs_next::data_local_dir().ok_or(())?;
                    let p = base
                        .join("com.openipc.dashboard")
                        .join("mediamtx")
                        .join("mediamtx.yml");
                    if !p.exists() {
                        return Err(());
                    }
                    p
                };
                let content = std::fs::read_to_string(cfg_path).map_err(|_| ())?;
                let root: serde_yaml::Value = serde_yaml::from_str(&content).map_err(|_| ())?;
                let src = root
                    .get("paths")
                    .and_then(|p| p.get(&serde_yaml::Value::String(target.clone())))
                    .and_then(|v| v.get("source"))
                    .and_then(|s| s.as_str())
                    .ok_or(())?;
                // Light TCP dial to rtsp host:port (doesn't send bytes). If ok, source likely reachable.
                if let Ok(url) = reqwest::Url::parse(src) {
                    if url.scheme() == "rtsp" {
                        let host = url.host_str().ok_or(())?;
                        let port = url.port().unwrap_or(554);
                        let _ = std::net::TcpStream::connect_timeout(
                            &format!("{}:{}", host, port).parse().map_err(|_| ())?,
                            std::time::Duration::from_millis(400),
                        );
                    }
                }
                Ok(())
            })();

            // Re-poke HLS lightly
            let hls_ready = {
                if let Ok(c) = reqwest::blocking::Client::builder()
                    .timeout(std::time::Duration::from_secs(3))
                    .build()
                {
                    // Prefer GET and check body for an M3U8 header; some servers may not support HEAD properly
                    if let Ok(resp) = c
                        .get(format!("http://127.0.0.1:8888/{}/index.m3u8", &target))
                        .send()
                    {
                        if resp.status().is_success() {
                            if let Ok(body) = resp.text() {
                                body.contains("#EXTM3U")
                            } else {
                                true
                            }
                        } else {
                            false
                        }
                    } else {
                        false
                    }
                } else {
                    false
                }
            };
            if hls_ready {
                return true;
            }

            // Query detailed path state (prefer POST /v3/paths/get with JSON)
            fn is_ready_value(val: &serde_json::Value, tgt: &str) -> bool {
                // Try top-level
                if let Some(name) = val.get("name").and_then(|v| v.as_str()) {
                    let ready = val.get("ready").and_then(|v| v.as_bool()).unwrap_or(false);
                    let sready = val
                        .get("sourceReady")
                        .and_then(|v| v.as_bool())
                        .unwrap_or(false);
                    if name == tgt && (sready || ready) {
                        return true;
                    }
                }
                // Try nested under 'item'
                if let Some(item) = val.get("item") {
                    if is_ready_value(item, tgt) {
                        return true;
                    }
                }
                // Try array under 'items'
                if let Some(items) = val.get("items").and_then(|v| v.as_array()) {
                    for it in items {
                        if is_ready_value(it, tgt) {
                            return true;
                        }
                    }
                }
                false
            }
            if let Ok(client) = reqwest::blocking::Client::builder()
                .timeout(std::time::Duration::from_secs(3))
                .build()
            {
                // Try POST first (current MediaMTX API)
                let try_post = client
                    .post("http://127.0.0.1:8889/v3/paths/get")
                    .json(&serde_json::json!({"name": target}))
                    .send();
                let mut api_ok = false;
                if let Ok(r) = try_post {
                    if let Ok(text) = r.text() {
                        if let Ok(val) = serde_json::from_str::<serde_json::Value>(&text) {
                            if is_ready_value(&val, &target) {
                                return true;
                            }
                            api_ok = true; // POST reachable but did not indicate ready yet
                        }
                    }
                }
                // Also try GET regardless, to support older/newer variants
                let url = format!("http://127.0.0.1:8889/v3/paths/get?name={}", target);
                if let Ok(r2) = client.get(url).send() {
                    if let Ok(text) = r2.text() {
                        if let Ok(val) = serde_json::from_str::<serde_json::Value>(&text) {
                            if is_ready_value(&val, &target) {
                                return true;
                            }
                        }
                    }
                } else if !api_ok {
                    // If API is not reachable at all, don't block on it; rely on HLS readiness instead
                }
            }
            false
        })
        .await
        .unwrap_or(false);

        if ready {
            return Ok(true);
        }
        tokio_sleep(std::time::Duration::from_millis(300)).await;
    }
    Ok(false)
}

#[tauri::command]
async fn save_config_file(file_path: String, content: String) -> Result<(), String> {
    std::fs::write(&file_path, content).map_err(|e| format!("Failed to write file: {}", e))?;
    Ok(())
}

fn default_app_settings_json() -> serde_json::Value {
    let recordings_default = fallback_recordings_dir().to_string_lossy().to_string();
    let screenshots_default = fallback_screenshots_dir().to_string_lossy().to_string();

    serde_json::json!({
        "language": "ru",
        "recordingsPath": recordings_default,
        "screenshotsPath": screenshots_default,
        "hwAccel": "auto",
        "notifications_enabled": true,
        "qscale": 8,
        "fps": 20,
        "analytics_resize_width": 640,
        "analytics_frame_skip": 5,
        "analytics_record_duration": 30,
        "analytics_provider": "auto",
        "enabledModules": [],
        "whepEndpoints": ["http://127.0.0.1:8889", "http://127.0.0.1:9997"],
        "streaming": {
            "provider": "mediamtx",
            "enableOnDemand": true,
            "restartOnConfigChange": true,
            "go2rtc": {
                "apiAddress": "http://127.0.0.1:1984"
            }
        }
    })
}

#[tauri::command]
async fn get_app_settings() -> Result<serde_json::Value, String> {
    get_app_settings_internal().await
}

// Internal function accessible to other modules
pub async fn get_app_settings_internal() -> Result<serde_json::Value, String> {
    let mut settings = default_app_settings_json();

    if let Some(saved) = load_settings_from_disk() {
        if let (Some(saved_map), Some(settings_map)) = (saved.as_object(), settings.as_object_mut())
        {
            for (key, value) in saved_map {
                settings_map.insert(key.clone(), value.clone());
            }
        }
    }

    Ok(settings)
}

#[tauri::command]
async fn save_app_settings(
    settings: serde_json::Value,
    app_handle: tauri::AppHandle,
) -> Result<bool, String> {
    let config_dir = app_handle
        .path()
        .app_config_dir()
        .map_err(|_| "Failed to get config directory")?;

    std::fs::create_dir_all(&config_dir)
        .map_err(|e| format!("Failed to create config directory: {}", e))?;

    let config_file = config_dir.join("settings.json");

    let content = serde_json::to_string_pretty(&settings)
        .map_err(|e| format!("Failed to serialize settings: {}", e))?;

    std::fs::write(&config_file, content)
        .map_err(|e| format!("Failed to write settings file: {}", e))?;

    Ok(true)
}

#[tauri::command]
async fn save_screenshot(args: ScreenshotArgs) -> Result<String, String> {
    let saved_path = spawn_blocking(move || {
        let ScreenshotArgs {
            image_data,
            directory,
            camera_name,
            stream_name,
            quality,
        } = args;

        let payload = image_data
            .split_once(',')
            .map(|(_, data)| data.trim())
            .unwrap_or_else(|| image_data.trim());

        if payload.is_empty() {
            return Err("Screenshot image data is empty".to_string());
        }

        let bytes = base64::engine::general_purpose::STANDARD
            .decode(payload)
            .map_err(|e| format!("Failed to decode screenshot data: {}", e))?;

        let mut target_dir = directory
            .map(|path| path.replace("\\\\", "\\"))
            .map(PathBuf::from)
            .unwrap_or_else(default_screenshots_dir);

        if target_dir.as_os_str().is_empty() {
            target_dir = default_screenshots_dir();
        }

        fs::create_dir_all(&target_dir)
            .map_err(|e| format!("Failed to create screenshots directory: {}", e))?;

        let timestamp = Utc::now().format("%Y%m%d_%H%M%S");
        let camera = camera_name.as_deref().unwrap_or("camera");
        let stream = stream_name.as_deref().unwrap_or("stream");
        let quality_label = quality.as_deref().unwrap_or("shot");

        let filename = format!(
            "{}_{}_{}_{}.png",
            sanitize_filename(camera),
            sanitize_filename(stream),
            sanitize_filename(quality_label),
            timestamp
        );

        let file_path = target_dir.join(filename);
        fs::write(&file_path, bytes)
            .map_err(|e| format!("Failed to write screenshot file: {}", e))?;

        Ok(file_path.to_string_lossy().to_string())
    })
    .await
    .map_err(|e| format!("Task join error: {}", e))??;

    Ok(saved_path)
}

#[tauri::command]
async fn start_recording(
    args: RecordingCommandArgs,
    state: State<'_, Arc<StdMutex<RecordingsState>>>,
) -> Result<bool, String> {
    let RecordingCommandArgs {
        camera_id,
        camera_name,
        stream_path,
        quality,
        directory,
        duration_seconds,
    } = args;

    let stream_key = stream_path.trim().to_string();
    if stream_key.is_empty() {
        return Err("streamPath is required".to_string());
    }

    let recordings_dir = directory
        .map(|path| PathBuf::from(path.replace("\\\\", "\\")))
        .unwrap_or_else(default_recordings_dir);

    if !recordings_dir.exists() {
        fs::create_dir_all(&recordings_dir)
            .map_err(|e| format!("Failed to create recordings directory: {}", e))?;
    }

    let camera_label = camera_name
        .clone()
        .or_else(|| camera_id.map(|id| format!("Camera {}", id)))
        .unwrap_or_else(|| "Camera".to_string());

    let quality_label = quality.clone().unwrap_or_else(|| {
        if stream_key.ends_with("_1") {
            "sd"
        } else {
            "hd"
        }
        .to_string()
    });

    let segment_duration = duration_seconds.unwrap_or(600);
    let rtsp_url = format!("rtsp://127.0.0.1:8554/{}", stream_key);

    println!(
        "Starting segmented recording: camera='{}' stream='{}' quality='{}' path={}",
        camera_label,
        stream_key,
        quality_label,
        recordings_dir.display()
    );

    let first_segment_file = create_segment_filename(
        &recordings_dir,
        &camera_label,
        &stream_key,
        &quality_label,
        1,
    );
    println!("First segment file: {:?}", first_segment_file);

    let hw_preference = match get_app_settings_internal().await {
        Ok(settings) => settings
            .get("hwAccel")
            .and_then(|v| v.as_str())
            .unwrap_or("auto")
            .to_string(),
        Err(err) => {
            println!(
                "Failed to load app settings while starting recording ({}); using 'auto'",
                err
            );
            "auto".to_string()
        }
    };

    let ffmpeg_cmd = if cfg!(windows) {
        "ffmpeg.exe"
    } else {
        "ffmpeg"
    };
    let hw_decision = ffmpeg::determine_hw_accel_strategy(ffmpeg_cmd, &hw_preference);
    println!(
        "Recording hardware acceleration choice: {} (codec: {})",
        hw_decision.message, hw_decision.config.video_codec
    );

    let mut recordings_state = state
        .lock()
        .map_err(|_| "Failed to lock recordings state".to_string())?;

    if recordings_state.active_recordings.contains_key(&stream_key) {
        return Err("Recording already in progress for this stream".to_string());
    }

    let ffmpeg_args = build_segment_ffmpeg_args(
        &rtsp_url,
        segment_duration,
        &first_segment_file,
        &hw_decision.config,
    );
    println!("FFmpeg args for recording: {:?}", ffmpeg_args);

    let mut cmd = StdCommand::new(ffmpeg_cmd);
    cmd.args(&ffmpeg_args)
        .stdout(Stdio::null())
        .stderr(Stdio::null());

    #[cfg(windows)]
    cmd.creation_flags(0x08000000);

    match cmd.spawn() {
        Ok(child) => {
            let recording = RecordingProcess {
                child,
                camera_name: camera_label.clone(),
                stream_path: stream_key.clone(),
                quality_label: quality_label.clone(),
                output_file: first_segment_file,
                segment_duration,
                current_segment: 1,
                recordings_dir: recordings_dir.clone(),
                rtsp_url: rtsp_url.clone(),
            };

            recordings_state
                .active_recordings
                .insert(stream_key.clone(), recording);

            let state_clone = state.inner().clone();
            let stream_key_clone = stream_key.clone();
            let segment_handle = spawn(async move {
                manage_recording_segments(state_clone, stream_key_clone).await;
            });

            recordings_state
                .segment_handles
                .insert(stream_key, segment_handle);

            println!("Segmented recording started successfully");
            Ok(true)
        }
        Err(e) => {
            println!("Failed to start FFmpeg: {}", e);
            Err(format!("Failed to start recording: {}", e))
        }
    }
}

#[tauri::command]
async fn stop_recording(
    args: StopRecordingArgs,
    state: State<'_, Arc<StdMutex<RecordingsState>>>,
) -> Result<bool, String> {
    let stream_key = args.stream_path.trim().to_string();
    if stream_key.is_empty() {
        return Err("streamPath is required".to_string());
    }

    let mut recordings_state = state.lock().unwrap();

    let camera_label = recordings_state
        .active_recordings
        .get(&stream_key)
        .map(|rec| rec.camera_name.clone())
        .unwrap_or_else(|| "Unknown camera".to_string());

    // Stop the segment management task
    if let Some(handle) = recordings_state.segment_handles.remove(&stream_key) {
        handle.abort();
        println!(
            "Stopped segment management task for stream {} (camera: {})",
            stream_key, camera_label
        );
    }

    if let Some(mut recording) = recordings_state.active_recordings.remove(&stream_key) {
        println!(
            "Stopping segmented recording for stream {} (camera: {})",
            stream_key, camera_label
        );

        // Terminate FFmpeg process
        if let Err(e) = recording.child.kill() {
            println!("Warning: Failed to kill FFmpeg process: {}", e);
        }

        // Wait for process to finish
        let _ = recording.child.wait();

        println!(
            "Segmented recording stopped, last segment saved to: {:?}",
            recording.output_file
        );
        Ok(true)
    } else {
        Err("No active recording found for this stream".to_string())
    }
}

// Helper function to create segment filename
fn create_segment_filename(
    recordings_dir: &std::path::Path,
    camera_name: &str,
    stream_path: &str,
    quality_label: &str,
    segment_number: u32,
) -> std::path::PathBuf {
    let sanitized_camera = sanitize_filename(camera_name);
    let sanitized_stream = sanitize_filename(stream_path);
    let sanitized_quality = sanitize_filename(quality_label);
    let timestamp = chrono::Utc::now().format("%Y%m%d_%H%M%S");

    recordings_dir.join(format!(
        "{}_{}_{}_seg{:03}_{}.mp4",
        sanitized_camera,
        sanitized_stream,
        sanitized_quality.to_uppercase(),
        segment_number,
        timestamp
    ))
}

fn build_segment_ffmpeg_args(
    rtsp_url: &str,
    segment_duration: u64,
    output_path: &Path,
    hw_config: &ffmpeg::HwAccelConfig,
) -> Vec<String> {
    let mut args: Vec<String> = Vec::new();

    args.extend(hw_config.pre_input.iter().cloned());
    args.push("-i".into());
    args.push(rtsp_url.to_string());
    args.push("-c:v".into());
    args.push(hw_config.video_codec.clone());
    args.extend(hw_config.video_args.clone());

    if !hw_config.video_args.iter().any(|arg| arg == "-preset") {
        args.push("-preset".into());
        args.push("fast".into());
    }

    if hw_config.video_codec == "libx264" && !hw_config.video_args.iter().any(|arg| arg == "-crf") {
        args.push("-crf".into());
        args.push("23".into());
    }

    args.push("-c:a".into());
    args.push("aac".into());
    args.push("-movflags".into());
    args.push("+faststart".into());
    args.push("-t".into());
    args.push(segment_duration.to_string());
    args.push("-y".into());
    args.push(output_path.to_string_lossy().to_string());

    args
}

// Background task to manage recording segments
async fn manage_recording_segments(state: Arc<StdMutex<RecordingsState>>, stream_key: String) {
    loop {
        let duration_secs = {
            let recordings_state = state.lock().unwrap();
            match recordings_state.active_recordings.get(&stream_key) {
                Some(rec) => rec.segment_duration,
                None => {
                    println!(
                        "Recording for stream {} is no longer active, stopping segment management",
                        stream_key
                    );
                    break;
                }
            }
        };

        tokio_sleep(Duration::from_secs(duration_secs)).await;

        if let Err(e) = start_next_segment(&state, &stream_key).await {
            println!(
                "Failed to start next segment for stream {}: {}",
                stream_key, e
            );
            break;
        }
    }
}

// Function to start the next segment
async fn start_next_segment(
    state: &Arc<StdMutex<RecordingsState>>,
    stream_key: &str,
) -> Result<(), String> {
    let hw_preference = match get_app_settings_internal().await {
        Ok(settings) => settings
            .get("hwAccel")
            .and_then(|v| v.as_str())
            .unwrap_or("auto")
            .to_string(),
        Err(err) => {
            println!(
                "Failed to load app settings while updating segment ({}); using 'auto'",
                err
            );
            "auto".to_string()
        }
    };

    let ffmpeg_cmd = if cfg!(windows) {
        "ffmpeg.exe"
    } else {
        "ffmpeg"
    };
    let hw_decision = ffmpeg::determine_hw_accel_strategy(ffmpeg_cmd, &hw_preference);
    println!(
        "Next segment hardware acceleration choice: {} (codec: {})",
        hw_decision.message, hw_decision.config.video_codec
    );

    let mut recordings_state = state.lock().unwrap();

    if let Some(recording) = recordings_state.active_recordings.get_mut(stream_key) {
        let rtsp_url = recording.rtsp_url.clone();
        let segment_duration = recording.segment_duration;
        let camera_name = recording.camera_name.clone();
        let stream_path = recording.stream_path.clone();
        let quality_label = recording.quality_label.clone();
        let recordings_dir = recording.recordings_dir.clone();

        // Stop current FFmpeg process
        if let Err(e) = recording.child.kill() {
            println!("Warning: Failed to kill current FFmpeg process: {}", e);
        }
        let _ = recording.child.wait();

        // Increment segment number
        recording.current_segment += 1;

        // Create new segment filename
        let new_segment_file = create_segment_filename(
            &recordings_dir,
            &camera_name,
            &stream_path,
            &quality_label,
            recording.current_segment,
        );
        recording.output_file = new_segment_file.clone();

        println!(
            "Starting segment {} for stream {}: {:?}",
            recording.current_segment, stream_key, new_segment_file
        );

        // Start new FFmpeg process for next segment
        let ffmpeg_args = build_segment_ffmpeg_args(
            &rtsp_url,
            segment_duration,
            &new_segment_file,
            &hw_decision.config,
        );
        println!(
            "FFmpeg args for next recording segment {}: {:?}",
            recording.current_segment, ffmpeg_args
        );

        let mut cmd = StdCommand::new(ffmpeg_cmd);
        cmd.args(&ffmpeg_args)
            .stdout(Stdio::null())
            .stderr(Stdio::null());

        #[cfg(windows)]
        cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW

        match cmd.spawn() {
            Ok(child) => {
                recording.child = child;
                println!(
                    "Successfully started segment {} for stream {}",
                    recording.current_segment, stream_key
                );
                Ok(())
            }
            Err(e) => {
                println!(
                    "Failed to start FFmpeg for segment {}: {}",
                    recording.current_segment, e
                );
                Err(format!("Failed to start new segment: {}", e))
            }
        }
    } else {
        Err("Recording not found".to_string())
    }
}

#[tauri::command]
async fn get_recordings_for_date(
    camera_name: String,
    date: String,
) -> Result<Vec<RecordingInfo>, String> {
    let recordings_dir = default_recordings_dir();

    // Parse the date
    let parsed_date = NaiveDate::parse_from_str(&date, "%Y-%m-%d")
        .map_err(|_| "Invalid date format. Expected YYYY-MM-DD")?;

    let mut recordings = Vec::new();

    // Check if recordings directory exists
    if !recordings_dir.exists() {
        println!("Recordings directory does not exist: {:?}", recordings_dir);
        return Ok(recordings);
    }

    // Read directory entries
    let entries = fs::read_dir(&recordings_dir)
        .map_err(|e| format!("Failed to read recordings directory: {}", e))?;

    for entry in entries {
        let entry = entry.map_err(|e| format!("Failed to read directory entry: {}", e))?;
        let path = entry.path();

        if path.is_file() {
            if let Some(filename) = path.file_name().and_then(|n| n.to_str()) {
                // Check if filename matches our recording pattern: camera_YYYYMMDD_HHMMSS_timestamp.mp4
                if filename.ends_with(".mp4")
                    && filename.contains(&parsed_date.format("%Y%m%d").to_string())
                    && filename.starts_with(&camera_name.replace(" ", "_"))
                {
                    // Get file metadata
                    let metadata = fs::metadata(&path)
                        .map_err(|e| format!("Failed to get file metadata: {}", e))?;

                    // Extract timestamp from filename parts
                    let parts: Vec<&str> = filename
                        .strip_suffix(".mp4")
                        .unwrap_or(filename)
                        .split('_')
                        .collect();

                    // Try to extract time from filename
                    let (_date_part, time_part) = if parts.len() >= 3 {
                        (parts[1], *parts.get(2).unwrap_or(&"000000"))
                    } else {
                        ("20251003", "000000")
                    };

                    // Format start time
                    let start_time = if time_part.len() >= 6 {
                        format!(
                            "{}T{}:{}:{}Z",
                            parsed_date.format("%Y-%m-%d"),
                            &time_part[0..2],
                            &time_part[2..4],
                            &time_part[4..6]
                        )
                    } else {
                        format!("{}T00:00:00Z", parsed_date.format("%Y-%m-%d"))
                    };

                    recordings.push(RecordingInfo {
                        filename: filename.to_string(),
                        start_time,
                        end_time: None, // We don't track end time for now
                        size: metadata.len(),
                        duration: None, // We could use FFprobe to get duration
                    });
                }
            }
        }
    }

    // Sort recordings by filename (which includes timestamp)
    recordings.sort_by(|a, b| a.filename.cmp(&b.filename));

    println!(
        "Found {} recordings for camera {} on date {} in directory {:?}",
        recordings.len(),
        camera_name,
        date,
        recordings_dir
    );
    Ok(recordings)
}

#[tauri::command]
async fn get_events_for_date(date: String) -> Result<Vec<EventInfo>, String> {
    // For now, we'll generate some mock events based on existing recordings
    // In a real implementation, this would read from a database or event log

    let parsed_date = NaiveDate::parse_from_str(&date, "%Y-%m-%d")
        .map_err(|_| "Invalid date format. Expected YYYY-MM-DD")?;

    let mut events = Vec::new();

    // Add some sample events for demonstration
    // In real implementation, these would come from motion detection, alerts, etc.
    let base_time = format!("{}T", parsed_date.format("%Y-%m-%d"));

    events.push(EventInfo {
        id: format!("motion_{}_{}", date, 1),
        timestamp: format!("{}08:15:30Z", base_time),
        event_type: "motion".to_string(),
        camera_id: "ONVIF Camera".to_string(),
        description: "Motion detected".to_string(),
    });

    events.push(EventInfo {
        id: format!("recording_{}_{}", date, 1),
        timestamp: format!("{}16:31:58Z", base_time),
        event_type: "recording_start".to_string(),
        camera_id: "ONVIF Camera".to_string(),
        description: "Recording started".to_string(),
    });

    events.push(EventInfo {
        id: format!("recording_{}_{}", date, 2),
        timestamp: format!("{}16:34:07Z", base_time),
        event_type: "recording_stop".to_string(),
        camera_id: "ONVIF Camera".to_string(),
        description: "Recording stopped".to_string(),
    });

    println!("Generated {} events for date {}", events.len(), date);
    Ok(events)
}

#[tauri::command]
async fn prepare_archive_for_playback(
    filename: String,
    #[allow(non_snake_case)] startTime: f64,
) -> Result<serde_json::Value, String> {
    let recordings_dir = default_recordings_dir();
    let file_path = recordings_dir.join(&filename);

    // Check if file exists
    if !file_path.exists() {
        return Ok(serde_json::json!({
            "success": false,
            "error": format!("Recording file not found: {}", filename)
        }));
    }

    println!(
        "Preparing archive file {} for playback at time {}",
        filename, startTime
    );
    println!("Video file exists at: {}", file_path.display());

    // Return the local file path for Tauri's convertFileSrc
    Ok(serde_json::json!({
        "success": true,
        "file_path": file_path.to_string_lossy(),
        "start_time": startTime,
        "format": "mp4"
    }))
}

#[tauri::command]
async fn read_video_file(filename: String) -> Result<Vec<u8>, String> {
    let recordings_dir = default_recordings_dir();
    let file_path = recordings_dir.join(&filename);

    // Check if file exists
    if !file_path.exists() {
        return Err(format!("Recording file not found: {}", filename));
    }

    // Check file size (limit to reasonable size for memory)
    let metadata =
        fs::metadata(&file_path).map_err(|e| format!("Failed to get file metadata: {}", e))?;

    let file_size = metadata.len();
    if file_size > 100 * 1024 * 1024 {
        // Уменьшаем лимит до 100MB
        return Err(format!(
            "File too large for blob loading ({} MB). Use HTTP streaming instead.",
            file_size / (1024 * 1024)
        ));
    }

    println!(
        "Reading video file: {} (size: {} bytes)",
        file_path.display(),
        file_size
    );

    // Read file as binary data with error handling
    let file_data = match fs::read(&file_path) {
        Ok(data) => data,
        Err(e) => {
            return Err(format!("Failed to read video file: {}", e));
        }
    };

    println!("Successfully read {} bytes", file_data.len());
    Ok(file_data)
}

#[tauri::command]
async fn get_archive_file_url(filename: String) -> Result<String, String> {
    let recordings_dir = default_recordings_dir();
    let file_path = recordings_dir.join(&filename);

    // Check if file exists
    if !file_path.exists() {
        return Err(format!("Recording file not found: {}", filename));
    }

    println!("Getting archive file URL for: {}", file_path.display());

    // Return the absolute file path that can be used with convertFileSrc
    Ok(file_path.to_string_lossy().to_string())
}

#[tauri::command]
async fn get_video_info(file_path: String) -> Result<serde_json::Value, String> {
    println!("Getting video info for file: {}", file_path);

    let mut cmd = TokioCommand::new("ffprobe");
    cmd.args(&[
        "-v",
        "quiet",
        "-print_format",
        "json",
        "-show_format",
        "-show_streams",
        &file_path,
    ]);

    // На Windows скрываем консольное окно
    #[cfg(windows)]
    {
        cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW
    }

    match cmd.output().await {
        Ok(output) => {
            if output.status.success() {
                let result = String::from_utf8_lossy(&output.stdout);
                match serde_json::from_str::<serde_json::Value>(&result) {
                    Ok(json) => {
                        let mut info = serde_json::json!({
                            "success": true,
                            "duration": 0.0,
                            "width": 0,
                            "height": 0,
                            "codec": "unknown",
                            "bitrate": 0
                        });

                        // Extract format info
                        if let Some(format) = json.get("format") {
                            if let Some(duration) = format.get("duration").and_then(|v| v.as_str())
                            {
                                if let Ok(dur) = duration.parse::<f64>() {
                                    info["duration"] = serde_json::Value::Number(
                                        serde_json::Number::from_f64(dur)
                                            .unwrap_or(serde_json::Number::from(0)),
                                    );
                                }
                            }
                            if let Some(bitrate) = format.get("bit_rate").and_then(|v| v.as_str()) {
                                if let Ok(br) = bitrate.parse::<u64>() {
                                    info["bitrate"] = serde_json::Value::Number(
                                        serde_json::Number::from(br / 1000),
                                    ); // Convert to kbps
                                }
                            }
                        }

                        // Extract video stream info
                        if let Some(streams) = json.get("streams").and_then(|v| v.as_array()) {
                            for stream in streams {
                                if let Some(codec_type) =
                                    stream.get("codec_type").and_then(|v| v.as_str())
                                {
                                    if codec_type == "video" {
                                        if let Some(width) =
                                            stream.get("width").and_then(|v| v.as_u64())
                                        {
                                            info["width"] = serde_json::Value::Number(
                                                serde_json::Number::from(width),
                                            );
                                        }
                                        if let Some(height) =
                                            stream.get("height").and_then(|v| v.as_u64())
                                        {
                                            info["height"] = serde_json::Value::Number(
                                                serde_json::Number::from(height),
                                            );
                                        }
                                        if let Some(codec) =
                                            stream.get("codec_name").and_then(|v| v.as_str())
                                        {
                                            info["codec"] =
                                                serde_json::Value::String(codec.to_string());
                                        }
                                        break;
                                    }
                                }
                            }
                        }

                        Ok(info)
                    }
                    Err(e) => {
                        println!("Failed to parse FFprobe JSON: {}", e);
                        Ok(serde_json::json!({
                            "success": false,
                            "error": format!("Failed to parse video info: {}", e)
                        }))
                    }
                }
            } else {
                let error = String::from_utf8_lossy(&output.stderr);
                println!("FFprobe failed: {}", error);
                Ok(serde_json::json!({
                    "success": false,
                    "error": format!("FFprobe failed: {}", error)
                }))
            }
        }
        Err(e) => {
            println!("Failed to run FFprobe: {}", e);
            Ok(serde_json::json!({
                "success": false,
                "error": format!("Failed to run FFprobe: {}", e)
            }))
        }
    }
}

#[tauri::command]
async fn export_archive_clip(
    camera_name: String,
    start_time: f64,
    end_time: f64,
    date: String,
) -> Result<serde_json::Value, String> {
    let recordings_dir = default_recordings_dir();

    // Find recordings for this camera and date
    let parsed_date = NaiveDate::parse_from_str(&date, "%Y-%m-%d")
        .map_err(|_| "Invalid date format. Expected YYYY-MM-DD")?;

    // Create export filename
    let export_filename = format!(
        "{}_{}_clip_{}_{}.mp4",
        camera_name,
        parsed_date.format("%Y%m%d"),
        start_time as u32,
        end_time as u32
    );

    let export_path = recordings_dir.join("exports").join(&export_filename);

    // Create exports directory if it doesn't exist
    if let Some(parent_dir) = export_path.parent() {
        fs::create_dir_all(parent_dir)
            .map_err(|e| format!("Failed to create exports directory: {}", e))?;
    }

    // For demonstration, we'll just copy one of the existing files
    // In real implementation, you would:
    // 1. Find the recording(s) that contain this time range
    // 2. Use FFmpeg to extract the exact clip
    // 3. Handle multiple files if the clip spans recordings

    let entries = fs::read_dir(&recordings_dir)
        .map_err(|e| format!("Failed to read recordings directory: {}", e))?;

    let mut source_file = None;
    for entry in entries {
        let entry = entry.map_err(|e| format!("Failed to read directory entry: {}", e))?;
        let path = entry.path();

        if path.is_file() {
            if let Some(filename) = path.file_name().and_then(|n| n.to_str()) {
                if filename.ends_with(".mp4")
                    && filename.contains(&parsed_date.format("%Y%m%d").to_string())
                    && filename.starts_with(&camera_name.replace(" ", "_"))
                {
                    source_file = Some(path);
                    break;
                }
            }
        }
    }

    match source_file {
        Some(source_path) => {
            // Use FFmpeg to extract the clip
            let duration = end_time - start_time;
            let mut cmd = StdCommand::new("ffmpeg");
            cmd.args(&[
                "-i",
                source_path.to_str().unwrap(),
                "-ss",
                &start_time.to_string(),
                "-t",
                &duration.to_string(),
                "-c",
                "copy",
                "-avoid_negative_ts",
                "make_zero",
                export_path.to_str().unwrap(),
            ]);

            // На Windows скрываем консольное окно
            #[cfg(windows)]
            {
                cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW
            }

            let output = cmd.output();

            match output {
                Ok(result) => {
                    if result.status.success() {
                        println!("Successfully exported clip: {}", export_filename);
                        Ok(serde_json::json!({
                            "success": true,
                            "export_path": export_path.to_string_lossy(),
                            "filename": export_filename
                        }))
                    } else {
                        let error = String::from_utf8_lossy(&result.stderr);
                        Ok(serde_json::json!({
                            "success": false,
                            "error": format!("FFmpeg failed: {}", error)
                        }))
                    }
                }
                Err(e) => Ok(serde_json::json!({
                    "success": false,
                    "error": format!("Failed to run FFmpeg: {}", e)
                })),
            }
        }
        None => Ok(serde_json::json!({
            "success": false,
            "error": format!("No recordings found for camera {} on date {}", camera_name, date)
        })),
    }
}

#[tauri::command]
async fn get_stream_stats(stream_path: String) -> Result<serde_json::Value, String> {
    println!("Getting stream stats for path: {}", stream_path);

    // Получаем статистику потока из MediaMTX API
    let client = match reqwest::Client::builder()
        .timeout(std::time::Duration::from_secs(3))
        .build()
    {
        Ok(client) => client,
        Err(e) => {
            println!("Failed to create HTTP client: {}", e);
            return Err(format!("Failed to create HTTP client: {}", e));
        }
    };

    // Сначала получаем список всех путей для отладки
    match client
        .get("http://127.0.0.1:8889/v3/paths/list")
        .send()
        .await
    {
        Ok(response) => {
            if let Ok(text) = response.text().await {
                println!("Available paths in MediaMTX: {}", text);

                // Пытаемся также получить конфигурацию MediaMTX для лучшего понимания источников
                if let Ok(paths_data) = serde_json::from_str::<serde_json::Value>(&text) {
                    if let Some(items) = paths_data.get("items").and_then(|v| v.as_array()) {
                        for item in items {
                            if let Some(name) = item.get("name").and_then(|v| v.as_str()) {
                                if name == stream_path {
                                    println!(
                                        "Found matching path config for {}: {:?}",
                                        stream_path, item
                                    );

                                    // Пытаемся извлечь информацию из конфигурации пути
                                    if let Some(source) = item.get("source") {
                                        if let Some(source_id) =
                                            source.get("id").and_then(|v| v.as_str())
                                        {
                                            println!(
                                                "Source ID for {}: {}",
                                                stream_path, source_id
                                            );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Err(e) => println!("Failed to list paths: {}", e),
    }

    // Теперь запрашиваем информацию о конкретном пути
    let api_url = format!("http://127.0.0.1:8889/v3/paths/get/{}", stream_path);
    println!("Querying MediaMTX API: {}", api_url);

    match client.get(&api_url).send().await {
        Ok(response) => {
            let status = response.status();
            println!("MediaMTX API response status: {}", status);

            if status.is_success() {
                match response.text().await {
                    Ok(text) => {
                        println!("MediaMTX API response: {}", text);

                        match serde_json::from_str::<serde_json::Value>(&text) {
                            Ok(json) => {
                                // Извлекаем данные о потоке
                                let mut stats = serde_json::json!({
                                    "success": true,
                                    "codec": "H.264",
                                    "bitrate": 0,
                                    "resolution": "Unknown",
                                    "fps": 0,
                                    "path": stream_path
                                });

                                // Парсим данные из ответа MediaMTX
                                if let Some(ready) = json.get("ready").and_then(|v| v.as_bool()) {
                                    if ready {
                                        // Пытаемся получить информацию о треках
                                        if let Some(tracks) = json.get("tracks") {
                                            if let Some(tracks_array) = tracks.as_array() {
                                                // Приоритет видео кодекам - ищем их первыми
                                                let mut video_codec_found = false;

                                                for track in tracks_array {
                                                    // Парсим строку трека для извлечения кодека и разрешения
                                                    if let Some(track_str) = track.as_str() {
                                                        println!(
                                                            "Parsing track string: {}",
                                                            track_str
                                                        );

                                                        // Обновляем кодек только для видео треков
                                                        if track_str.contains("H264") {
                                                            stats["codec"] =
                                                                serde_json::Value::String(
                                                                    "H.264".to_string(),
                                                                );
                                                            video_codec_found = true;
                                                        } else if track_str.contains("H265")
                                                            || track_str.contains("HEVC")
                                                        {
                                                            stats["codec"] =
                                                                serde_json::Value::String(
                                                                    "H.265".to_string(),
                                                                );
                                                            video_codec_found = true;
                                                        } else if track_str.contains("VP8") {
                                                            stats["codec"] =
                                                                serde_json::Value::String(
                                                                    "VP8".to_string(),
                                                                );
                                                            video_codec_found = true;
                                                        } else if track_str.contains("VP9") {
                                                            stats["codec"] =
                                                                serde_json::Value::String(
                                                                    "VP9".to_string(),
                                                                );
                                                            video_codec_found = true;
                                                        } else if track_str.contains("AV1") {
                                                            stats["codec"] =
                                                                serde_json::Value::String(
                                                                    "AV1".to_string(),
                                                                );
                                                            video_codec_found = true;
                                                        }
                                                        // Игнорируем аудио кодеки (G711, AAC, MP3, etc.)
                                                    }
                                                }

                                                // Если видео кодек не найден, используем "Unknown"
                                                if !video_codec_found {
                                                    stats["codec"] = serde_json::Value::String(
                                                        "Unknown".to_string(),
                                                    );
                                                }
                                            }
                                        }

                                        // Пытаемся получить подробную информацию о медиа из дополнительного API запроса
                                        // MediaMTX может предоставить более детальную информацию через другой endpoint
                                        match client
                                            .get(&format!(
                                                "http://127.0.0.1:8889/v3/paths/get/{}",
                                                stream_path
                                            ))
                                            .send()
                                            .await
                                        {
                                            Ok(detailed_response) => {
                                                if detailed_response.status().is_success() {
                                                    if let Ok(detailed_text) =
                                                        detailed_response.text().await
                                                    {
                                                        if let Ok(detailed_json) =
                                                            serde_json::from_str::<serde_json::Value>(
                                                                &detailed_text,
                                                            )
                                                        {
                                                            // Ищем более подробную информацию о треках
                                                            if let Some(source) =
                                                                detailed_json.get("source")
                                                            {
                                                                if let Some(source_type) =
                                                                    source.get("type")
                                                                {
                                                                    println!(
                                                                        "Source type: {:?}",
                                                                        source_type
                                                                    );
                                                                }
                                                            }

                                                            // Пытаемся извлечь разрешение из расширенной информации
                                                            if let Some(tracks_detailed) =
                                                                detailed_json.get("tracks")
                                                            {
                                                                if let Some(tracks_array) =
                                                                    tracks_detailed.as_array()
                                                                {
                                                                    for track in tracks_array {
                                                                        if let Some(track_obj) =
                                                                            track.as_object()
                                                                        {
                                                                            // Ищем информацию о разрешении в объекте трека
                                                                            if let (Some(width), Some(height)) = (
                                                                                        track_obj.get("width").and_then(|v| v.as_u64()),
                                                                                        track_obj.get("height").and_then(|v| v.as_u64())
                                                                                    ) {
                                                                                        stats["resolution"] = serde_json::Value::String(format!("{}x{}", width, height));
                                                                                        println!("Found resolution from detailed track: {}x{}", width, height);
                                                                                    }

                                                                            // Ищем кодек в детальной информации
                                                                            if let Some(codec) =
                                                                                track_obj
                                                                                    .get("codec")
                                                                                    .and_then(|v| {
                                                                                        v.as_str()
                                                                                    })
                                                                            {
                                                                                stats["codec"] = serde_json::Value::String(codec.to_string());
                                                                                println!("Found codec from detailed track: {}", codec);
                                                                            }

                                                                            // Ищем FPS
                                                                            if let Some(fps) =
                                                                                track_obj
                                                                                    .get("fps")
                                                                                    .and_then(|v| {
                                                                                        v.as_f64()
                                                                                    })
                                                                            {
                                                                                stats["fps"] = serde_json::Value::Number(serde_json::Number::from_f64(fps).unwrap_or(serde_json::Number::from(0)));
                                                                                println!("Found FPS from detailed track: {}", fps);
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            Err(e) => {
                                                println!(
                                                    "Failed to get detailed track info: {}",
                                                    e
                                                );
                                            }
                                        } // Пытаемся получить битрейт из статистики
                                        if let Some(bytes_received) =
                                            json.get("bytesReceived").and_then(|v| v.as_u64())
                                        {
                                            if bytes_received > 1024 {
                                                // Минимум 1KB данных
                                                // Оценка битрейта на основе времени готовности потока
                                                if let Some(ready_time_str) =
                                                    json.get("readyTime").and_then(|v| v.as_str())
                                                {
                                                    // Парсим время запуска
                                                    if let Ok(ready_time) =
                                                        chrono::DateTime::parse_from_rfc3339(
                                                            ready_time_str,
                                                        )
                                                    {
                                                        let now = chrono::Utc::now();
                                                        let duration_secs = (now.timestamp()
                                                            - ready_time.timestamp())
                                                        .max(1)
                                                            as u64;

                                                        // Расчет реального битрейта в kbps
                                                        let bitrate_kbps = (bytes_received * 8)
                                                            / 1024
                                                            / duration_secs;
                                                        let calculated_bitrate =
                                                            bitrate_kbps.max(100).min(50000); // Ограничение от 100kbps до 50Mbps
                                                        stats["bitrate"] =
                                                            serde_json::Value::Number(
                                                                serde_json::Number::from(
                                                                    calculated_bitrate,
                                                                ),
                                                            );
                                                        println!("Calculated real bitrate for {}: {} kbps (from {} bytes in {} secs)", stream_path, calculated_bitrate, bytes_received, duration_secs);
                                                    } else {
                                                        // Если не можем парсить время, используем примерную оценку
                                                        let bitrate_kbps =
                                                            (bytes_received * 8) / 1024 / 10; // Предполагаем 10 секунд
                                                        let calculated_bitrate =
                                                            bitrate_kbps.max(500).min(50000);
                                                        stats["bitrate"] =
                                                            serde_json::Value::Number(
                                                                serde_json::Number::from(
                                                                    calculated_bitrate,
                                                                ),
                                                            );
                                                        println!("Estimated bitrate for {}: {} kbps (no timestamp)", stream_path, calculated_bitrate);
                                                    }
                                                } else {
                                                    // Если нет времени готовности, используем базовую оценку
                                                    let bitrate_kbps =
                                                        (bytes_received * 8) / 1024 / 10;
                                                    let calculated_bitrate =
                                                        bitrate_kbps.max(500).min(50000);
                                                    stats["bitrate"] = serde_json::Value::Number(
                                                        serde_json::Number::from(
                                                            calculated_bitrate,
                                                        ),
                                                    );
                                                    println!(
                                                        "Default bitrate estimate for {}: {} kbps",
                                                        stream_path, calculated_bitrate
                                                    );
                                                }
                                            } else {
                                                // Если данных мало, возвращаем базовое значение
                                                let fallback_bitrate =
                                                    if stream_path.ends_with("_0") {
                                                        5310
                                                    } else {
                                                        1024
                                                    };
                                                stats["bitrate"] = serde_json::Value::Number(
                                                    serde_json::Number::from(fallback_bitrate),
                                                );
                                                println!("Using fallback bitrate for {}: {} kbps (insufficient data)", stream_path, fallback_bitrate);
                                            }
                                        } else {
                                            // Если нет поля bytesReceived, возвращаем базовое значение
                                            let fallback_bitrate = if stream_path.ends_with("_0") {
                                                5310
                                            } else {
                                                1024
                                            };
                                            stats["bitrate"] = serde_json::Value::Number(
                                                serde_json::Number::from(fallback_bitrate),
                                            );
                                            println!("Using fallback bitrate for {}: {} kbps (no bytesReceived field)", stream_path, fallback_bitrate);
                                        }

                                        // Если не удалось получить разрешение из API, пытаемся определить из RTSP URL или по имени потока
                                        if stats["resolution"] == "Unknown" {
                                            // Сначала пытаемся получить конфигурацию MediaMTX для этого пути
                                            match get_stream_resolution_from_config(&stream_path)
                                                .await
                                            {
                                                Ok(Some(resolution)) => {
                                                    stats["resolution"] =
                                                        serde_json::Value::String(resolution);
                                                    println!("Got resolution from MediaMTX config for {}: {}", stream_path, stats["resolution"]);
                                                }
                                                Ok(None) => {
                                                    // Пытаемся использовать FFprobe для анализа потока
                                                    if let Ok(Some(resolution)) =
                                                        analyze_stream_with_ffprobe(&stream_path)
                                                            .await
                                                    {
                                                        stats["resolution"] =
                                                            serde_json::Value::String(resolution);
                                                        println!("Got resolution from FFprobe for {}: {}", stream_path, stats["resolution"]);
                                                    } else {
                                                        // Попытка получить информацию о разрешении через анализ источника
                                                        if let Some(source) = json.get("source") {
                                                            if let Some(rtsp_url) = source
                                                                .get("id")
                                                                .and_then(|v| v.as_str())
                                                            {
                                                                println!("Analyzing RTSP URL for resolution hints: {}", rtsp_url);

                                                                // Попытка извлечь разрешение из URL (некоторые камеры включают это в путь)
                                                                if rtsp_url.contains("1920x1080")
                                                                    || rtsp_url.contains("1080p")
                                                                {
                                                                    stats["resolution"] =
                                                                        serde_json::Value::String(
                                                                            "1920x1080".to_string(),
                                                                        );
                                                                } else if rtsp_url
                                                                    .contains("2560x1440")
                                                                    || rtsp_url.contains("1440p")
                                                                {
                                                                    stats["resolution"] =
                                                                        serde_json::Value::String(
                                                                            "2560x1440".to_string(),
                                                                        );
                                                                } else if rtsp_url
                                                                    .contains("3840x2160")
                                                                    || rtsp_url.contains("4K")
                                                                    || rtsp_url.contains("2160p")
                                                                {
                                                                    stats["resolution"] =
                                                                        serde_json::Value::String(
                                                                            "3840x2160".to_string(),
                                                                        );
                                                                } else if rtsp_url
                                                                    .contains("720x576")
                                                                    || rtsp_url.contains("704x576")
                                                                {
                                                                    stats["resolution"] =
                                                                        serde_json::Value::String(
                                                                            "704x576".to_string(),
                                                                        );
                                                                } else if rtsp_url
                                                                    .contains("stream=0")
                                                                    || rtsp_url.contains("main")
                                                                {
                                                                    // Основной поток - обычно HD
                                                                    stats["resolution"] =
                                                                        serde_json::Value::String(
                                                                            "2560x1440".to_string(),
                                                                        );
                                                                } else if rtsp_url
                                                                    .contains("stream=1")
                                                                    || rtsp_url.contains("sub")
                                                                {
                                                                    // Вспомогательный поток - обычно SD
                                                                    stats["resolution"] =
                                                                        serde_json::Value::String(
                                                                            "704x576".to_string(),
                                                                        );
                                                                }
                                                            }
                                                        }

                                                        // Если всё ещё неизвестно, используем имя потока как подсказку
                                                        if stats["resolution"] == "Unknown" {
                                                            if stream_path.ends_with("_0") {
                                                                stats["resolution"] =
                                                                    serde_json::Value::String(
                                                                        "2560x1440".to_string(),
                                                                    );
                                                            } else {
                                                                stats["resolution"] =
                                                                    serde_json::Value::String(
                                                                        "704x576".to_string(),
                                                                    );
                                                            }
                                                            println!("Using fallback resolution for {}: {}", stream_path, stats["resolution"]);
                                                        }
                                                    }
                                                }
                                                Err(e) => {
                                                    println!(
                                                        "Error getting resolution from config: {}",
                                                        e
                                                    );
                                                    // Используем fallback логику
                                                    if stream_path.ends_with("_0") {
                                                        stats["resolution"] =
                                                            serde_json::Value::String(
                                                                "2560x1440".to_string(),
                                                            );
                                                    } else {
                                                        stats["resolution"] =
                                                            serde_json::Value::String(
                                                                "704x576".to_string(),
                                                            );
                                                    }
                                                    println!(
                                                        "Using fallback resolution for {}: {}",
                                                        stream_path, stats["resolution"]
                                                    );
                                                }
                                            }
                                        }
                                    } else {
                                        println!("Stream {} is not ready", stream_path);
                                        // Поток не готов, возвращаем базовые значения
                                        stats["success"] = serde_json::Value::Bool(false);
                                        stats["error"] = serde_json::Value::String(
                                            "Stream not ready".to_string(),
                                        );
                                    }
                                }

                                println!("Final stream stats for {}: {:?}", stream_path, stats);
                                Ok(stats)
                            }
                            Err(e) => {
                                println!("Failed to parse MediaMTX response: {}", e);
                                // Возвращаем базовые значения
                                Ok(serde_json::json!({
                                    "success": false,
                                    "error": "Failed to parse response",
                                    "codec": "H.264",
                                    "bitrate": if stream_path.ends_with("_0") { 5310 } else { 1024 },
                                    "resolution": if stream_path.ends_with("_0") { "2560x1440" } else { "704x576" },
                                    "fps": 25
                                }))
                            }
                        }
                    }
                    Err(e) => {
                        println!("Failed to read response: {}", e);
                        Err(format!("Failed to read response: {}", e))
                    }
                }
            } else {
                println!(
                    "MediaMTX API returned status: {} for path: {}",
                    status, stream_path
                );
                // Возвращаем базовые значения при ошибке API
                Ok(serde_json::json!({
                    "success": false,
                    "error": format!("API returned status: {}", status),
                    "codec": "H.264",
                    "bitrate": if stream_path.ends_with("_0") { 5310 } else { 1024 },
                    "resolution": if stream_path.ends_with("_0") { "2560x1440" } else { "704x576" },
                    "fps": 25
                }))
            }
        }
        Err(e) => {
            println!("Failed to query MediaMTX API: {}", e);
            // Возвращаем базовые значения при ошибке
            Ok(serde_json::json!({
                "success": false,
                "error": "MediaMTX API unavailable",
                "codec": "H.264",
                "bitrate": if stream_path.ends_with("_0") { 5310 } else { 1024 },
                "resolution": if stream_path.ends_with("_0") { "2560x1440" } else { "704x576" },
                "fps": 25
            }))
        }
    }
}

async fn get_stream_resolution_from_config(stream_path: &str) -> Result<Option<String>, String> {
    let key = stream_path.to_string();
    spawn_blocking(move || {
        let Some(base_dir) = dirs_next::data_local_dir() else {
            return Ok(None);
        };

        let config_path = base_dir
            .join("com.openipc.dashboard")
            .join("mediamtx")
            .join("mediamtx.yml");

        if !config_path.exists() {
            return Ok(None);
        }

        let content = fs::read_to_string(&config_path)
            .map_err(|e| format!("Failed to read mediamtx config: {}", e))?;
        let yaml: serde_yaml::Value = serde_yaml::from_str(&content)
            .map_err(|e| format!("Failed to parse mediamtx config: {}", e))?;

        if let Some(paths) = yaml.get("paths").and_then(|v| v.as_mapping()) {
            if let Some(entry) = paths.get(&serde_yaml::Value::String(key.clone())) {
                if let Some(map) = entry.as_mapping() {
                    if let (Some(width), Some(height)) = (
                        map.get(&serde_yaml::Value::String("width".into()))
                            .and_then(|v| v.as_i64()),
                        map.get(&serde_yaml::Value::String("height".into()))
                            .and_then(|v| v.as_i64()),
                    ) {
                        return Ok(Some(format!("{}x{}", width, height)));
                    }

                    if let Some(resolution) = map
                        .get(&serde_yaml::Value::String("resolution".into()))
                        .and_then(|v| v.as_str())
                    {
                        return Ok(Some(resolution.to_string()));
                    }

                    if let Some(fallback) = map
                        .get(&serde_yaml::Value::String("fallbackResolution".into()))
                        .and_then(|v| v.as_str())
                    {
                        return Ok(Some(fallback.to_string()));
                    }

                    if let Some(source) = map
                        .get(&serde_yaml::Value::String("source".into()))
                        .and_then(|v| v.as_str())
                    {
                        if let Some(res) = sniff_resolution_from_text(source) {
                            return Ok(Some(res));
                        }
                    }
                } else if let Some(source) = entry.as_str() {
                    if let Some(res) = sniff_resolution_from_text(source) {
                        return Ok(Some(res));
                    }
                }
            }
        }

        Ok(None)
    })
    .await
    .map_err(|e| format!("Task join error: {}", e))?
}

async fn analyze_stream_with_ffprobe(stream_path: &str) -> Result<Option<String>, String> {
    let rtsp_url = format!("rtsp://127.0.0.1:8554/{}", stream_path);
    let mut cmd = TokioCommand::new("ffprobe");
    cmd.args(&[
        "-v",
        "error",
        "-select_streams",
        "v:0",
        "-show_entries",
        "stream=width,height",
        "-of",
        "csv=p=0:s=x",
        &rtsp_url,
    ]);

    #[cfg(windows)]
    {
        cmd.creation_flags(0x08000000);
    }

    match cmd.output().await {
        Ok(output) => {
            if !output.status.success() {
                let stderr = String::from_utf8_lossy(&output.stderr);
                println!("ffprobe returned error for {}: {}", stream_path, stderr);
                return Ok(None);
            }

            let stdout = String::from_utf8_lossy(&output.stdout);
            let resolution = stdout
                .lines()
                .map(str::trim)
                .find(|line| !line.is_empty() && line.contains('x'))
                .map(|line| line.to_string());

            Ok(resolution)
        }
        Err(e) => Err(format!("Failed to execute ffprobe: {}", e)),
    }
}

fn sniff_resolution_from_text(input: &str) -> Option<String> {
    if let Ok(re) = Regex::new(r"(?i)(\d{3,4})x(\d{3,4})") {
        if let Some(caps) = re.captures(input) {
            if let (Some(width), Some(height)) = (caps.get(1), caps.get(2)) {
                return Some(format!("{}x{}", width.as_str(), height.as_str()));
            }
        }
    }

    let lower = input.to_lowercase();
    if lower.contains("3840x2160") || lower.contains("4k") || lower.contains("2160p") {
        return Some("3840x2160".to_string());
    }
    if lower.contains("2560x1440") || lower.contains("1440p") || lower.contains("2k") {
        return Some("2560x1440".to_string());
    }
    if lower.contains("1920x1080") || lower.contains("1080p") {
        return Some("1920x1080".to_string());
    }
    if lower.contains("1280x720") || lower.contains("720p") {
        return Some("1280x720".to_string());
    }
    if lower.contains("704x576") || lower.contains("576p") || lower.contains("sd") {
        return Some("704x576".to_string());
    }
    if lower.contains("640x480") || lower.contains("480p") {
        return Some("640x480".to_string());
    }

    None
}

#[tauri::command]
async fn check_mediamtx_status() -> Result<serde_json::Value, String> {
    // Проверяем, доступен ли MediaMTX API
    let client = reqwest::Client::new();
    match client
        .get("http://127.0.0.1:8889/v3/paths/list")
        .timeout(std::time::Duration::from_secs(5))
        .send()
        .await
    {
        Ok(response) => {
            if response.status().is_success() {
                match response.json::<serde_json::Value>().await {
                    Ok(json) => {
                        println!("MediaMTX API response: {:?}", json);
                        Ok(json)
                    }
                    Err(e) => {
                        println!("Failed to parse MediaMTX API response: {}", e);
                        Ok(serde_json::json!({
                            "success": false,
                            "error": format!("Parse error: {}", e)
                        }))
                    }
                }
            } else {
                println!("MediaMTX API returned status: {}", response.status());
                Ok(serde_json::json!({
                    "success": false,
                    "error": format!("HTTP {}", response.status())
                }))
            }
        }
        Err(e) => {
            println!("Failed to connect to MediaMTX API: {}", e);
            Ok(serde_json::json!({
                "success": false,
                "error": format!("Connection error: {}", e)
            }))
        }
    }
}

#[tauri::command]
async fn check_rtsp_stream(url: String) -> Result<bool, String> {
    println!("Checking RTSP stream availability: {}", url);

    // First check if ffmpeg is available
    match tokio::process::Command::new("ffmpeg")
        .args(&["-version"])
        .stdout(std::process::Stdio::null())
        .stderr(std::process::Stdio::null())
        .output()
        .await
    {
        Ok(_) => {
            println!("FFmpeg is available, proceeding with RTSP check");
        }
        Err(e) => {
            println!(
                "FFmpeg is not available: {}. Returning true to allow camera setup.",
                e
            );
            return Ok(true); // Return true to allow camera setup to proceed
        }
    }

    // Try to connect to RTSP stream using ffmpeg with timeout
    match tokio::time::timeout(
        std::time::Duration::from_secs(10),
        tokio::process::Command::new("ffmpeg")
            .args(&["-rtsp_transport", "tcp", "-i", &url, "-f", "null", "-"])
            .stdout(std::process::Stdio::null())
            .stderr(std::process::Stdio::piped())
            .output(),
    )
    .await
    {
        Ok(result) => match result {
            Ok(output) => {
                let stderr = String::from_utf8_lossy(&output.stderr);
                if output.status.success() {
                    println!("RTSP stream is available: {}", url);
                    Ok(true)
                } else {
                    println!("RTSP stream check failed: {}", stderr);
                    // Don't fail completely - let the user try anyway
                    Ok(false)
                }
            }
            Err(e) => {
                println!("Failed to run ffmpeg for RTSP check: {}", e);
                Ok(false)
            }
        },
        Err(_) => {
            println!("RTSP stream check timed out after 10 seconds");
            Ok(false)
        }
    }
}
