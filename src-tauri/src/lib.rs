mod analytics;
mod crypto;
mod auth;
mod camera_store;
mod database;
mod discovery;
mod ffmpeg;
#[cfg(feature = "hevc-export")]
mod hevc_export;
mod http_server;
mod onvif;
mod rtsp_client;
mod rtsp_utils;

use std::cmp::Ordering;
use std::collections::{BTreeSet, HashMap, HashSet, VecDeque};
use std::env;
use std::fs;
use std::io::{BufRead, BufReader, Read, Write};
use std::net::{TcpStream, ToSocketAddrs};
use std::path::{Path, PathBuf};
use std::process::{Child as StdChild, ChildStdin, Command as StdCommand, Stdio};
use std::sync::{Arc, Mutex as StdMutex};
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

use base64::prelude::BASE64_STANDARD;
use base64::Engine;
use chrono::{NaiveDate, Utc};
use rand::{distributions::Alphanumeric, Rng};
use regex::Regex;
use serde::{Deserialize, Serialize};
use ssh2::{Channel, Session};
use sysinfo::{PidExt, ProcessExt, System, SystemExt};
use tauri::{AppHandle, Manager, State};
use tokio::process::Command as TokioCommand;
use tokio::task::spawn_blocking;
use tokio::time::sleep as tokio_sleep;

use reqwest::Client;

#[cfg(unix)]
use std::os::unix::fs::PermissionsExt;
#[cfg(windows)]
use std::os::windows::process::CommandExt;

type AsyncJoinHandle = tokio::task::JoinHandle<()>;

const APP_STORAGE_DIR: &str = "vms-dashboard";
const RECORDINGS_SUBDIR: &str = "recordings";
const SCREENSHOTS_SUBDIR: &str = "screenshots";

#[cfg(windows)]
const GO2RTC_BINARY_NAME: &str = "go2rtc.exe";
#[cfg(not(windows))]
const GO2RTC_BINARY_NAME: &str = "go2rtc";

#[cfg(all(windows, target_arch = "x86_64"))]
const GO2RTC_DOWNLOAD_URL: &str =
    "https://github.com/AlexxIT/go2rtc/releases/download/v1.9.7/go2rtc_windows_amd64.zip";

const GO2RTC_DEFAULT_API: &str = "http://127.0.0.1:1984";

struct SshShellSession {
    _session: Session,
    channel: Channel,
}

impl SshShellSession {
    fn establish(
        host: &str,
        port: Option<u16>,
        username: Option<String>,
        password_plain: Option<String>,
        password_enc: Option<String>,
    ) -> Result<(Self, String), String> {
        let session = build_ssh_session(host, port, username, password_plain, password_enc)?;

        let mut channel = session
            .channel_session()
            .map_err(|err| format!("Failed to open SSH channel: {}", err))?;
        channel
            .request_pty("xterm", None, Some((80, 24, 0, 0)))
            .map_err(|err| format!("Failed to request PTY: {}", err))?;
        channel
            .shell()
            .map_err(|err| format!("Failed to start remote shell: {}", err))?;
        session.set_blocking(false);

        let mut shell = SshShellSession {
            _session: session,
            channel,
        };
        let mut welcome =
            shell.collect_output(Duration::from_secs(2), Duration::from_millis(100))?;
        if welcome.trim().is_empty() {
            let _ = shell.channel.write_all(b"\n");
            welcome = shell.collect_output(Duration::from_secs(2), Duration::from_millis(100))?;
        }

        Ok((shell, welcome))
    }

    fn collect_output(
        &mut self,
        max_duration: Duration,
        idle_threshold: Duration,
    ) -> Result<String, String> {
        let mut stdout_data: Vec<u8> = Vec::new();
        let mut stderr_data: Vec<u8> = Vec::new();
        let mut buffer = [0u8; 4096];
        let mut stderr_buffer = [0u8; 4096];

        let start = Instant::now();
        let mut last_activity = Instant::now();

        loop {
            let mut made_progress = false;

            loop {
                match self.channel.read(&mut buffer) {
                    Ok(0) => break,
                    Ok(len) => {
                        stdout_data.extend_from_slice(&buffer[..len]);
                        made_progress = true;
                        if len < buffer.len() {
                            break;
                        }
                    }
                    Err(err) if err.kind() == std::io::ErrorKind::WouldBlock => break,
                    Err(err) => {
                        return Err(format!("Failed to read from SSH shell: {}", err));
                    }
                }
            }

            {
                let mut stderr_stream = self.channel.stderr();
                loop {
                    match stderr_stream.read(&mut stderr_buffer) {
                        Ok(0) => break,
                        Ok(len) => {
                            stderr_data.extend_from_slice(&stderr_buffer[..len]);
                            made_progress = true;
                            if len < stderr_buffer.len() {
                                break;
                            }
                        }
                        Err(err) if err.kind() == std::io::ErrorKind::WouldBlock => break,
                        Err(err) => {
                            return Err(format!("Failed to read stderr from SSH shell: {}", err));
                        }
                    }
                }
            }

            if made_progress {
                last_activity = Instant::now();
            } else if last_activity.elapsed() >= idle_threshold {
                break;
            }

            if start.elapsed() >= max_duration {
                break;
            }

            std::thread::sleep(Duration::from_millis(25));
        }

        if !stderr_data.is_empty() {
            if !stdout_data.ends_with(b"\n") {
                stdout_data.push(b'\n');
            }
            stdout_data.extend_from_slice(&stderr_data);
        }

        Ok(String::from_utf8_lossy(&stdout_data).to_string())
    }

    fn send_command(&mut self, command: &str) -> Result<String, String> {
        let mut payload = command.as_bytes().to_vec();
        if !command.ends_with('\n') {
            payload.push(b'\n');
        }

        self.channel
            .write_all(&payload)
            .map_err(|err| format!("Failed to send command to SSH shell: {}", err))?;
        self.channel
            .flush()
            .map_err(|err| format!("Failed to flush SSH shell command: {}", err))?;

        self.collect_output(Duration::from_secs(5), Duration::from_millis(200))
    }

    fn close(&mut self) {
        let _ = self.channel.close();
        let _ = self.channel.wait_close();
    }
}

#[derive(Default)]
struct SshShellManager {
    sessions: HashMap<String, SshShellSession>,
}

impl SshShellManager {
    fn insert(&mut self, id: String, session: SshShellSession) {
        self.sessions.insert(id, session);
    }

    fn remove(&mut self, id: &str) -> Option<SshShellSession> {
        self.sessions.remove(id)
    }

    fn get_mut(&mut self, id: &str) -> Option<&mut SshShellSession> {
        self.sessions.get_mut(id)
    }
}

fn shell_escape(value: &str) -> String {
    if value.is_empty() {
        return "''".to_string();
    }

    let mut escaped = String::with_capacity(value.len() + 2);
    escaped.push('\'');
    for ch in value.chars() {
        if ch == '\'' {
            escaped.push_str("'\\''");
        } else {
            escaped.push(ch);
        }
    }
    escaped.push('\'');
    escaped
}

fn run_remote_command(session: &Session, command: &str) -> Result<String, String> {
    let mut channel = session
        .channel_session()
        .map_err(|err| format!("Failed to create SSH channel: {}", err))?;
    channel
        .exec(command)
        .map_err(|err| format!("Failed to execute remote command '{}': {}", command, err))?;

    let mut stdout_buf: Vec<u8> = Vec::new();
    channel
        .read_to_end(&mut stdout_buf)
        .map_err(|err| format!("Failed to read command output: {}", err))?;

    let mut stderr_buf: Vec<u8> = Vec::new();
    channel
        .stderr()
        .read_to_end(&mut stderr_buf)
        .map_err(|err| format!("Failed to read command stderr: {}", err))?;

    channel
        .wait_close()
        .map_err(|err| format!("Failed to close command channel: {}", err))?;

    let exit_status = channel
        .exit_status()
        .map_err(|err| format!("Failed to read command exit status: {}", err))?;

    let stdout = String::from_utf8_lossy(&stdout_buf).to_string();
    let stderr = String::from_utf8_lossy(&stderr_buf).to_string();

    if exit_status != 0 {
        let stderr_trimmed = stderr.trim();
        if stderr_trimmed.is_empty() {
            return Err(format!(
                "Remote command '{}' failed with exit status {}",
                command, exit_status
            ));
        }
        return Err(stderr_trimmed.to_string());
    }

    Ok(stdout)
}

fn permissions_from_ls(perms: &str) -> Option<u32> {
    if perms.len() < 10 {
        return None;
    }

    let chars: Vec<char> = perms.chars().collect();
    let mut mode: u32 = 0;
    let mapping = [
        (1usize, 0o400),
        (2, 0o200),
        (3, 0o100),
        (4, 0o040),
        (5, 0o020),
        (6, 0o010),
        (7, 0o004),
        (8, 0o002),
        (9, 0o001),
    ];

    for (index, bit) in mapping {
        if let Some(ch) = chars.get(index) {
            match ch {
                'r' | 'w' => mode |= bit,
                'x' | 's' | 't' => mode |= bit,
                'S' | 'T' => {}
                _ => {}
            }
        }
    }

    if let Some(ch) = chars.get(3) {
        if *ch == 's' || *ch == 'S' {
            mode |= 0o4000;
        }
    }

    if let Some(ch) = chars.get(6) {
        if *ch == 's' || *ch == 'S' {
            mode |= 0o2000;
        }
    }

    if let Some(ch) = chars.get(9) {
        if *ch == 't' || *ch == 'T' {
            mode |= 0o1000;
        }
    }

    Some(mode)
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SshShellOpenResponse {
    session_id: String,
    output: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SshShellDataResponse {
    output: String,
}

#[derive(Clone, Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct AppResourceUsage {
    cpu_usage: f32,
    memory_bytes: u64,
    timestamp: i64,
}

#[derive(Clone, Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct EventInfo {
    id: String,
    timestamp: String,
    event_type: String,
    camera_id: String,
    description: String,
}

#[derive(Clone, Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct RecordingInfo {
    filename: String,
    start_time: String,
    end_time: Option<String>,
    size: u64,
    duration: Option<f64>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SshCommandResult {
    stdout: String,
    stderr: String,
    exit_status: i32,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct LocalFsEntry {
    name: String,
    path: String,
    is_dir: bool,
    size: u64,
    modified: Option<i64>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct SftpEntry {
    name: String,
    is_dir: bool,
    size: u64,
    modified: Option<i64>,
    permissions: Option<u32>,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct RemoteRecentFile {
    absolute_path: String,
    relative_path: String,
    size: u64,
    modified: i64,
}

#[derive(Debug, Copy, Clone)]
enum StreamVideoCodec {
    H264,
    H265,
}

impl StreamVideoCodec {
    fn label(self) -> &'static str {
        match self {
            StreamVideoCodec::H264 => "H.264",
            StreamVideoCodec::H265 => "H.265",
        }
    }
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
enum StreamAudioCodec {
    Aac,
    Opus,
    Pcma,
    Pcmu,
    Unknown,
}

impl StreamAudioCodec {
    #[allow(dead_code)]
    fn label(self) -> &'static str {
        match self {
            StreamAudioCodec::Aac => "AAC",
            StreamAudioCodec::Opus => "Opus",
            StreamAudioCodec::Pcma => "G711 (PCMA)",
            StreamAudioCodec::Pcmu => "G711 (PCMU)",
            StreamAudioCodec::Unknown => "Unknown",
        }
    }
}

#[derive(Clone)]
enum RecordingVideoPipeline {
    Copy,
    Reencode(ffmpeg::HwAccelConfig),
}

impl RecordingVideoPipeline {
    fn description(&self) -> String {
        match self {
            RecordingVideoPipeline::Copy => "copy original video".into(),
            RecordingVideoPipeline::Reencode(cfg) => {
                format!("reencode via {}", cfg.video_codec)
            }
        }
    }
}

struct RecordingProcess {
    child: StdChild,
    stdin: Option<ChildStdin>,
    camera_name: String,
    stream_path: String,
    quality_label: String,
    output_file: PathBuf,
    segment_duration: u64,
    current_segment: u32,
    recordings_dir: PathBuf,
    rtsp_url: String,
    video_pipeline: RecordingVideoPipeline,
}

struct RecordingsState {
    active_recordings: HashMap<String, RecordingProcess>,
    segment_handles: HashMap<String, AsyncJoinHandle>,
}

impl RecordingsState {
    fn new(recordings_dir: PathBuf) -> Self {
        if !recordings_dir.exists() {
            if let Err(err) = fs::create_dir_all(&recordings_dir) {
                println!(
                    "Failed to create recordings directory {:?}: {}",
                    recordings_dir, err
                );
            }
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

struct Go2RtcState {
    child: Option<StdChild>,
    go2rtc_dir: PathBuf,
    config_path: PathBuf,
    exe_path: PathBuf,
    ffmpeg_path: PathBuf,
    ffmpeg_silent_path: PathBuf,
}

impl Go2RtcState {
    fn new(app_handle: &AppHandle) -> Self {
        println!("[Go2RtcState::new] Initializing Go2RTC state...");
        
        let base_dir = app_handle.path().app_local_data_dir().unwrap_or_else(|_| {
            dirs_next::data_local_dir()
                .or_else(dirs_next::data_dir)
                .unwrap_or_else(|| env::current_dir().unwrap_or_else(|_| PathBuf::from(".")))
        });
        println!("[Go2RtcState::new] Base directory: {:?}", base_dir);

        let go2rtc_dir = base_dir.join("go2rtc");
        println!("[Go2RtcState::new] Go2RTC directory: {:?}", go2rtc_dir);
        
        if !go2rtc_dir.exists() {
            println!("[Go2RtcState::new] Creating go2rtc directory...");
            if let Err(err) = fs::create_dir_all(&go2rtc_dir) {
                println!(
                    "Failed to create go2rtc directory {:?}: {}",
                    go2rtc_dir, err
                );
            }
        }

        let config_path = go2rtc_dir.join("go2rtc.yaml");
        let exe_path = go2rtc_dir.join(GO2RTC_BINARY_NAME);
        
        println!("[Go2RtcState::new] Config path: {:?}", config_path);
        println!("[Go2RtcState::new] Exe path: {:?}", exe_path);
        println!("[Go2RtcState::new] Calling prepare_binary...");
        
        Self::prepare_binary(app_handle, &exe_path);

        let ffmpeg_path = go2rtc_dir.join("ffmpeg.exe");
        Self::prepare_ffmpeg_binary(app_handle, &ffmpeg_path, "ffmpeg.exe");

        let ffmpeg_silent_path = go2rtc_dir.join("ffmpeg-silent.exe");
        Self::prepare_ffmpeg_binary(app_handle, &ffmpeg_silent_path, "ffmpeg-silent.exe");

        println!("[Go2RtcState::new] Cleaning up any existing go2rtc processes...");
        
        /*
        #[cfg(windows)]
        {
            const CREATE_NO_WINDOW: u32 = 0x08000000;
            const DETACHED_PROCESS: u32 = 0x00000008;
            if let Ok(mut child) = StdCommand::new("taskkill")
                .args(["/IM", GO2RTC_BINARY_NAME, "/F"])
                .stdout(Stdio::null())
                .stderr(Stdio::null())
                .creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS)
                .spawn()
            {
                // Wait for taskkill to complete
                let _ = child.wait();
                // Give Windows time to release the process handle
                std::thread::sleep(std::time::Duration::from_millis(200));
                println!("[Go2RtcState::new] Killed existing go2rtc processes");
            }
        }
        */
        println!("[Go2RtcState::new] Skipped cleanup of existing go2rtc processes (manual mode)");

        println!("[Go2RtcState::new] Go2RTC state initialized");
        
        Self {
            child: None,
            go2rtc_dir,
            config_path,
            exe_path,
            ffmpeg_path,
            ffmpeg_silent_path,
        }
    }

    fn prepare_binary(app_handle: &AppHandle, destination: &Path) {
        println!("[prepare_binary] Destination: {:?}", destination);

        if destination.exists() {
            println!("[prepare_binary] Binary already exists at destination");
            
            // Verify the binary is executable and has content
            if let Ok(metadata) = fs::metadata(destination) {
                println!("[prepare_binary] Binary size: {} bytes", metadata.len());
                if metadata.len() < 1000 {
                    println!("[prepare_binary] WARNING: Binary seems too small, may be corrupted");
                }
            }
            return;
        }

        let mut copied = false;

        if let Ok(resource_root) = app_handle.path().resource_dir() {
            println!("[prepare_binary] Resource root: {:?}", resource_root);
            println!(
                "[prepare_binary] Checking candidates: {:?}",
                Self::resource_candidates()
            );

            for candidate in Self::resource_candidates() {
                let source = resource_root.join(candidate);
                println!("[prepare_binary] Checking: {:?}", source);

                if source.exists() {
                    println!("[prepare_binary] Found source binary at {:?}", source);
                    match fs::copy(&source, destination) {
                        Ok(bytes) => {
                            println!(
                                "Copied go2rtc binary from {:?} to {:?} ({} bytes)",
                                source, destination, bytes
                            );
                            copied = true;
                            break;
                        }
                        Err(err) => {
                            println!("[prepare_binary] Failed to copy from {:?}: {}", source, err);
                        }
                    }
                } else {
                    println!("[prepare_binary] Source does not exist: {:?}", source);
                }
            }
        } else {
            println!("[prepare_binary] Failed to get resource_dir");
        }

        if !copied {
            println!("[prepare_binary] Not copied from resources, trying current_dir");
            if let Ok(current_dir) = env::current_dir() {
                let mut search_roots: Vec<PathBuf> = vec![current_dir.clone()];
                if let Some(parent) = current_dir.parent() {
                    search_roots.push(parent.to_path_buf());
                }

                for root in search_roots {
                    let candidates = [
                        root.join("go2rtc").join(GO2RTC_BINARY_NAME),
                        root.join("binaries").join(GO2RTC_BINARY_NAME),
                        root.join("binaries")
                            .join("windows")
                            .join(GO2RTC_BINARY_NAME),
                        root.join("src-tauri")
                            .join("go2rtc")
                            .join(GO2RTC_BINARY_NAME),
                        root.join("src-tauri")
                            .join("binaries")
                            .join(GO2RTC_BINARY_NAME),
                        root.join("src-tauri")
                            .join("binaries")
                            .join("windows")
                            .join(GO2RTC_BINARY_NAME),
                    ];

                    for bundled in candidates.iter() {
                        if !bundled.exists() {
                            continue;
                        }

                        match fs::copy(bundled, destination) {
                            Ok(_) => {
                                println!(
                                    "Copied go2rtc binary from {:?} to {:?}",
                                    bundled, destination
                                );
                                copied = true;
                                break;
                            }
                            Err(err) => {
                                println!(
                                    "Failed to copy go2rtc binary from {:?}: {}",
                                    bundled, err
                                );
                            }
                        }
                    }

                    if copied {
                        break;
                    }
                }
            }
        }

        if !copied {
            #[cfg(all(windows, target_arch = "x86_64"))]
            {
                if let Err(err) = Self::download_binary(destination) {
                    println!(
                        "Failed to download go2rtc binary to {:?}: {}",
                        destination, err
                    );
                } else {
                    copied = true;
                }
            }

            #[cfg(not(all(windows, target_arch = "x86_64")))]
            {
                println!("go2rtc binary not bundled; expected at {:?}", destination);
            }
        }

        if !copied {
            println!(
                "go2rtc binary was not prepared; manual setup may be required at {:?}",
                destination
            );
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

    #[cfg(windows)]
    fn resource_candidates() -> &'static [&'static str] {
        &[
            "go2rtc/go2rtc.exe",
            "binaries/go2rtc.exe",
            "binaries/go2rtc-x86_64-pc-windows-msvc.exe",
        ]
    }

    #[cfg(not(windows))]
    fn resource_candidates() -> &'static [&'static str] {
        &["go2rtc/go2rtc", "binaries/go2rtc"]
    }

    fn prepare_ffmpeg_binary(app_handle: &AppHandle, destination: &Path, binary_name: &str) {
        println!("[prepare_ffmpeg_binary] Destination: {:?}", destination);

        if destination.exists() {
            println!("[prepare_ffmpeg_binary] Binary already exists at destination");
            return;
        }

        let mut copied = false;

        if let Ok(resource_root) = app_handle.path().resource_dir() {
            // Construct candidates based on the requested binary name
            let candidates = [
                format!("binaries/{}", binary_name),
                binary_name.to_string(),
            ];
            
            for candidate in candidates.iter() {
                let source = resource_root.join(candidate);
                if source.exists() {
                    match fs::copy(&source, destination) {
                        Ok(_) => {
                            println!("Copied {} from {:?} to {:?}", binary_name, source, destination);
                            copied = true;
                            break;
                        }
                        Err(err) => {
                            println!("[prepare_ffmpeg_binary] Failed to copy from {:?}: {}", source, err);
                        }
                    }
                }
            }
        }

        if !copied {
            println!("[prepare_ffmpeg_binary] Not copied from resources, trying current_dir");
            if let Ok(current_dir) = env::current_dir() {
                let mut search_roots: Vec<PathBuf> = vec![current_dir.clone()];
                if let Some(parent) = current_dir.parent() {
                    search_roots.push(parent.to_path_buf());
                }

                for root in search_roots {
                    let candidates = [
                        root.join("binaries").join(binary_name),
                        root.join(binary_name),
                        root.join("src-tauri").join("binaries").join(binary_name),
                        root.join("target").join("debug").join("binaries").join(binary_name),
                        root.join("target").join("release").join("binaries").join(binary_name),
                    ];

                    for bundled in candidates.iter() {
                        if !bundled.exists() {
                            continue;
                        }
                        match fs::copy(bundled, destination) {
                            Ok(_) => {
                                println!("Copied {} from {:?} to {:?}", binary_name, bundled, destination);
                                copied = true;
                                break;
                            }
                            Err(err) => {
                                println!("Failed to copy {} from {:?}: {}", binary_name, bundled, err);
                            }
                        }
                    }
                    if copied { break; }
                }
            }
        }

        if !copied {
            println!("[prepare_ffmpeg_binary] WARNING: Failed to copy {} to {:?}", binary_name, destination);
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum StreamingProvider {
    Go2Rtc,
}

impl StreamingProvider {
    fn from_str(_value: &str) -> Self {
        // Always use Go2RTC, MediaMTX support removed
        StreamingProvider::Go2Rtc
    }

    #[allow(dead_code)]
    fn as_str(&self) -> &'static str {
        "go2rtc"
    }
}

impl Default for StreamingProvider {
    fn default() -> Self {
        StreamingProvider::Go2Rtc
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
            provider: StreamingProvider::Go2Rtc,
            enable_on_demand: true,
            restart_on_config_change: true,
            go2rtc_api_addresses: vec![GO2RTC_DEFAULT_API.to_string()],
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
                    for entry in list.iter().filter_map(|v| v.as_str()) {
                        let normalized = entry.trim().trim_end_matches('/');
                        if !normalized.is_empty() {
                            addresses.push(normalized.to_string());
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

    #[allow(dead_code)]
    fn on_demand(&self) -> bool {
        self.enable_on_demand
    }

    #[allow(dead_code)]
    fn should_restart_on_change(&self) -> bool {
        self.restart_on_config_change
    }

    fn go2rtc_api_bases(&self) -> Vec<String> {
        if self.go2rtc_api_addresses.is_empty() {
            vec![GO2RTC_DEFAULT_API.to_string()]
        } else {
            self.go2rtc_api_addresses.clone()
        }
    }
}

async fn load_streaming_settings() -> Result<StreamingSettings, String> {
    let settings = get_app_settings_internal().await?;
    Ok(StreamingSettings::from_value(&settings))
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
    rtsp_url: Option<String>,
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

#[derive(Clone)]
enum LiveStreamPipeline {
    Direct {
        source_url: String,
    },
    #[allow(dead_code)]
    Transcode {
        origin_url: String,
        command: String,
        encoder: String,
    },
}

impl LiveStreamPipeline {
    #[allow(dead_code)]
    fn description(&self) -> String {
        match self {
            LiveStreamPipeline::Direct { .. } => "direct source passthrough".into(),
            LiveStreamPipeline::Transcode { encoder, .. } => {
                format!("FFmpeg on-demand transcode ({})", encoder)
            }
        }
    }

    #[allow(dead_code)]
    fn origin_url(&self) -> &str {
        match self {
            LiveStreamPipeline::Direct { source_url } => source_url,
            LiveStreamPipeline::Transcode { origin_url, .. } => origin_url,
        }
    }
}

#[allow(dead_code)]
fn build_audio_transcode_command(ffmpeg_cmd: &str, origin_url: &str, stream_name: &str) -> String {
    let ffmpeg_exec = if ffmpeg_cmd.contains(' ') && !ffmpeg_cmd.starts_with('"') {
        format!("\"{}\"", ffmpeg_cmd)
    } else {
        ffmpeg_cmd.to_string()
    };

    let sanitized_origin = origin_url.replace('"', "\\\"");
    let output_url = format!("rtsp://127.0.0.1:8554/{}", stream_name);

    format!(
        "{ffmpeg_exec} -nostdin -loglevel warning -rtsp_transport tcp -i \"{origin}\" -map 0:v:0? -map 0:a:0? -c:v copy -c:a libopus -b:a 96k -ar 48000 -ac 2 -f rtsp -rtsp_transport tcp \"{output}\"",
        ffmpeg_exec = ffmpeg_exec,
        origin = sanitized_origin,
        output = output_url
    )
}

#[allow(dead_code)]
async fn decide_live_pipeline(
    stream_name: &str,
    origin_url: &str,
    ffmpeg_cmd: &str,
    hw_config: &ffmpeg::HwAccelConfig,
) -> LiveStreamPipeline {
    let _ = hw_config;
    let trimmed_origin = origin_url.trim().to_string();

    match fetch_rtsp_sdp(&trimmed_origin, true).await {
        Some(sdp) => {
            let audio_codec = parse_audio_codec_from_sdp(&sdp);
            match audio_codec {
                Some(StreamAudioCodec::Aac) => {
                    println!(
                    "Stream '{}' exposes AAC audio; enabling on-demand FFmpeg transcode to Opus",
                    stream_name
                );

                    let command =
                        build_audio_transcode_command(ffmpeg_cmd, &trimmed_origin, stream_name);
                    return LiveStreamPipeline::Transcode {
                        origin_url: trimmed_origin,
                        command,
                        encoder: format!("{} -> Opus", StreamAudioCodec::Aac.label()),
                    };
                }
                Some(StreamAudioCodec::Opus)
                | Some(StreamAudioCodec::Pcma)
                | Some(StreamAudioCodec::Pcmu) => {
                    let codec = audio_codec.unwrap();
                    println!(
                        "Stream '{}' audio codec '{}' is WebRTC-compatible; using direct pipeline",
                        stream_name,
                        codec.label()
                    );
                }
                Some(StreamAudioCodec::Unknown) | None => {
                    println!(
                        "Stream '{}' audio codec could not be determined; falling back to direct pipeline",
                        stream_name
                    );
                }
            }
        }
        None => {
            println!(
                "Failed to probe SDP for stream '{}'; using direct pipeline",
                stream_name
            );
        }
    }

    LiveStreamPipeline::Direct {
        source_url: trimmed_origin,
    }
}

fn camera_password(camera: &camera_store::Camera) -> Option<String> {
    if !camera.pass.trim().is_empty() {
        return Some(camera.pass.trim().to_string());
    }

    if !camera.pass_enc.trim().is_empty() {
        return camera_store::decrypt_password(&camera.pass_enc)
            .ok()
            .map(|value| value.trim().to_string())
            .filter(|value| !value.is_empty());
    }

    None
}

fn build_rtsp_base(camera: &camera_store::Camera) -> Option<String> {
    let ip = camera.ip.trim();
    if ip.is_empty() {
        return None;
    }

    let protocol = if camera.protocol.trim().is_empty() {
        "rtsp"
    } else {
        camera.protocol.trim()
    };

    let mut auth = String::new();
    if !camera.user.trim().is_empty() {
        auth.push_str(camera.user.trim());
        if let Some(password) = camera_password(camera) {
            auth.push(':');
            auth.push_str(&password);
        }
        auth.push('@');
    }

    let port = if camera.port == 0 { 554 } else { camera.port };
    Some(format!("{}://{}{}:{}", protocol, auth, ip, port))
}

fn camera_default_suffix(variant: Option<u8>) -> &'static str {
    match variant {
        Some(1) => "/stream2",
        _ => "/stream1",
    }
}

fn parse_camera_stream_hint(value: &str) -> Option<(u32, Option<u8>)> {
    let trimmed = value.trim();
    if !trimmed.starts_with("cam") {
        return None;
    }
    let rest = &trimmed[3..];
    if rest.is_empty() {
        return None;
    }

    if let Some((id_part, variant_part)) = rest.split_once('_') {
        let id: u32 = id_part.parse().ok()?;
        let variant = variant_part.parse().ok();
        Some((id, variant))
    } else {
        let id: u32 = rest.parse().ok()?;
        Some((id, None))
    }
}

pub(crate) fn settings_root_dir() -> PathBuf {
    if let Some(dir) = dirs_next::config_dir() {
        dir.join(APP_STORAGE_DIR)
    } else {
        env::current_dir()
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

#[derive(Debug, Copy, Clone)]
enum PasswordSource {
    Plain,
    Encrypted,
    Stored,
    None,
}

impl PasswordSource {
    fn as_str(&self) -> &'static str {
        match self {
            PasswordSource::Plain => "plain",
            PasswordSource::Encrypted => "encrypted",
            PasswordSource::Stored => "stored",
            PasswordSource::None => "none",
        }
    }
}

fn resolve_camera_password(
    password_plain: Option<String>,
    password_enc: Option<String>,
) -> Result<(Option<String>, PasswordSource), String> {
    if let Some(plain) = password_plain {
        let trimmed = plain.trim();
        if !trimmed.is_empty() {
            return Ok((Some(trimmed.to_string()), PasswordSource::Plain));
        }
    }

    if let Some(enc) = password_enc {
        let enc_trimmed = enc.trim();
        if !enc_trimmed.is_empty() {
            match camera_store::decrypt_password(enc_trimmed) {
                Ok(value) => {
                    let trimmed = value.trim();
                    if trimmed.is_empty() {
                        return Ok((None, PasswordSource::Encrypted));
                    }
                    return Ok((Some(trimmed.to_string()), PasswordSource::Encrypted));
                }
                Err(err) => {
                    return Err(format!("Failed to decrypt password: {}", err));
                }
            }
        }
    }

    Ok((None, PasswordSource::None))
}

fn resolve_password_with_store(
    host: &str,
    password_plain: Option<String>,
    password_enc: Option<String>,
) -> Result<(Option<String>, PasswordSource), String> {
    let (password_opt, mut source) = resolve_camera_password(password_plain, password_enc)?;
    let mut resolved = password_opt.filter(|value| !value.is_empty());

    if resolved.is_none() {
        match camera_store::load_cameras() {
            Ok(cameras) => {
                if let Some(found) = cameras
                    .iter()
                    .find(|camera| camera.ip.trim().eq_ignore_ascii_case(host.trim()))
                {
                    if let Some(stored) = camera_password(found) {
                        resolved = Some(stored);
                        source = PasswordSource::Stored;
                    }
                }
            }
            Err(err) => {
                println!(
                    "[ssh] failed to load stored cameras for password lookup: {}",
                    err
                );
            }
        }
    }

    Ok((resolved, source))
}

struct PasswordPrompt<'a> {
    password: &'a str,
}

impl<'a> ssh2::KeyboardInteractivePrompt for PasswordPrompt<'a> {
    fn prompt(
        &mut self,
        _username: &str,
        _instructions: &str,
        prompts: &[ssh2::Prompt<'_>],
    ) -> Vec<String> {
        prompts
            .iter()
            .map(|prompt| {
                if prompt.echo {
                    String::new()
                } else {
                    self.password.to_string()
                }
            })
            .collect()
    }
}

fn find_executable_in_path(name: &str) -> Option<PathBuf> {
    if let Some(paths) = env::var_os("PATH") {
        for entry in env::split_paths(&paths) {
            let candidate = entry.join(name);
            if candidate.is_file() {
                return Some(candidate);
            }
        }
    }
    None
}

fn find_winscp_executable() -> Option<PathBuf> {
    const KNOWN_WIN_SCP_LOCATIONS: &[&str] = &[
        r"C:\\Program Files\\WinSCP\\WinSCP.exe",
        r"C:\\Program Files (x86)\\WinSCP\\WinSCP.exe",
    ];

    for entry in KNOWN_WIN_SCP_LOCATIONS {
        let candidate = PathBuf::from(entry);
        if candidate.is_file() {
            return Some(candidate);
        }
    }

    find_executable_in_path("WinSCP.exe").or_else(|| find_executable_in_path("winscp.exe"))
}

fn ssh_username_candidates(username: Option<String>) -> Vec<String> {
    let mut candidates: Vec<String> = Vec::new();

    if let Some(raw) = username {
        let trimmed = raw.trim();
        if !trimmed.is_empty() {
            candidates.push(trimmed.to_string());
        }
    }

    for fallback in ["root", "admin"] {
        if !candidates
            .iter()
            .any(|existing| existing.eq_ignore_ascii_case(fallback))
        {
            candidates.push(fallback.to_string());
        }
    }

    if candidates.is_empty() {
        candidates.push("root".to_string());
    }

    candidates
}

fn build_ssh_session(
    host: &str,
    port: Option<u16>,
    username: Option<String>,
    password_plain: Option<String>,
    password_enc: Option<String>,
) -> Result<Session, String> {
    if host.trim().is_empty() {
        return Err("SSH host is required".to_string());
    }

    let (password_opt, password_source) =
        resolve_password_with_store(host, password_plain, password_enc)?;
    let password = password_opt.unwrap_or_default();

    println!(
        "[ssh] password resolved via {} (length={})",
        password_source.as_str(),
        password.chars().count()
    );
    let username_candidates = ssh_username_candidates(username);
    let primary_username = username_candidates
        .first()
        .cloned()
        .unwrap_or_else(|| "root".to_string());
    let host_trimmed = host.trim();

    if let Some(explicit) = port {
        if explicit != 0 && explicit != 22 {
            println!(
                "[ssh] ignoring configured port {} for {} (forcing port 22 for SSH)",
                explicit, host_trimmed
            );
        }
    }

    let candidate_ports: Vec<u16> = vec![22];

    let mut last_err: Option<String> = None;

    for candidate_port in candidate_ports.iter().copied() {
        println!(
            "[ssh] attempting connection to {}:{} as {}",
            host_trimmed, candidate_port, primary_username
        );
        let connect_target = format!("{}:{}", host_trimmed, candidate_port);

        let mut addrs = match (host_trimmed, candidate_port).to_socket_addrs() {
            Ok(addrs) => addrs,
            Err(err) => {
                println!(
                    "[ssh] failed to resolve {}:{} -> {}",
                    host_trimmed, candidate_port, err
                );
                last_err = Some(format!("Failed to resolve {}: {}", connect_target, err));
                continue;
            }
        };

        let mut tcp_stream: Option<TcpStream> = None;
        let mut inner_err: Option<std::io::Error> = None;
        while let Some(addr) = addrs.next() {
            match TcpStream::connect_timeout(&addr, Duration::from_secs(10)) {
                Ok(stream) => {
                    tcp_stream = Some(stream);
                    println!(
                        "[ssh] TCP connection established to {}:{}",
                        host_trimmed, candidate_port
                    );
                    break;
                }
                Err(err) => {
                    inner_err = Some(err);
                }
            }
        }

        let error_detail = inner_err.as_ref().map(|err| err.to_string());
        let tcp = match tcp_stream {
            Some(stream) => stream,
            None => {
                let message = inner_err
                    .map(|err| format!("Failed to connect to {}: {}", connect_target, err))
                    .unwrap_or_else(|| format!("Failed to connect to {}", connect_target));
                if let Some(detail) = error_detail {
                    println!(
                        "[ssh] TCP connect failed for {}:{} -> {}",
                        host_trimmed, candidate_port, detail
                    );
                } else {
                    println!(
                        "[ssh] TCP connect failed for {}:{} (no error detail)",
                        host_trimmed, candidate_port
                    );
                }
                last_err = Some(message);
                continue;
            }
        };

        let _ = tcp.set_read_timeout(Some(Duration::from_secs(30)));
        let _ = tcp.set_write_timeout(Some(Duration::from_secs(30)));

        let mut session =
            Session::new().map_err(|err| format!("Failed to create SSH session: {}", err))?;
        session.set_tcp_stream(tcp);
        session.set_timeout(30_000);

        if let Err(err) = session.handshake() {
            println!(
                "[ssh] handshake failed for {}:{} -> {}",
                host_trimmed, candidate_port, err
            );
            last_err = Some(format!(
                "SSH handshake failed for {}:{}: {}",
                host_trimmed, candidate_port, err
            ));
            continue;
        }

        println!(
            "[ssh] handshake succeeded for {}:{}",
            host_trimmed, candidate_port
        );

        let mut auth_errors: Vec<String> = Vec::new();

        for (idx, candidate_username) in username_candidates.iter().enumerate() {
            if idx == 0 {
                println!(
                    "[ssh] authenticating as {}@{}:{}",
                    candidate_username, host_trimmed, candidate_port
                );
            } else {
                println!(
                    "[ssh] retrying authentication with fallback username {}@{}:{}",
                    candidate_username, host_trimmed, candidate_port
                );
            }

            match session.userauth_password(candidate_username, &password) {
                Ok(()) => {
                    if session.authenticated() {
                        println!(
                            "[ssh] authentication succeeded for {}@{}:{}",
                            candidate_username, host_trimmed, candidate_port
                        );
                        return Ok(session);
                    }
                    let message = format!(
                        "[ssh] authentication reported failure for {}@{}:{} despite Ok",
                        candidate_username, host_trimmed, candidate_port
                    );
                    println!("{}", message);
                    auth_errors.push(message);
                    continue;
                }
                Err(err) => {
                    let err_msg = err.to_string();
                    println!(
                        "[ssh] authentication failed for {}@{}:{} -> {}",
                        candidate_username, host_trimmed, candidate_port, err_msg
                    );
                    if password.is_empty() {
                        return Err(format!(
                            "SSH authentication failed for {}@{}:{}: no password provided",
                            candidate_username, host_trimmed, candidate_port
                        ));
                    }

                    println!(
                        "[ssh] trying keyboard-interactive auth for {}@{}:{}",
                        candidate_username, host_trimmed, candidate_port
                    );

                    let mut prompt_handler = PasswordPrompt {
                        password: &password,
                    };

                    match session.userauth_keyboard_interactive(
                        candidate_username,
                        &mut prompt_handler,
                    ) {
                        Ok(()) => {
                            if session.authenticated() {
                                println!(
                                    "[ssh] keyboard-interactive authentication succeeded for {}@{}:{}",
                                    candidate_username, host_trimmed, candidate_port
                                );
                                return Ok(session);
                            }
                            let message = format!(
                                "[ssh] keyboard-interactive authentication reported failure for {}@{}:{} despite Ok",
                                candidate_username, host_trimmed, candidate_port
                            );
                            println!("{}", message);
                            auth_errors.push(message);
                        }
                        Err(fallback_err) => {
                            let fallback_msg = fallback_err.to_string();
                            println!(
                                "[ssh] keyboard-interactive authentication failed for {}@{}:{} -> {}",
                                candidate_username, host_trimmed, candidate_port, fallback_msg
                            );
                            auth_errors.push(format!(
                                "SSH authentication failed for {}@{}:{}: {} (keyboard-interactive fallback: {})",
                                candidate_username, host_trimmed, candidate_port, err_msg, fallback_msg
                            ));
                        }
                    }
                }
            }
        }

        if auth_errors.is_empty() {
            last_err = Some(format!(
                "SSH authentication failed for {}:{} (no username attempts executed)",
                host_trimmed, candidate_port
            ));
        } else {
            last_err = Some(format!(
                "SSH authentication failed for {}:{} (tried usernames: {}): {}",
                host_trimmed,
                candidate_port,
                username_candidates
                    .iter()
                    .map(|name| name.as_str())
                    .collect::<Vec<_>>()
                    .join(", "),
                auth_errors.last().cloned().unwrap_or_default()
            ));
        }

        continue;
    }

    Err(last_err.unwrap_or_else(|| {
        format!(
            "Failed to establish SSH session with {} (ports tried: {:?})",
            host_trimmed, candidate_ports
        )
    }))
}

fn is_directory_from_perm(perm: Option<u32>) -> bool {
    perm.map(|mode| (mode & libc::S_IFMT as u32) == libc::S_IFDIR as u32)
        .unwrap_or(false)
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

            let go2rtc_state = Arc::new(StdMutex::new(Go2RtcState::new(&app_handle)));

            // Автоматический запуск Go2RTC при старте приложения
            {
                println!("[setup] Attempting to start go2rtc automatically...");
                let mut guard = go2rtc_state.lock().map_err(|e| {
                    let err_msg = format!("Failed to lock go2rtc state: {}", e);
                    println!("[setup] ERROR: {}", err_msg);
                    Box::new(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        err_msg,
                    )) as Box<dyn std::error::Error>
                })?;

                println!("[setup] Got go2rtc state lock, calling spawn_go2rtc_process...");
                if let Err(err) = spawn_go2rtc_process(&mut guard) {
                    println!("[setup] FAILED to start go2rtc automatically: {}", err);
                    eprintln!("[setup] FAILED to start go2rtc automatically: {}", err);
                    // Не прерываем инициализацию приложения, если Go2RTC не запустился
                    // Пользователь сможет запустить его вручную через команду mediamtx_start
                } else {
                    println!("[setup] ✓ Go2RTC started successfully");
                }
            }

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

            let ssh_shell_state = Arc::new(StdMutex::new(SshShellManager::default()));
            app.manage(ssh_shell_state);

            let analytics_state = match analytics::prepare_analytics_manager(&app_handle) {
                Ok(state) => state,
                Err(err) => return Err(Box::new(err)),
            };
            app.manage(analytics_state);

            // Initialize plate database
            let app_data_dir = app_handle.path().app_data_dir().map_err(|e| {
                Box::new(std::io::Error::new(
                    std::io::ErrorKind::Other,
                    e.to_string(),
                )) as Box<dyn std::error::Error>
            })?;

            if !app_data_dir.exists() {
                std::fs::create_dir_all(&app_data_dir)
                    .map_err(|e| Box::new(e) as Box<dyn std::error::Error>)?;
            }

            let plate_db = database::commands::init_plate_database(&app_data_dir).map_err(|e| {
                Box::new(std::io::Error::new(std::io::ErrorKind::Other, e))
                    as Box<dyn std::error::Error>
            })?;

            let database_state = database::commands::DatabaseState { db: plate_db };
            app.manage(database_state);

            let object_counter_db =
                database::commands::init_object_counter_database(&app_data_dir).map_err(|e| {
                    Box::new(std::io::Error::new(std::io::ErrorKind::Other, e))
                        as Box<dyn std::error::Error>
                })?;

            let object_counter_state =
                database::commands::ObjectCounterDatabaseState { db: object_counter_db };
            app.manage(object_counter_state);

            Ok(())
        })
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_shell::init())
        .plugin(tauri_plugin_updater::Builder::new().build())
        .plugin(tauri_plugin_process::init())
        .invoke_handler(tauri::generate_handler![
            ffmpeg::start_stream,
            ffmpeg::stop_stream,
            ffmpeg::play_recording,
            start_go2rtc,
            add_camera_streams,
            check_stream_status,
            check_camera_http,
            whep_play,
            resolve_stream_source,
            rtsp_handshake,
            probe_hevc_export,
            get_whep_endpoints,
            ensure_path_ready,
            get_go2rtc_snapshot,
            camera_store::save_cameras,
            camera_store::load_cameras,
            camera_store::save_groups,
            camera_store::load_groups,
            camera_store::encrypt_password,
            camera_store::decrypt_password,
            camera_store::remove_camera,
            discovery::discover_cameras,
            discovery::list_network_interfaces,
            onvif::get_rtsp_url,
            onvif::ptz_move,
            onvif::ptz_stop,
            onvif::ptz_focus,
            onvif::focus_stop,
            onvif::get_focus_mode,
            onvif::set_focus_mode,
            play_direct_rtsp,
            save_config_file,
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
            open_camera_file_manager,
            local_fs_list,
            run_camera_ssh_command,
            camera_ssh_shell_open,
            camera_ssh_shell_send,
            camera_ssh_shell_close,
            camera_scp_list,
            camera_scp_download,
            camera_scp_upload,
            camera_sftp_list,
            camera_sftp_download,
            camera_sftp_upload,
            camera_collect_recent_files,
            camera_remote_delete,
            local_fs_delete,
            local_fs_ensure_dir,
            local_reveal_path,
            get_app_resource_usage,
            auth::login,
            auth::auto_login,
            auth::logout,
            auth::get_users,
            auth::add_user,
            auth::update_user_password,
            auth::update_user_role,
            auth::update_user_permissions,
            auth::delete_user,
            analytics::analytics_list_modules,
            analytics::analytics_list_snapshots,
            analytics::analytics_enable_module,
            analytics::analytics_disable_module,
            analytics::analytics_update_module_config,
            analytics::analytics_process_frame,
            analytics::anpr_config::get_anpr_config,
            database::commands::get_plate_records,
            database::commands::get_plate_record_by_id,
            database::commands::update_plate_notes,
            database::commands::delete_plate_record,
            database::commands::get_plate_statistics,
            database::commands::search_plate_history,
            database::commands::read_plate_image,
            database::commands::read_object_image,
            database::commands::get_object_counter_events,
            database::commands::get_object_counter_aggregates,
            database::commands::get_object_counter_top_cameras,
            database::commands::get_object_counter_lines,
            database::commands::upsert_object_counter_line,
            database::commands::delete_object_counter_line,
            database::commands::get_object_counter_zones,
            database::commands::upsert_object_counter_zone,
            database::commands::delete_object_counter_zone,
            check_go2rtc_stream_online
        ])
        .on_window_event(|window, event| {
            if let tauri::WindowEvent::CloseRequested { .. } = event {
                let app_handle = window.app_handle();
                let go2rtc_state = app_handle.state::<Arc<StdMutex<Go2RtcState>>>();
                let go2rtc_arc = go2rtc_state.inner().clone();
                drop(go2rtc_state);

                let mut go2rtc_child = match go2rtc_arc.lock() {
                    Ok(mut guard) => guard.child.take(),
                    Err(err) => {
                        println!(
                            "[shutdown] Failed to lock Go2RTC state for termination: {}",
                            err
                        );
                        None
                    }
                };

                if let Some(mut child) = go2rtc_child.take() {
                    println!("[shutdown] Terminating Go2RTC process...");
                    let _ = child.kill();
                    let _ = child.wait();
                    println!("[shutdown] Go2RTC process terminated");
                }
            }
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[allow(dead_code)]
#[tauri::command]
async fn list_stream_paths(
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<Vec<StreamPathStatus>, String> {
    let settings = load_streaming_settings().await?;
    fetch_go2rtc_paths(&go2rtc_state, &settings).await
}

async fn prewarm_go2rtc_streams(bases: Vec<String>, stream_names: Vec<String>) {
    if bases.is_empty() || stream_names.is_empty() {
        return;
    }

    let client = match Client::builder().timeout(Duration::from_secs(2)).build() {
        Ok(client) => client,
        Err(err) => {
            println!("[go2rtc][warmup] Failed to build HTTP client: {}", err);
            return;
        }
    };

    // КРИТИЧНО: Используем только базовые имена стримов без модификаторов
    // Это предотвращает создание множественных producer'ов в Go2RTC
    let mut unique_sources: BTreeSet<String> = BTreeSet::new();
    for name in stream_names {
        let trimmed = name.trim();
        if trimmed.is_empty() {
            continue;
        }
        unique_sources.insert(trimmed.to_string());
    }

    for base in bases {
        let normalized_base = base.trim().trim_end_matches('/');
        if normalized_base.is_empty() {
            continue;
        }

        for source in &unique_sources {
            let encoded = urlencoding::encode(source);
            let url = format!("{}/api/streams?src={}", normalized_base, encoded);
            match client.get(&url).send().await {
                Ok(response) if !response.status().is_success() => {
                    println!(
                        "[go2rtc][warmup] {} returned status {}",
                        url,
                        response.status()
                    );
                }
                Ok(_) => {
                    // Successful responses are expected (typically HTTP 200/204) and don't need logging.
                }
                Err(err) => {
                    println!("[go2rtc][warmup] request {} failed: {}", url, err);
                }
            }
        }
    }
}

#[allow(dead_code)]
fn load_go2rtc_api_bases_internal(state: &State<'_, Arc<StdMutex<Go2RtcState>>>) -> Vec<String> {
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
        if let Some(base) = parse_address_to_base(api_addr, "http") {
            bases.insert(base);
        }
    }

    if let Some(entries) = yaml.get("apiAddresses").and_then(|v| v.as_sequence()) {
        for entry in entries.iter().filter_map(|v| v.as_str()) {
            if let Some(base) = parse_address_to_base(entry, "http") {
                bases.insert(base);
            }
        }
    }

    if let Some(webrtc_addr) = yaml.get("webrtcAddress").and_then(|v| v.as_str()) {
        if let Some(base) = parse_address_to_base(webrtc_addr, "http") {
            bases.insert(base);
        }
    }

    bases.into_iter().collect()
}

fn ensure_go2rtc_files(state: &mut Go2RtcState) -> Result<(), String> {
    println!("[ensure_go2rtc_files] Starting Go2RTC configuration check...");
    println!(
        "[ensure_go2rtc_files] Go2RTC directory: {:?}",
        state.go2rtc_dir
    );
    println!("[ensure_go2rtc_files] Config path: {:?}", state.config_path);
    println!(
        "[ensure_go2rtc_files] Executable path: {:?}",
        state.exe_path
    );

    if !state.go2rtc_dir.exists() {
        println!("[ensure_go2rtc_files] Creating go2rtc directory...");
        fs::create_dir_all(&state.go2rtc_dir)
            .map_err(|e| format!("Failed to create go2rtc directory: {}", e))?;
    }

    if !state.exe_path.exists() {
        println!(
            "[ensure_go2rtc_files] WARNING: Go2RTC executable not found at {:?}",
            state.exe_path
        );
        return Err(format!("Go2RTC executable not found at {:?}. Please ensure the binary is included in the installer.", state.exe_path));
    } else {
        println!("[ensure_go2rtc_files] Go2RTC executable found");
    }

    if !state.config_path.exists() {
                println!("[ensure_go2rtc_files] Creating default config...");
                let default_cfg = r#"api:
    listen: ":1984"
    origin: "*"
rtsp:
    listen: ":8554"
webrtc:
    listen: ":8555"
    # Low-latency optimizations
    ice_servers: []
streams: {}
"#;

        fs::write(&state.config_path, default_cfg)
            .map_err(|e| format!("Failed to write default go2rtc config: {}", e))?;
        println!("[ensure_go2rtc_files] Default config created");
    }

    println!("[ensure_go2rtc_files] Loading config...");
    let mut config = load_go2rtc_config(state)?;
    let mut updated = false;

    if ensure_go2rtc_api_defaults(&mut config) {
        updated = true;
    }

    if ensure_go2rtc_log_defaults(&mut config) {
        updated = true;
    }

    if ensure_go2rtc_rtsp_defaults(&mut config) {
        updated = true;
    }

    if ensure_go2rtc_webrtc_defaults(&mut config) {
        updated = true;
    }

    if ensure_go2rtc_ffmpeg_defaults(&mut config, state) {
        updated = true;
    }

    if backfill_go2rtc_variants(&mut config) {
        updated = true;
    }

    if updated {
        save_go2rtc_config(state, &config)?;
    }

    Ok(())
}

fn ensure_go2rtc_api_defaults(config: &mut serde_yaml::Value) -> bool {
    use serde_yaml::{Mapping, Value};

    let mut changed = false;

    if !config.is_mapping() {
        *config = Value::Mapping(Mapping::new());
        changed = true;
    }

    let root = config.as_mapping_mut().unwrap();
    let api_key = Value::String("api".to_string());
    let api_entry = root
        .entry(api_key.clone())
        .or_insert(Value::Mapping(Mapping::new()));

    if !api_entry.is_mapping() {
        *api_entry = Value::Mapping(Mapping::new());
        changed = true;
    }

    let api_map = api_entry.as_mapping_mut().unwrap();

    if maybe_update_listen(api_map, ":1984") {
        changed = true;
    }

    let origin_key = Value::String("origin".to_string());
    let desired_origin = Value::String("*".to_string());
    match api_map.get(&origin_key) {
        Some(existing) if existing == &desired_origin => {}
        _ => {
            api_map.insert(origin_key, desired_origin);
            changed = true;
        }
    }

    changed
}

fn ensure_go2rtc_log_defaults(config: &mut serde_yaml::Value) -> bool {
    use serde_yaml::{Mapping, Value};

    if !config.is_mapping() {
        return false;
    }

    let root = config.as_mapping_mut().unwrap();
    let log_key = Value::String("log".into());
    let log_entry = root
        .entry(log_key.clone())
        .or_insert(Value::Mapping(Mapping::new()));

    if !log_entry.is_mapping() {
        *log_entry = Value::Mapping(Mapping::new());
    }

    let map = log_entry.as_mapping_mut().unwrap();
    let level_key = Value::String("level".into());
    // Enable trace logging for debugging
    let desired_level = Value::String("trace".into());

    match map.get(&level_key) {
        Some(existing) if existing == &desired_level => false,
        _ => {
            map.insert(level_key, desired_level);
            true
        }
    }
}

fn maybe_update_listen(map: &mut serde_yaml::Mapping, desired: &str) -> bool {
    use serde_yaml::Value;
    let listen_key = Value::String("listen".into());
    let desired_value = Value::String(desired.into());
    match map.get(&listen_key) {
        Some(Value::String(existing)) if existing == desired => false,
        Some(Value::String(existing)) => {
            let trimmed = existing.trim();
            if trimmed.starts_with("127.0.0.1:") {
                let desired_port = desired.trim_start_matches(':');
                if trimmed.ends_with(desired_port) {
                    map.insert(listen_key, desired_value);
                    return true;
                }
            }
            false
        }
        Some(_) => {
            map.insert(listen_key, desired_value);
            true
        }
        None => {
            map.insert(listen_key, desired_value);
            true
        }
    }
}

fn ensure_go2rtc_rtsp_defaults(config: &mut serde_yaml::Value) -> bool {
    use serde_yaml::{Mapping, Value};

    if !config.is_mapping() {
        *config = Value::Mapping(Mapping::new());
    }

    let root = config.as_mapping_mut().unwrap();
    let key = Value::String("rtsp".into());
    let entry = root.entry(key.clone()).or_insert(Value::Mapping(Mapping::new()));
    if !entry.is_mapping() {
        *entry = Value::Mapping(Mapping::new());
    }

    maybe_update_listen(entry.as_mapping_mut().unwrap(), ":8554")
}

fn ensure_go2rtc_webrtc_defaults(config: &mut serde_yaml::Value) -> bool {
    use serde_yaml::{Mapping, Value};

    if !config.is_mapping() {
        *config = Value::Mapping(Mapping::new());
    }

    let root = config.as_mapping_mut().unwrap();
    let key = Value::String("webrtc".into());
    let entry = root.entry(key.clone()).or_insert(Value::Mapping(Mapping::new()));
    if !entry.is_mapping() {
        *entry = Value::Mapping(Mapping::new());
    }

    let map = entry.as_mapping_mut().unwrap();
    let mut changed = maybe_update_listen(map, ":8555");

    let ice_key = Value::String("ice_servers".into());
    match map.get(&ice_key) {
        Some(Value::Sequence(_)) => {}
        Some(_) => {
            map.insert(ice_key.clone(), Value::Sequence(Vec::new()));
            changed = true;
        }
        None => {
            map.insert(ice_key, Value::Sequence(Vec::new()));
            changed = true;
        }
    }

    changed
}

fn check_nvenc_availability(ffmpeg_path: &str) -> bool {
    use std::process::Command;
    if ffmpeg_path.is_empty() { return false; }
    
    #[cfg(windows)]
    use std::os::windows::process::CommandExt;
    #[cfg(windows)]
    const CREATE_NO_WINDOW: u32 = 0x08000000;

    let mut cmd = Command::new(ffmpeg_path);
    cmd.arg("-hide_banner").arg("-encoders");
    
    #[cfg(windows)]
    cmd.creation_flags(CREATE_NO_WINDOW);

    match cmd.output() {
        Ok(out) => {
            let stdout = String::from_utf8_lossy(&out.stdout);
            stdout.contains("h264_nvenc")
        },
        Err(_) => false,
    }
}

fn ensure_go2rtc_ffmpeg_defaults(config: &mut serde_yaml::Value, state: &Go2RtcState) -> bool {
    use serde_yaml::{Mapping, Value};

    if !config.is_mapping() {
        return false;
    }

    let ffmpeg_bin = resolve_ffmpeg_bin(state);
    let _has_nvenc = check_nvenc_availability(&ffmpeg_bin);

    let root = config.as_mapping_mut().unwrap();
    let ffmpeg_key = Value::String("ffmpeg".into());
    let entry = root
        .entry(ffmpeg_key.clone())
        .or_insert(Value::Mapping(Mapping::new()));

    if !entry.is_mapping() {
        *entry = Value::Mapping(Mapping::new());
    }

    let map = entry.as_mapping_mut().unwrap();
    let mut changed = false;

    let bin_key = Value::String("bin".into());
    let desired_bin = Value::String(ffmpeg_bin);

    match map.get(&bin_key) {
        Some(existing) if existing == &desired_bin => {},
        _ => {
            map.insert(bin_key, desired_bin);
            changed = true;
        }
    }

    // Set h264 preset to fix NVENC zerolatency crash or provide safe software fallback
    let h264_key = Value::String("h264".into());
    // FORCE SOFTWARE ENCODING FOR STABILITY
    // We detected issues with NVENC stream delivery, so we fallback to libx264
    // which is guaranteed to work.
    let desired_h264 = Value::String("-c:v libx264 -g 50 -sc_threshold 0 -preset superfast -tune zerolatency".into());

    match map.get(&h264_key) {
        Some(existing) if existing == &desired_h264 => {
             println!("[ensure_go2rtc_ffmpeg_defaults] h264 preset already correct");
        },
        _ => {
            println!("[ensure_go2rtc_ffmpeg_defaults] Updating h264 preset to software fallback");
            map.insert(h264_key, desired_h264);
            changed = true;
        }
    }

    changed
}

fn resolve_ffmpeg_bin(state: &Go2RtcState) -> String {
    let env_override = env::var_os("FFMPEG_PATH").and_then(|raw| {
        if raw.is_empty() {
            return None;
        }

        let override_path = PathBuf::from(&raw);
        if override_path.is_dir() {
            let binary_name = if cfg!(windows) {
                "ffmpeg.exe"
            } else {
                "ffmpeg"
            };
            Some(override_path.join(binary_name))
        } else {
            Some(override_path)
        }
    });

    if let Some(path) = env_override {
        let override_str = path.to_string_lossy().to_string();
        println!(
            "[resolve_ffmpeg_bin] Using FFMPEG_PATH override: {}",
            override_str
        );
        return override_str;
    }

    #[cfg(windows)]
    {
        if state.ffmpeg_silent_path.exists() {
            return state.ffmpeg_silent_path.to_string_lossy().to_string();
        }

        if state.ffmpeg_path.exists() {
            return state.ffmpeg_path.to_string_lossy().to_string();
        }

        if let Some(path) = resolve_bundled_ffmpeg() {
            return path.to_string_lossy().to_string();
        }

        println!(
            "[resolve_ffmpeg_bin] Bundled ffmpeg-silent binaries not found, falling back to ffmpeg-silent.exe in PATH"
        );
        return "ffmpeg-silent.exe".into();
    }

    #[cfg(not(windows))]
    {
        if state.ffmpeg_path.exists() {
            return state.ffmpeg_path.to_string_lossy().to_string();
        }

        "ffmpeg".into()
    }
}

#[cfg(windows)]
fn resolve_bundled_ffmpeg() -> Option<std::path::PathBuf> {
    use std::collections::HashSet;
    use std::env;

    let exe = env::current_exe().ok()?;
    let exe_dir = exe.parent()?;

    let mut dirs: Vec<PathBuf> = Vec::new();
    let mut push_dir = |dir: PathBuf| {
        if dir.as_os_str().is_empty() {
            return;
        }
        dirs.push(dir);
    };

    // Search specific binary directories first to ensure we find the full toolset (ffmpeg.exe + shims)
    // rather than just shims that might be stranded in the root.
    push_dir(exe_dir.join("binaries"));
    push_dir(exe_dir.join("resources").join("binaries"));
    push_dir(exe_dir.to_path_buf());
    push_dir(exe_dir.join("resources"));

    // Search ancestors and their common binary folders (covers dev + packaged installs).
    for ancestor in exe_dir.ancestors() {
        push_dir(ancestor.to_path_buf());
        push_dir(ancestor.join("binaries"));
        push_dir(ancestor.join("src-tauri").join("binaries"));
        push_dir(ancestor.join("src-tauri").join("binaries").join("windows"));
        push_dir(ancestor.join("src-tauri").join("target").join("debug"));
        push_dir(ancestor.join("src-tauri").join("target").join("release"));
    }

    let mut seen = HashSet::new();
    for dir in dirs {
        if !seen.insert(dir.clone()) {
            continue;
        }

        for filename in [
            "ffmpeg-silent.exe",
            "ffmpeg-silent-launcher.exe",
            "ffmpeg.exe",
        ] {
            let candidate = dir.join(filename);
            if candidate.exists() {
                println!("[resolve_bundled_ffmpeg] Using {}", candidate.display());
                // Debug log to verify path in production
                if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open("C:\\Users\\Public\\dashboard_path.log") {
                    let _ = writeln!(f, "Found ffmpeg at: {}", candidate.display());
                }
                return Some(candidate);
            }
        }
    }

    None
}

pub(crate) fn resolve_ffmpeg_command() -> String {
    #[cfg(windows)]
    {
        if let Some(path) = resolve_bundled_ffmpeg() {
            return path.to_string_lossy().to_string();
        }
        println!(
            "[resolve_ffmpeg_command] Bundled ffmpeg-silent binaries not found, falling back to ffmpeg-silent.exe"
        );
        return "ffmpeg-silent.exe".into();
    }

    #[cfg(not(windows))]
    {
        "ffmpeg".into()
    }
}

fn backfill_go2rtc_variants(config: &mut serde_yaml::Value) -> bool {
    use serde_yaml::Value;

    // Clone the top-level mapping to iterate safely before mutating config.
    let root = match config.as_mapping() {
        Some(map) => map.clone(),
        None => return false,
    };

    let streams_value = match root.get(&Value::String("streams".into())) {
        Some(value) => value.clone(),
        None => return false,
    };

    let streams_map = match streams_value.as_mapping() {
        Some(map) => map,
        None => return false,
    };

    // Collect base RTSP streams so we can regenerate codec-specific variants with ffmpeg fallbacks.
    let mut candidates: Vec<(String, String)> = Vec::new();

    for (key, value) in streams_map.iter() {
        let name = match key.as_str() {
            Some(name) if !name.contains('#') => name.to_string(),
            _ => continue,
        };

        let url = match value.as_sequence().and_then(|seq| seq.first()) {
            Some(Value::String(url))
                if url.starts_with("rtsp://") || url.starts_with("rtsps://") =>
            {
                url.to_string()
            }
            _ => continue,
        };

        candidates.push((name, url));
    }

    let mut changed = false;
    for (name, url) in candidates {
        if ensure_go2rtc_stream(config, &name, &url) {
            changed = true;
        }
    }

    changed
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

fn ensure_go2rtc_preload(config: &mut serde_yaml::Value, stream_name: &str, query: &str) -> bool {
    use serde_yaml::{Mapping, Value};

    if !config.is_mapping() {
        return false;
    }

    let root = config.as_mapping_mut().unwrap();
    let preload_entry = root
        .entry(Value::String("preload".into()))
        .or_insert(Value::Mapping(Mapping::new()));

    if !preload_entry.is_mapping() {
        *preload_entry = Value::Mapping(Mapping::new());
    }

    let preload_map = preload_entry.as_mapping_mut().unwrap();
    let key = Value::String(stream_name.to_string());
    let desired = Value::String(query.to_string());

    match preload_map.get(&key) {
        Some(existing) if existing == &desired => false,
        _ => {
            preload_map.insert(key, desired);
            true
        }
    }
}

fn ensure_go2rtc_stream(map: &mut serde_yaml::Value, stream_name: &str, url: &str) -> bool {
    use serde_yaml::Value;

    if !map.is_mapping() {
        *map = Value::Mapping(serde_yaml::Mapping::new());
    }

    let mut changed = false;
    // DISABLED: Preload causes Go2RTC to crash if camera is unreachable at startup
    // Let streams start on-demand instead of preloading
    let should_preload = false; // was: !stream_name.contains('#') && (url.starts_with("rtsp://") || url.starts_with("rtsps://"));

    // Add RTSP buffer parameter for low latency
    // Note: Frontend controls actual latency behavior via WebRTC settings
    // This just sets go2rtc's RTSP buffer to minimum by default
    // DISABLED: buffer=0 can cause connection issues with some cameras
    let optimized_url = url.to_string();

    /*
    let optimized_url = if url.starts_with("rtsp://") || url.starts_with("rtsps://") {
        if url.contains('?') {
            if !url.contains("buffer=") {
                format!("{}&buffer=0", url)
            } else {
                url.to_string()
            }
        } else {
            format!("{}?buffer=0", url)
        }
    } else {
        url.to_string()
    };
    */

    {
        let root = map.as_mapping_mut().unwrap();
        let streams_entry = root
            .entry(Value::String("streams".into()))
            .or_insert(Value::Mapping(serde_yaml::Mapping::new()));

        if !streams_entry.is_mapping() {
            *streams_entry = Value::Mapping(serde_yaml::Mapping::new());
        }

        let streams_map = streams_entry.as_mapping_mut().unwrap();
        let key = Value::String(stream_name.to_string());

        let mut insert_base = false;

        match streams_map.get_mut(&key) {
            Some(existing) => match existing {
                Value::Sequence(sequence) => {
                    // CRITICAL FIX: Check if ANY URL in sequence differs from expected
                    // Previously we only checked the first, but variants can appear first
                    // If the base stream URL changed (e.g., password updated), we must restart go2rtc
                    let base_url_matches = sequence
                        .iter()
                        .filter_map(|v| v.as_str())
                        .any(|s| s == &optimized_url);

                    if !base_url_matches || sequence.len() != 1 {
                        println!("[ensure_go2rtc_stream] Stream '{}' URL changed or has extra entries, replacing", stream_name);
                        *existing = Value::Sequence(vec![Value::String(optimized_url.clone())]);
                        changed = true;
                    }
                }
                Value::String(current) => {
                    if current != &optimized_url {
                        println!("[ensure_go2rtc_stream] Stream '{}' URL changed (was string)", stream_name);
                        *existing = Value::Sequence(vec![Value::String(optimized_url.clone())]);
                        changed = true;
                    }
                }
                _ => {
                    println!("[ensure_go2rtc_stream] Stream '{}' had non-string/non-sequence value", stream_name);
                    *existing = Value::Sequence(vec![Value::String(optimized_url.clone())]);
                    changed = true;
                }
            },
            None => {
                insert_base = true;
            }
        }

        if insert_base {
            println!("[ensure_go2rtc_stream] Adding new stream '{}'", stream_name);
            streams_map.insert(
                key,
                Value::Sequence(vec![Value::String(optimized_url.clone())]),
            );
            changed = true;
        }

        // Ensure helper variants that request audio transcoding exist so WHEP lookups
        // like "cam3_0#audio=opus" resolve instead of returning 404.
        if optimized_url.starts_with("rtsp://") || optimized_url.starts_with("rtsps://") {
            const VARIANTS: [&str; 3] = [
                "#audio=opus",
                "#video=h264#audio=opus",
                "#video=copy#audio=opus",
            ];

            for suffix in VARIANTS.into_iter() {
                let variant_name = format!("{}{}", stream_name, suffix);
                let variant_key = Value::String(variant_name);

                let mut sources: Vec<Value> = Vec::new();
                let direct_variant = format!("{}{}", url, suffix);
                sources.push(Value::String(direct_variant));

                // Add ffmpeg-backed fallback so WebRTC always has an Opus track even when the camera lacks it.
                let ffmpeg_variant = format!("ffmpeg:{}{}", stream_name, suffix);
                if !sources
                    .iter()
                    .any(|value| matches!(value, Value::String(s) if s == &ffmpeg_variant))
                {
                    sources.push(Value::String(ffmpeg_variant));
                }

                let variant_value = Value::Sequence(sources);

                match streams_map.get(&variant_key) {
                    Some(existing) if existing == &variant_value => {}
                    _ => {
                        streams_map.insert(variant_key, variant_value);
                        changed = true;
                    }
                }
            }
        }
    }

    if should_preload {
        if ensure_go2rtc_preload(map, stream_name, "video&audio=opus") {
            changed = true;
        }
    }

    changed
}

fn spawn_go2rtc_process(state: &mut Go2RtcState) -> Result<(), String> {
    // Check if Go2RTC API is already listening
    if std::net::TcpStream::connect("127.0.0.1:1984").is_ok() {
        println!("[spawn_go2rtc_process] Go2RTC API is already listening on 127.0.0.1:1984. Assuming external process.");
        return Ok(());
    }

    println!("[spawn_go2rtc_process] Starting Go2RTC process...");
    println!("[spawn_go2rtc_process] Executable: {:?}", state.exe_path);
    println!("[spawn_go2rtc_process] Config: {:?}", state.config_path);
    println!(
        "[spawn_go2rtc_process] Working directory: {:?}",
        state.go2rtc_dir
    );

    ensure_go2rtc_files(state)?;

    if !state.exe_path.exists() {
        let error = format!(
            "Go2RTC executable not found at {:?}. Candidates checked:\n\
            - {:?}\n\
            Please ensure go2rtc.exe is present in src-tauri/binaries/ for dev builds,\n\
            or in the installed application resources for production builds.",
            state.exe_path,
            Go2RtcState::resource_candidates()
        );
        println!("[spawn_go2rtc_process] ERROR: {}", error);
        return Err(error);
    }

    println!("[spawn_go2rtc_process] Building command...");
    let mut cmd = StdCommand::new(&state.exe_path);
    cmd.arg("-c")
        .arg(&state.config_path)
        .current_dir(&state.go2rtc_dir)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped());

    #[cfg(windows)]
    {
        // CREATE_NO_WINDOW (0x08000000) - don't create a console window
        // DETACHED_PROCESS (0x00000008) - detach from parent console
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        const DETACHED_PROCESS: u32 = 0x00000008;
        cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
        println!("[spawn_go2rtc_process] Set CREATE_NO_WINDOW and DETACHED_PROCESS flags");
    }

    println!("[spawn_go2rtc_process] Spawning process...");
    let mut child = cmd.spawn().map_err(|e| {
        let error = format!("Failed to start go2rtc: {}", e);
        println!("[spawn_go2rtc_process] ERROR: {}", error);
        error
    })?;

    println!(
        "[spawn_go2rtc_process] Process spawned with PID: {:?}",
        child.id()
    );

    if let Some(stdout) = child.stdout.take() {
        std::thread::spawn(move || {
            let reader = BufReader::new(stdout);
            for line in reader.lines().flatten() {
                println!("[go2rtc][out] {}", line);
            }
        });
    }

    if let Some(stderr) = child.stderr.take() {
        std::thread::spawn(move || {
            let reader = BufReader::new(stderr);
            for line in reader.lines().flatten() {
                eprintln!("[go2rtc][err] {}", line);
            }
        });
    }

    // Give Go2RTC a moment to initialize and check if it's still running
    std::thread::sleep(std::time::Duration::from_millis(500));

    match child.try_wait() {
        Ok(Some(status)) => {
            let error = format!("Go2RTC process exited immediately with status: {}", status);
            println!("[spawn_go2rtc_process] ERROR: {}", error);
            return Err(error);
        }
        Ok(None) => {
            println!("[spawn_go2rtc_process] Go2RTC process is running");
        }
        Err(e) => {
            println!(
                "[spawn_go2rtc_process] Warning: Could not check process status: {}",
                e
            );
        }
    }

    state.child = Some(child);
    println!("[spawn_go2rtc_process] Go2RTC process started successfully");

    // Wait for Go2RTC API to become available in a blocking thread
    println!("[spawn_go2rtc_process] Waiting for Go2RTC API to be ready...");

    // Use a simple loop with std::thread::sleep to avoid tokio blocking issues
    let start = std::time::Instant::now();
    let timeout = std::time::Duration::from_secs(10);
    let check_interval = std::time::Duration::from_millis(100);

    while start.elapsed() < timeout {
        match std::net::TcpStream::connect_timeout(
            &"127.0.0.1:1984".parse().unwrap(),
            std::time::Duration::from_millis(500),
        ) {
            Ok(_) => {
                println!("[spawn_go2rtc_process] Go2RTC API is ready");
                return Ok(());
            }
            Err(_) => {
                std::thread::sleep(check_interval);
            }
        }
    }

    Err("Timeout waiting for Go2RTC API to become ready".to_string())
}

#[allow(dead_code)]
fn wait_for_go2rtc_ready_deprecated() -> Result<(), String> {
    // Deprecated: causes panic when called from async context
    // Use TcpStream check in spawn_go2rtc_process instead
    use std::time::{Duration, Instant};

    let start = Instant::now();
    let timeout = Duration::from_secs(10);
    let check_interval = Duration::from_millis(100);

    while start.elapsed() < timeout {
        match reqwest::blocking::Client::builder()
            .timeout(Duration::from_millis(500))
            .build()
        {
            Ok(client) => {
                if let Ok(response) = client.get("http://127.0.0.1:1984/api").send() {
                    if response.status().is_success() {
                        return Ok(());
                    }
                }
            }
            Err(e) => {
                println!(
                    "[wait_for_go2rtc_ready_deprecated] Failed to create HTTP client: {}",
                    e
                );
            }
        }
        std::thread::sleep(check_interval);
    }

    Err("Timeout waiting for Go2RTC API to become ready".to_string())
}

// Restart Go2RTC process with updated configuration
fn restart_go2rtc(state: &mut Go2RtcState) -> Result<(), String> {
    println!("[restart_go2rtc] Stopping Go2RTC process...");
    if let Some(mut child) = state.child.take() {
        let _ = child.kill();
        let _ = child.wait();
    }
    println!("[restart_go2rtc] Starting Go2RTC process...");
    spawn_go2rtc_process(state)
}

#[allow(dead_code)]
fn restart_go2rtc_if_running(state: &mut Go2RtcState) -> Result<(), String> {
    if state.child.is_some() {
        restart_go2rtc(state)?;
    }
    Ok(())
}

#[tauri::command]
async fn start_go2rtc(
    state: tauri::State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<String, String> {
    let arc = state.inner().clone();
    let mut guard = arc
        .lock()
        .map_err(|e| format!("Failed to lock go2rtc state: {}", e))?;

    // If already running, just return success
    if guard.child.is_some() {
        return Ok("go2rtc already running".to_string());
    }

    // Start go2rtc process
    spawn_go2rtc_process(&mut guard)?;

    Ok("go2rtc started successfully".to_string())
}

#[tauri::command]
async fn get_go2rtc_snapshot(
    stream_name: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<Vec<u8>, String> {
    // Load settings to get the correct API URL
    let settings = load_streaming_settings().await.map_err(|e| e.to_string())?;
    let api_base = settings.go2rtc_api_bases().first().cloned().unwrap_or_else(|| GO2RTC_DEFAULT_API.to_string());

    let api_url = format!("{}/api/frame.jpeg?src={}", api_base, stream_name);
    
    println!("[get_go2rtc_snapshot] Fetching snapshot from: {}", api_url);
    
    let client = Client::builder()
        .timeout(Duration::from_secs(5))
        .build()
        .map_err(|e| format!("Failed to create HTTP client: {}", e))?;

    // Try HTTP first
    let http_result = async {
        let response = client.get(&api_url).send().await.map_err(|e| e.to_string())?;
        if !response.status().is_success() {
            let status = response.status();
            let text = response.text().await.unwrap_or_default();
            return Err(format!("HTTP {} - {}", status, text));
        }
        response.bytes().await.map(|b| b.to_vec()).map_err(|e| e.to_string())
    }.await;

    if let Ok(bytes) = http_result {
        println!("[get_go2rtc_snapshot] HTTP snapshot success, {} bytes", bytes.len());
        return Ok(bytes);
    }

    let http_error = http_result.err().unwrap();
    println!("[get_go2rtc_snapshot] HTTP snapshot failed: {}. Attempting fallback...", http_error);

    // Fallback: Use ffmpeg directly
    // Scope the lock to ensure it is dropped before await
    let (url, ffmpeg_bin) = {
        let state_guard = go2rtc_state.lock().map_err(|_| "Failed to lock state".to_string())?;
        let config = load_go2rtc_config(&state_guard)?;
        
        // Find stream URL
        let stream_url = if let Some(streams) = config.get("streams").and_then(|s| s.as_mapping()) {
            if let Some(val) = streams.get(&serde_yaml::Value::String(stream_name.clone())) {
                match val {
                    serde_yaml::Value::String(s) => Some(s.clone()),
                    serde_yaml::Value::Sequence(seq) => {
                        seq.first().and_then(|v| v.as_str()).map(|s| s.to_string())
                    },
                    _ => None
                }
            } else {
                None
            }
        } else {
            None
        };

        let url = stream_url.ok_or_else(|| format!("Stream '{}' not found in config, cannot use fallback. Original error: {}", stream_name, http_error))?;
        let bin = resolve_ffmpeg_bin(&state_guard);
        (url, bin)
    };

    println!("[get_go2rtc_snapshot] Fallback: capturing from {} using {}", url, ffmpeg_bin);

    // Use a temporary file instead of stdout to avoid issues with ffmpeg wrappers swallowing stdout
    let temp_dir = std::env::temp_dir();
    let temp_filename = format!("snapshot_{}.jpg", SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_millis());
    let temp_path = temp_dir.join(temp_filename);
    let temp_path_str = temp_path.to_string_lossy().to_string();

    let mut cmd = TokioCommand::new(ffmpeg_bin);
    cmd.args(&["-y", "-rtsp_transport", "tcp", "-i", &url, "-frames:v", "1", "-f", "image2", &temp_path_str]);

    // Ensure no window is shown on Windows, even if using standard ffmpeg
    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        cmd.creation_flags(CREATE_NO_WINDOW);
    }

    let output = cmd.output()
        .await
        .map_err(|e| format!("Failed to execute ffmpeg: {}", e))?;

    if output.status.success() {
        if temp_path.exists() {
            match fs::read(&temp_path) {
                Ok(bytes) => {
                    let _ = fs::remove_file(&temp_path);
                    if bytes.is_empty() {
                        println!("[get_go2rtc_snapshot] Fallback failed: Captured file is empty");
                        Err("Captured file is empty".to_string())
                    } else {
                        println!("[get_go2rtc_snapshot] Fallback success, captured {} bytes", bytes.len());
                        Ok(bytes)
                    }
                },
                Err(e) => {
                    println!("[get_go2rtc_snapshot] Fallback failed: Could not read temp file: {}", e);
                    Err(format!("Failed to read snapshot file: {}", e))
                }
            }
        } else {
            println!("[get_go2rtc_snapshot] Fallback failed: Output file not created");
            Err("FFmpeg reported success but output file was not created".to_string())
        }
    } else {
        let stderr = String::from_utf8_lossy(&output.stderr);
        println!("[get_go2rtc_snapshot] Fallback failed: {}", stderr);
        Err(format!("Snapshot failed. HTTP: {}. Fallback (ffmpeg): {}", http_error, stderr))
    }
}

#[allow(dead_code)]
fn ensure_webrtc_codecs(config: &mut serde_yaml::Value) -> bool {
    use serde_yaml::Value;

    if !config.is_mapping() {
        return false;
    }

    let map = config.as_mapping_mut().unwrap();
    let removed_primary = map.remove(&Value::String("webrtcCodecs".into())).is_some();
    let removed_additional = map
        .remove(&Value::String("webrtcAdditionalCodecs".into()))
        .is_some();

    if removed_primary || removed_additional {
        println!("[mediamtx] Removed unsupported webrtc codec configuration keys");
    }

    removed_primary || removed_additional
}

#[tauri::command]
fn get_app_resource_usage(
    system_state: State<'_, Arc<StdMutex<System>>>,
) -> Result<AppResourceUsage, String> {
    let mut system = system_state
        .lock()
        .map_err(|_| "Failed to lock system information state".to_string())?;

    system.refresh_cpu();
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

#[allow(dead_code)]
fn ensure_paths_mapping<'a>(root: &'a mut serde_yaml::Value) -> &'a mut serde_yaml::Mapping {
    if !root.is_mapping() {
        *root = serde_yaml::Value::Mapping(serde_yaml::Mapping::new());
    }
    root.as_mapping_mut().unwrap()
}

#[allow(dead_code)]
fn ensure_stream_path(
    map: &mut serde_yaml::Value,
    stream_name: &str,
    pipeline: &LiveStreamPipeline,
    enable_on_demand: bool,
) -> bool {
    use serde_yaml::Value;

    let root_map = ensure_paths_mapping(map);
    let paths_entry = root_map
        .entry(Value::String("paths".into()))
        .or_insert(Value::Mapping(serde_yaml::Mapping::new()));

    let paths_map = ensure_paths_mapping(paths_entry);
    let key = Value::String(stream_name.to_string());

    let desired_value = match pipeline {
        LiveStreamPipeline::Direct { source_url } => {
            let mut stream_map = serde_yaml::Mapping::new();
            stream_map.insert(
                Value::String("source".into()),
                Value::String(source_url.to_string()),
            );
            stream_map.insert(
                Value::String("sourceOnDemand".into()),
                Value::Bool(enable_on_demand),
            );
            stream_map.insert(
                Value::String("rtspTransport".into()),
                Value::String("tcp".into()),
            );
            stream_map.insert(
                Value::String("disablePublisherOverride".into()),
                Value::Bool(false),
            );
            Value::Mapping(stream_map)
        }
        LiveStreamPipeline::Transcode { command, .. } => {
            let mut stream_map = serde_yaml::Mapping::new();
            stream_map.insert(
                Value::String("source".into()),
                Value::String("publisher".into()),
            );
            stream_map.insert(
                Value::String("runOnDemand".into()),
                Value::String(command.to_string()),
            );
            stream_map.insert(
                Value::String("runOnDemandRestart".into()),
                Value::Bool(true),
            );
            stream_map.insert(
                Value::String("runOnDemandStartTimeout".into()),
                Value::String("20s".into()),
            );
            stream_map.insert(
                Value::String("runOnDemandCloseAfter".into()),
                Value::String("15s".into()),
            );
            stream_map.insert(
                Value::String("disablePublisherOverride".into()),
                Value::Bool(false),
            );
            Value::Mapping(stream_map)
        }
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

fn extract_go2rtc_source(config: &serde_yaml::Value, stream_name: &str) -> Option<String> {
    let streams = config.get("streams")?.as_mapping()?;
    let entry = streams.get(&serde_yaml::Value::String(stream_name.to_string()))?;

    match entry {
        serde_yaml::Value::String(value) if !value.trim().is_empty() => {
            Some(value.trim().to_string())
        }
        serde_yaml::Value::Sequence(sequence) => {
            for item in sequence {
                match item {
                    serde_yaml::Value::String(value) if !value.trim().is_empty() => {
                        return Some(value.trim().to_string());
                    }
                    serde_yaml::Value::Mapping(map) => {
                        if let Some(url) = map
                            .get(&serde_yaml::Value::String("url".into()))
                            .and_then(|v| v.as_str())
                        {
                            let trimmed = url.trim();
                            if !trimmed.is_empty() {
                                return Some(trimmed.to_string());
                            }
                        }
                    }
                    _ => {}
                }
            }
            None
        }
        serde_yaml::Value::Mapping(map) => map
            .get(&serde_yaml::Value::String("url".into()))
            .and_then(|v| v.as_str())
            .map(|s| s.trim().to_string()),
        _ => None,
    }
}

#[allow(dead_code)]
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

#[allow(dead_code)]
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
            if let Some(url) = map.get(key).and_then(|v| v.as_str()).map(|s| s.to_string()) {
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

#[allow(dead_code)]
fn map_go2rtc_stream(
    name: &str,
    entry: &serde_json::Value,
    settings: &StreamingSettings,
) -> StreamPathStatus {
    let ready = entry
        .get("status")
        .and_then(|v| v.as_str())
        .map(|s| {
            matches!(
                s.to_ascii_lowercase().as_str(),
                "online" | "ready" | "active"
            )
        })
        .or_else(|| {
            entry
                .get("state")
                .and_then(|v| v.as_str())
                .map(|s| matches!(s.to_ascii_lowercase().as_str(), "ok" | "online" | "ready"))
        })
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
        .or_else(|| {
            entry
                .get("url")
                .and_then(|v| v.as_str())
                .map(|s| s.to_string())
        })
        .or_else(|| {
            entry
                .get("producers")
                .and_then(|v| v.as_object())
                .and_then(|map| map.values().find_map(extract_url_from_value))
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

fn load_go2rtc_api_bases(
    state: &State<'_, Arc<StdMutex<Go2RtcState>>>,
    settings: &StreamingSettings,
) -> Vec<String> {
    let mut bases: BTreeSet<String> = settings
        .go2rtc_api_bases()
        .into_iter()
        .filter(|s| !s.trim().is_empty())
        .collect();

    if bases.is_empty() {
        bases.insert(GO2RTC_DEFAULT_API.to_string());
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
            println!("[go2rtc] Failed to read config {:?}: {}", config_path, err);
            return bases.into_iter().collect();
        }
    };

    let yaml: serde_yaml::Value = match serde_yaml::from_str(&config_raw) {
        Ok(value) => value,
        Err(err) => {
            println!("[go2rtc] Failed to parse config {:?}: {}", config_path, err);
            return bases.into_iter().collect();
        }
    };

    if let Some(api_node) = yaml.get("api") {
        if let Some(addr) = api_node.as_str() {
            if let Some(base) = parse_address_to_base(addr, "http") {
                bases.insert(base);
            }
        } else if let Some(map) = api_node.as_mapping() {
            for key in ["listen", "address", "addr"] {
                if let Some(value) = map
                    .get(&serde_yaml::Value::String(key.to_string()))
                    .and_then(|v| v.as_str())
                {
                    if let Some(base) = parse_address_to_base(value, "http") {
                        bases.insert(base);
                    }
                }
            }
        }
    }

    if let Some(http_node) = yaml.get("http") {
        if let Some(addr) = http_node.as_str() {
            if let Some(base) = parse_address_to_base(addr, "http") {
                bases.insert(base);
            }
        } else if let Some(map) = http_node.as_mapping() {
            for key in ["listen", "address", "addr"] {
                if let Some(value) = map
                    .get(&serde_yaml::Value::String(key.to_string()))
                    .and_then(|v| v.as_str())
                {
                    if let Some(base) = parse_address_to_base(value, "http") {
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

#[allow(dead_code)]
async fn fetch_go2rtc_paths(
    go2rtc_state: &State<'_, Arc<StdMutex<Go2RtcState>>>,
    settings: &StreamingSettings,
) -> Result<Vec<StreamPathStatus>, String> {
    let bases = load_go2rtc_api_bases(go2rtc_state, settings);
    if bases.is_empty() {
        return Ok(Vec::new());
    }

    let client = Client::builder()
        .timeout(Duration::from_secs(3))
        .build()
        .map_err(|e| format!("Failed to build HTTP client: {}", e))?;

    let mut last_error: Option<String> = None;

    for base in bases {
        let trimmed = base.trim().trim_end_matches('/');
        if trimmed.is_empty() {
            continue;
        }

        let endpoints = [
            format!("{}/api/streams", trimmed),
            format!("{}/streams", trimmed),
            format!("{}/api/streams/list", trimmed),
        ];

        for endpoint in &endpoints {
            match client.get(endpoint).send().await {
                Ok(response) if response.status().is_success() => {
                    let payload: serde_json::Value = response
                        .json()
                        .await
                        .map_err(|e| format!("Failed to parse go2rtc response: {}", e))?;

                    let mut result: Vec<StreamPathStatus> = Vec::new();

                    if let Some(map) = payload.get("streams").and_then(|v| v.as_object()) {
                        for (name, entry) in map {
                            result.push(map_go2rtc_stream(name, entry, settings));
                        }
                    } else if let Some(map) = payload.as_object() {
                        for (name, entry) in map {
                            result.push(map_go2rtc_stream(name, entry, settings));
                        }
                    } else if let Some(array) = payload.as_array() {
                        for entry in array {
                            if let Some(name) = entry.get("name").and_then(|v| v.as_str()) {
                                result.push(map_go2rtc_stream(name, entry, settings));
                            }
                        }
                    }

                    if !result.is_empty() {
                        result.sort_by(|a, b| a.name.cmp(&b.name));
                        return Ok(result);
                    }
                }
                Ok(response) => {
                    last_error = Some(format!(
                        "go2rtc endpoint {} returned status {}",
                        endpoint,
                        response.status()
                    ));
                }
                Err(err) => {
                    last_error = Some(format!("Failed to query {}: {}", endpoint, err));
                }
            }
        }
    }

    if let Some(err) = last_error {
        Err(err)
    } else {
        Ok(Vec::new())
    }
}

#[allow(dead_code)]
fn parse_origin_from_run_on_demand(command: &str) -> Option<String> {
    let mut parts = command.split("-i");
    parts.next()?; // skip text before first -i

    if let Some(after_flag) = parts.next() {
        let trimmed = after_flag.trim_start();
        if trimmed.starts_with('"') {
            let remainder = &trimmed[1..];
            if let Some(end) = remainder.find('"') {
                return Some(remainder[..end].to_string());
            }
        } else if !trimmed.is_empty() {
            let end = trimmed
                .find(|c: char| c.is_whitespace())
                .unwrap_or(trimmed.len());
            return Some(trimmed[..end].to_string());
        }
    }

    None
}

#[allow(dead_code)]
fn infer_rtsp_transport(transport_header: &str) -> Option<&'static str> {
    let lower = transport_header.to_ascii_lowercase();
    if lower.contains("tcp") {
        Some("tcp")
    } else if lower.contains("udp") {
        Some("udp")
    } else {
        None
    }
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

#[allow(dead_code)]
#[tauri::command]
async fn mediamtx_start(
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<String, String> {
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
    Ok("started".into())
}

#[allow(dead_code)]
#[tauri::command]
async fn mediamtx_stop(
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<String, String> {
    match go2rtc_state.lock() {
        Ok(mut guard) => {
            if let Some(mut child) = guard.child.take() {
                let _ = child.kill();
                let _ = child.wait();
                Ok("stopped".into())
            } else {
                Ok("not running".into())
            }
        }
        Err(err) => Err(format!("Failed to lock go2rtc state for stop: {}", err)),
    }
}

#[allow(dead_code)]
#[tauri::command]
async fn add_camera_to_mediamtx(
    name: String,
    url: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let stream_name = sanitize_stream_key(&name);
    let mut guard = go2rtc_state
        .lock()
        .map_err(|_| "Failed to lock go2rtc state".to_string())?;

    ensure_go2rtc_files(&mut guard)?;
    let mut config = load_go2rtc_config(&guard)?;
    let updated = ensure_go2rtc_stream(&mut config, &stream_name, url.trim());
    if updated {
        save_go2rtc_config(&guard, &config)?;
        println!(
            "go2rtc stream '{}' added to config, restarting Go2RTC...",
            stream_name
        );

        // Restart Go2RTC to load the new stream
        restart_go2rtc(&mut guard)?;
        println!("Go2RTC restarted successfully for stream '{}'", stream_name);
    } else {
        println!("go2rtc stream '{}' already configured", stream_name);
    }
    Ok(true)
}

#[allow(dead_code)]
#[tauri::command]
async fn mediamtx_add_camera(
    name: String,
    rtsp: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    add_camera_to_mediamtx(name, rtsp, go2rtc_state).await
}

#[tauri::command]
async fn add_camera_streams(
    camera_id: u32,
    hd_url: String,
    sd_url: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let settings = load_streaming_settings().await?;
    let hd_stream = format!("cam{}_0", camera_id);
    let sd_stream = format!("cam{}_1", camera_id);
    let warmup_streams = vec![hd_stream.clone(), sd_stream.clone()];

    let mut guard = go2rtc_state
        .lock()
        .map_err(|_| "Failed to lock go2rtc state".to_string())?;

    ensure_go2rtc_files(&mut guard)?;
    let mut config = load_go2rtc_config(&guard)?;
    let mut updated = false;

    if ensure_go2rtc_stream(&mut config, &hd_stream, hd_url.trim()) {
        updated = true;
    }
    if ensure_go2rtc_stream(&mut config, &sd_stream, sd_url.trim()) {
        updated = true;
    }

    if updated {
        save_go2rtc_config(&guard, &config)?;
        println!(
            "go2rtc config updated with streams '{}' and '{}', restarting Go2RTC...",
            hd_stream, sd_stream
        );

        // Restart Go2RTC to load the new streams
        restart_go2rtc(&mut guard)?;
        println!("Go2RTC restarted successfully");
    } else {
        println!(
            "go2rtc streams '{}' and '{}' already configured",
            hd_stream, sd_stream
        );
    }

    // Release the lock before calling load_go2rtc_api_bases to avoid deadlock
    drop(guard);

    let warmup_bases = load_go2rtc_api_bases(&go2rtc_state, &settings);
    tauri::async_runtime::spawn(async move {
        if warmup_bases.is_empty() {
            return;
        }
        tokio_sleep(Duration::from_millis(500)).await;
        prewarm_go2rtc_streams(warmup_bases, warmup_streams).await;
    });

    Ok(true)
}

#[allow(dead_code)]
#[tauri::command]
async fn check_mediamtx_path_ready(
    path_name: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let path = path_name.trim().to_string();
    if path.is_empty() {
        return Ok(false);
    }
    let settings = load_streaming_settings().await?;
    let statuses = fetch_go2rtc_paths(&go2rtc_state, &settings).await?;
    Ok(statuses
        .iter()
        .any(|status| status.name == path && status.ready))
}

fn parse_address_to_base(addr: &str, default_scheme: &str) -> Option<String> {
    let mut scheme = if default_scheme.is_empty() {
        "http"
    } else {
        default_scheme
    };

    let mut addr = addr.trim().trim_matches('"');
    if let Some(stripped) = addr.strip_prefix("http://") {
        scheme = "http";
        addr = stripped;
    } else if let Some(stripped) = addr.strip_prefix("https://") {
        scheme = "https";
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

    Some(format!("{}://{}:{}", scheme, host, port))
}

#[tauri::command]
async fn whep_play(
    path: String,
    offer_sdp: String,
    rtsp_session: Option<String>,
    rtsp_transport: Option<String>,
    hevc_supported: Option<bool>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<String, String> {
    let settings = load_streaming_settings().await?;
    println!("[whep_play] Active streaming provider: Go2RTC");
    println!("[whep_play] Preparing WHEP endpoints for path '{}' (HEVC supported: {:?})", path, hevc_supported);

    let base_urls = load_go2rtc_api_bases(&go2rtc_state, &settings);
    let mut endpoints: Vec<String> = Vec::new();
    let mut seen: HashSet<String> = HashSet::new();
    let path_ref = &path;

    let mut src_variants: Vec<String> = Vec::new();
    
    // If client explicitly says HEVC is NOT supported, we force H.264 transcoding
    if hevc_supported == Some(false) {
        // Transcode video to H.264 and audio to Opus (best compatibility)
        src_variants.push(format!("{}#video=h264#audio=opus", path_ref));
        // Transcode video to H.264, keep audio as is (fallback)
        src_variants.push(format!("{}#video=h264", path_ref));
    } else {
        // Default behavior: Try with opus transcoding FIRST to ensure WebRTC-compatible audio
        src_variants.push(format!("{}#audio=opus", path_ref));
        // Fallback to direct stream
        src_variants.push(path.clone());
    }

    for base in base_urls {
        let normalized = base.trim_end_matches('/');
        if normalized.is_empty() {
            continue;
        }

        for variant in &src_variants {
            let encoded = urlencoding::encode(variant);
            let query_candidates = [
                format!("{}/api/webrtc?src={}", normalized, encoded),
                format!("{}/api/webrtc?src={}&dst=whep", normalized, encoded),
                format!("{}/api/webrtc?dst=whep&src={}", normalized, encoded),
            ];

            for candidate in query_candidates {
                if seen.insert(candidate.clone()) {
                    endpoints.push(candidate);
                }
            }
        }
    }

    if endpoints.is_empty() {
        // Try without audio modifier first (camera may not have audio or may already provide Opus)
        let encoded = urlencoding::encode(path_ref);
        let fallback = format!("{}/api/webrtc?src={}", GO2RTC_DEFAULT_API, encoded);
        println!(
            "[whep_play] No endpoints discovered; using default loopback WHEP endpoint {}",
            fallback
        );
        endpoints.push(fallback);
    }

    let client = Client::builder()
        .timeout(Duration::from_secs(10))
        .build()
        .map_err(|e| format!("Failed to create HTTP client: {}", e))?;

    let mut last_error: Option<String> = None;

    for endpoint in endpoints {
        println!("[whep_play] Posting SDP offer to {}", endpoint);
        let mut request = client
            .post(&endpoint)
            .header("Content-Type", "application/sdp")
            .header("Accept", "application/sdp")
            .body(offer_sdp.clone());

        if let Some(session) = rtsp_session.as_ref() {
            request = request.header("X-RTSP-Session", session);
        }
        if let Some(transport) = rtsp_transport.as_ref() {
            request = request.header("X-RTSP-Transport", transport);
        }

        match request.send().await {
            Ok(response) => {
                if response.status().is_success() {
                    let answer = response
                        .text()
                        .await
                        .map_err(|e| format!("Failed to read SDP answer: {}", e))?;
                    println!("[whep_play] Received SDP answer ({} bytes)", answer.len());
                    return Ok(answer);
                }

                let status = response.status();
                let body = response
                    .text()
                    .await
                    .unwrap_or_else(|_| String::from("<no body>"));
                let msg = format!(
                    "Endpoint {} responded with status {} and body {}",
                    endpoint, status, body
                );
                println!("[whep_play] {}", msg);
                last_error = Some(msg);
            }
            Err(err) => {
                let msg = format!("Failed to send WHEP offer to {}: {}", endpoint, err);
                println!("[whep_play] {}", msg);
                last_error = Some(msg);
            }
        }
    }

    Err(last_error.unwrap_or_else(|| "No WHEP endpoint accepted the offer".into()))
}

fn camera_rtsp_source(camera: &camera_store::Camera, variant: Option<u8>) -> Option<String> {
    let mut candidates: Vec<String> = Vec::new();

    match variant {
        Some(1) => {
            if !camera.path_sd.trim().is_empty() {
                candidates.push(camera.path_sd.trim().to_string());
            }
            if !camera.path_hd.trim().is_empty() {
                candidates.push(camera.path_hd.trim().to_string());
            }
        }
        Some(0) | None => {
            if !camera.path_hd.trim().is_empty() {
                candidates.push(camera.path_hd.trim().to_string());
            }
            if !camera.path_sd.trim().is_empty() {
                candidates.push(camera.path_sd.trim().to_string());
            }
        }
        Some(_) => {
            if !camera.path_hd.trim().is_empty() {
                candidates.push(camera.path_hd.trim().to_string());
            }
            if !camera.path_sd.trim().is_empty() {
                candidates.push(camera.path_sd.trim().to_string());
            }
        }
    }

    if let Some(stream) = camera.stream_url.as_ref() {
        let trimmed = stream.trim();
        if !trimmed.is_empty() {
            candidates.push(trimmed.to_string());
        }
    }

    for raw in &candidates {
        let trimmed = raw.trim();
        if trimmed.is_empty() {
            continue;
        }
        if trimmed.to_ascii_lowercase().starts_with("rtsp://") {
            return Some(trimmed.to_string());
        }
    }

    if let Some(base) = build_rtsp_base(camera) {
        if let Some(raw) = candidates.into_iter().find(|value| {
            let trimmed = value.trim();
            !trimmed.is_empty()
        }) {
            let trimmed = raw.trim();
            if !trimmed.is_empty() {
                if trimmed.contains("://") {
                    return Some(trimmed.to_string());
                }

                let suffix = if trimmed.starts_with('/') {
                    trimmed.to_string()
                } else {
                    format!("/{}", trimmed)
                };
                return Some(format!("{}{}", base, suffix));
            }
        }

        let suffix = camera_default_suffix(variant);
        if suffix.contains("://") {
            return Some(suffix.to_string());
        }

        return Some(format!("{}{}", base, suffix));
    }

    None
}

fn resolve_camera_store_source(stream_name: &str) -> Option<String> {
    let (camera_id, variant) = parse_camera_stream_hint(stream_name)?;
    let cameras = camera_store::load_cameras().ok()?;
    let camera = cameras.into_iter().find(|cam| cam.id == camera_id)?;
    camera_rtsp_source(&camera, variant)
}

#[tauri::command]
async fn resolve_stream_source(
    path: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<Option<String>, String> {
    let stream_name = sanitize_stream_key(&path);

    let mut resolved: Option<String> = match go2rtc_state.lock() {
        Ok(mut guard) => {
            if let Err(err) = ensure_go2rtc_files(&mut guard) {
                println!(
                    "[resolve_stream_source] Failed to ensure go2rtc files: {}",
                    err
                );
                None
            } else {
                match load_go2rtc_config(&guard) {
                    Ok(config) => extract_go2rtc_source(&config, &stream_name),
                    Err(err) => {
                        println!(
                            "[resolve_stream_source] Failed to load go2rtc config: {}",
                            err
                        );
                        None
                    }
                }
            }
        }
        Err(err) => {
            println!(
                "[resolve_stream_source] Failed to lock go2rtc state: {}",
                err
            );
            None
        }
    };

    if resolved.is_none() {
        if let Some(fallback) = resolve_camera_store_source(&stream_name) {
            println!(
                "[resolve_stream_source] Using camera store fallback for '{}' -> {}",
                stream_name, fallback
            );
            resolved = Some(fallback);
        }
    }

    if resolved.is_none() && stream_name != path {
        if let Some(fallback) = resolve_camera_store_source(path.trim()) {
            println!(
                "[resolve_stream_source] Using camera store fallback for raw path '{}' -> {}",
                path.trim(),
                fallback
            );
            resolved = Some(fallback);
        }
    }

    if let Some(ref value) = resolved {
        println!(
            "[resolve_stream_source] Resolved '{}' (normalized '{}') -> {}",
            path, stream_name, value
        );
    } else {
        println!(
            "[resolve_stream_source] No source found for '{}' (normalized '{}')",
            path, stream_name
        );
    }

    Ok(resolved)
}

#[derive(Deserialize)]
struct RtspHandshakeRequest {
    url: String,
    username: Option<String>,
    password: Option<String>,
    transport: Option<String>,
    include_audio: Option<bool>,
    timeout_ms: Option<u64>,
}

#[derive(Serialize)]
struct RtspHandshakeTrack {
    control_uri: String,
    response_headers: HashMap<String, String>,
}

#[derive(Serialize)]
struct RtspHandshakeResponse {
    base_uri: String,
    session: Option<String>,
    sdp: Option<String>,
    video: RtspHandshakeTrack,
    audio: Option<RtspHandshakeTrack>,
    log: Vec<String>,
}

#[tauri::command]
async fn rtsp_handshake(request: RtspHandshakeRequest) -> Result<RtspHandshakeResponse, String> {
    use rtsp_client::{perform_handshake, RtspHandshakeParams, TransportProfile};

    let transport = match request
        .transport
        .as_deref()
        .map(|value| value.to_ascii_lowercase())
        .as_deref()
    {
        Some("udp") => TransportProfile::Udp,
        _ => TransportProfile::Tcp,
    };

    let include_audio = request.include_audio.unwrap_or(true);
    let timeout = request
        .timeout_ms
        .map(Duration::from_millis)
        .unwrap_or_else(|| Duration::from_millis(3000));

    let params = RtspHandshakeParams {
        url: request.url.clone(),
        username: request.username.clone(),
        password: request.password.clone(),
        transport,
        include_audio,
        timeout,
    };

    tauri::async_runtime::spawn_blocking(move || {
        perform_handshake(params)
            .map(|result| RtspHandshakeResponse {
                base_uri: result.base_uri,
                session: result.session,
                sdp: result.sdp,
                video: RtspHandshakeTrack {
                    control_uri: result.video.control_uri,
                    response_headers: result.video.response_headers,
                },
                audio: result.audio.map(|track| RtspHandshakeTrack {
                    control_uri: track.control_uri,
                    response_headers: track.response_headers,
                }),
                log: result.log,
            })
            .map_err(|err| err.to_string())
    })
    .await
    .map_err(|join_err| join_err.to_string())?
}

#[derive(Serialize)]
struct HevcProbeParameterSets {
    vps: Vec<String>,
    sps: Vec<String>,
    pps: Vec<String>,
}

#[derive(Serialize)]
struct HevcProbeResponse {
    rtsp_url: String,
    width: u32,
    height: u32,
    fps: Option<f32>,
    parameter_sets: HevcProbeParameterSets,
    annexb_header: String,
    handshake_log: Vec<String>,
}

#[cfg(feature = "hevc-export")]
#[tauri::command]
async fn probe_hevc_export(
    stream_path: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<HevcProbeResponse, String> {
    let trimmed = stream_path.trim();
    let rtsp_url = if trimmed.to_ascii_lowercase().starts_with("rtsp://") {
        trimmed.to_string()
    } else {
        resolve_stream_source(trimmed.to_string(), mediamtx_state, go2rtc_state)
            .await?
            .ok_or_else(|| format!("Unable to resolve stream source for '{}'", trimmed))?
    };

    let handshake_result = spawn_blocking({
        let url = rtsp_url.clone();
        move || {
            let params = rtsp_client::RtspHandshakeParams {
                url,
                username: None,
                password: None,
                transport: rtsp_client::TransportProfile::Tcp,
                include_audio: false,
                timeout: Duration::from_millis(4000),
            };
            rtsp_client::perform_handshake(params)
        }
    })
    .await
    .map_err(|err| format!("Failed to join RTSP handshake task: {}", err))?;
    let handshake_result =
        handshake_result.map_err(|err| format!("RTSP handshake failed: {}", err))?;

    let sdp = handshake_result
        .sdp
        .as_deref()
        .ok_or_else(|| "RTSP handshake did not return an SDP body".to_string())?;

    let descriptor = hevc_export::parse_hevc_sdp(sdp)
        .map_err(|err| format!("Failed to parse HEVC SDP: {}", err))?;

    let parameter_sets_ref = &descriptor.parameter_sets;
    let parameter_sets = HevcProbeParameterSets {
        vps: parameter_sets_ref
            .vps
            .iter()
            .map(|nal| BASE64_STANDARD.encode(nal))
            .collect(),
        sps: parameter_sets_ref
            .sps
            .iter()
            .map(|nal| BASE64_STANDARD.encode(nal))
            .collect(),
        pps: parameter_sets_ref
            .pps
            .iter()
            .map(|nal| BASE64_STANDARD.encode(nal))
            .collect(),
    };

    let annexb_header = hevc_export::annexb_parameter_sets(parameter_sets_ref);
    let annexb_header = BASE64_STANDARD.encode(&annexb_header);

    Ok(HevcProbeResponse {
        rtsp_url,
        width: descriptor.width,
        height: descriptor.height,
        fps: descriptor.fps,
        parameter_sets,
        annexb_header,
        handshake_log: handshake_result.log,
    })
}

#[cfg(not(feature = "hevc-export"))]
#[tauri::command]
async fn probe_hevc_export(_stream_path: String) -> Result<HevcProbeResponse, String> {
    Err("HEVC export support is disabled at build time".to_string())
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
struct WhepEndpointDescriptor {
    base: String,
    provider: String,
}

#[tauri::command]
async fn get_whep_endpoints(
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<Vec<WhepEndpointDescriptor>, String> {
    let settings = load_streaming_settings().await?;
    let provider_label = "go2rtc".to_string();
    let bases = load_go2rtc_api_bases(&go2rtc_state, &settings);

    let descriptors = bases
        .into_iter()
        .filter_map(|raw| {
            let normalized = raw.trim().trim_end_matches('/').to_string();
            if normalized.is_empty() {
                None
            } else {
                Some(WhepEndpointDescriptor {
                    base: normalized,
                    provider: provider_label.clone(),
                })
            }
        })
        .collect();

    Ok(descriptors)
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
            "restartOnConfigChange": true
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
    let config_dir = settings_root_dir();

    std::fs::create_dir_all(&config_dir)
        .map_err(|e| format!("Failed to create settings directory: {}", e))?;

    let config_file = config_dir.join("settings.json");

    let content = serde_json::to_string_pretty(&settings)
        .map_err(|e| format!("Failed to serialize settings: {}", e))?;

    std::fs::write(&config_file, &content)
        .map_err(|e| format!("Failed to write settings file: {}", e))?;

    // Keep writing to the legacy location so older builds or tools reading from it keep working.
    if let Ok(legacy_dir) = app_handle.path().app_config_dir() {
        if legacy_dir != config_dir {
            if let Err(err) = std::fs::create_dir_all(&legacy_dir) {
                eprintln!(
                    "[Settings] Failed to create legacy config directory {:?}: {}",
                    legacy_dir, err
                );
            } else {
                let legacy_file = legacy_dir.join("settings.json");
                if let Err(err) = std::fs::write(&legacy_file, &content) {
                    eprintln!(
                        "[Settings] Failed to write legacy settings file {:?}: {}",
                        legacy_file, err
                    );
                }
            }
        }
    }

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

        let bytes = BASE64_STANDARD
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

fn parse_video_codec_from_sdp(sdp: &str) -> Option<StreamVideoCodec> {
    for line in sdp.lines() {
        let trimmed = line.trim();
        if trimmed.is_empty() {
            continue;
        }

        let lower = trimmed.to_ascii_lowercase();
        if trimmed.starts_with("a=rtpmap:") || trimmed.starts_with("m=video") {
            if lower.contains("h265") || lower.contains("hevc") {
                return Some(StreamVideoCodec::H265);
            }
            if lower.contains("h264") || lower.contains("avc") {
                return Some(StreamVideoCodec::H264);
            }
        }

        if trimmed.starts_with("a=fmtp:") {
            if lower.contains("sprop-vps") || lower.contains("hev1") || lower.contains("hvc1") {
                return Some(StreamVideoCodec::H265);
            }
            if lower.contains("sprop-parameter-sets") {
                return Some(StreamVideoCodec::H264);
            }
        }
    }
    None
}

#[allow(dead_code)]
fn parse_audio_codec_from_sdp(sdp: &str) -> Option<StreamAudioCodec> {
    let mut in_audio_section = false;

    for line in sdp.lines() {
        let trimmed = line.trim();
        if trimmed.is_empty() {
            continue;
        }

        if let Some(media) = trimmed.strip_prefix("m=") {
            in_audio_section = media
                .split_whitespace()
                .next()
                .map(|token| token.eq_ignore_ascii_case("audio"))
                .unwrap_or(false);
            continue;
        }

        if !in_audio_section {
            continue;
        }

        if let Some(payload) = trimmed.strip_prefix("a=rtpmap:") {
            if let Some((_pt, codec_part)) = payload.split_once(' ') {
                let codec_token = codec_part
                    .split('/')
                    .next()
                    .unwrap_or("")
                    .to_ascii_lowercase();

                let detected = if codec_token.contains("opus") {
                    StreamAudioCodec::Opus
                } else if codec_token.contains("pcmu") || codec_token.contains("g711u") {
                    StreamAudioCodec::Pcmu
                } else if codec_token.contains("pcma") || codec_token.contains("g711a") {
                    StreamAudioCodec::Pcma
                } else if codec_token.contains("mpeg4")
                    || codec_token.contains("mp4a")
                    || codec_token.contains("aac")
                {
                    StreamAudioCodec::Aac
                } else {
                    StreamAudioCodec::Unknown
                };

                return Some(detected);
            }
        }
    }

    None
}

async fn fetch_rtsp_sdp(rtsp_url: &str, include_audio: bool) -> Option<String> {
    let handshake = spawn_blocking({
        let url = rtsp_url.to_string();
        move || {
            let params = rtsp_client::RtspHandshakeParams {
                url,
                username: None,
                password: None,
                transport: rtsp_client::TransportProfile::Tcp,
                include_audio,
                timeout: Duration::from_millis(2500),
            };
            rtsp_client::perform_handshake(params)
        }
    })
    .await;

    match handshake {
        Ok(Ok(result)) => result.sdp,
        Ok(Err(err)) => {
            println!(
                "Failed to probe RTSP stream {} for codec information: {}",
                rtsp_url, err
            );
            None
        }
        Err(err) => {
            println!("Failed to join RTSP probe task for {}: {}", rtsp_url, err);
            None
        }
    }
}

async fn detect_stream_codec(rtsp_url: &str) -> Option<StreamVideoCodec> {
    fetch_rtsp_sdp(rtsp_url, true)
        .await
        .as_deref()
        .and_then(parse_video_codec_from_sdp)
}

async fn decide_recording_pipeline(
    rtsp_url: &str,
    hw_config: &ffmpeg::HwAccelConfig,
) -> RecordingVideoPipeline {
    match detect_stream_codec(rtsp_url).await {
        Some(StreamVideoCodec::H265) => {
            println!(
                "Detected H.265 at {}; using copy pipeline for recording",
                rtsp_url
            );
            RecordingVideoPipeline::Copy
        }
        Some(codec) => {
            println!(
                "Detected {} at {}; using transcode pipeline",
                codec.label(),
                rtsp_url
            );
            RecordingVideoPipeline::Reencode(hw_config.clone())
        }
        None => {
            println!(
                "Could not determine video codec for {}; defaulting to transcode pipeline",
                rtsp_url
            );
            RecordingVideoPipeline::Reencode(hw_config.clone())
        }
    }
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
        rtsp_url,
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
    let target_rtsp_url = rtsp_url
        .filter(|value| !value.trim().is_empty())
        .unwrap_or_else(|| format!("rtsp://127.0.0.1:8554/{}", stream_key));

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

    let ffmpeg_cmd = resolve_ffmpeg_command();
    let hw_decision = ffmpeg::determine_hw_accel_strategy(&ffmpeg_cmd, &hw_preference);
    println!(
        "Recording hardware acceleration choice: {} (codec: {})",
        hw_decision.message, hw_decision.config.video_codec
    );

    let video_pipeline = decide_recording_pipeline(&target_rtsp_url, &hw_decision.config).await;
    println!(
        "Recording video pipeline for {}: {}",
        target_rtsp_url,
        video_pipeline.description()
    );

    let mut recordings_state = state
        .lock()
        .map_err(|_| "Failed to lock recordings state".to_string())?;

    if recordings_state.active_recordings.contains_key(&stream_key) {
        return Err("Recording already in progress for this stream".to_string());
    }

    let ffmpeg_args = build_segment_ffmpeg_args(
        &target_rtsp_url,
        segment_duration,
        &first_segment_file,
        &video_pipeline,
    );
    println!("FFmpeg args for recording: {:?}", ffmpeg_args);

    let mut cmd = StdCommand::new(&ffmpeg_cmd);
    cmd.args(&ffmpeg_args)
        .stdin(Stdio::piped())
        .stdout(Stdio::null())
        .stderr(Stdio::null());

    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        const DETACHED_PROCESS: u32 = 0x00000008;
        cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
    }

    match cmd.spawn() {
        Ok(mut child) => {
            let stdin = child.stdin.take();
            let recording = RecordingProcess {
                child,
                stdin,
                camera_name: camera_label.clone(),
                stream_path: stream_key.clone(),
                quality_label: quality_label.clone(),
                output_file: first_segment_file,
                segment_duration,
                current_segment: 1,
                recordings_dir: recordings_dir.clone(),
                rtsp_url: target_rtsp_url.clone(),
                video_pipeline: video_pipeline.clone(),
            };

            recordings_state
                .active_recordings
                .insert(stream_key.clone(), recording);

            let state_clone = state.inner().clone();
            let stream_key_clone = stream_key.clone();
            let segment_handle = tokio::spawn(async move {
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
        gracefully_stop_recording_process(&mut recording, "manual stop");

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
    pipeline: &RecordingVideoPipeline,
) -> Vec<String> {
    let mut args: Vec<String> = Vec::new();

    if let RecordingVideoPipeline::Reencode(hw_config) = pipeline {
        args.extend(hw_config.pre_input.iter().cloned());
    }

    args.push("-rtsp_transport".into());
    args.push("tcp".into());
    args.push("-i".into());
    args.push(rtsp_url.to_string());

    match pipeline {
        RecordingVideoPipeline::Copy => {
            args.push("-c".into());
            args.push("copy".into());
        }
        RecordingVideoPipeline::Reencode(hw_config) => {
            args.push("-c:v".into());
            args.push(hw_config.video_codec.clone());
            args.extend(hw_config.video_args.clone());

            if !hw_config.video_args.iter().any(|arg| arg == "-preset") {
                args.push("-preset".into());
                args.push("fast".into());
            }

            if hw_config.video_codec == "libx264"
                && !hw_config.video_args.iter().any(|arg| arg == "-crf")
            {
                args.push("-crf".into());
                args.push("23".into());
            }

            args.push("-c:a".into());
            args.push("aac".into());
            args.push("-ar".into());
            args.push("48000".into());
            args.push("-ac".into());
            args.push("2".into());
        }
    }

    args.push("-movflags".into());
    // Use frag_keyframe+empty_moov to allow file to be playable even if not properly closed
    // and to avoid long "indexing" (writing moov atom at the end)
    args.push("+frag_keyframe+empty_moov+default_base_moof".into());
    args.push("-t".into());
    args.push(segment_duration.to_string());
    args.push("-y".into());
    args.push(output_path.to_string_lossy().to_string());

    args
}

fn wait_for_process_exit(child: &mut StdChild, attempts: usize, delay_ms: u64) -> bool {
    for _ in 0..attempts {
        match child.try_wait() {
            Ok(Some(status)) => {
                println!("FFmpeg process exited with status: {}", status);
                return true;
            }
            Ok(None) => std::thread::sleep(Duration::from_millis(delay_ms)),
            Err(err) => {
                println!("Failed to poll FFmpeg process state: {}", err);
                break;
            }
        }
    }
    false
}

fn gracefully_stop_recording_process(recording: &mut RecordingProcess, context: &str) {
    let mut requested = false;

    if let Some(stdin) = recording.stdin.as_mut() {
        match stdin.write_all(b"q\n") {
            Ok(_) => {
                let _ = stdin.flush();
                requested = true;
                println!(
                    "Sent graceful stop signal to FFmpeg for {} ({})",
                    recording.stream_path, context
                );
            }
            Err(err) => {
                println!(
                    "Failed to write graceful stop signal for {}: {}",
                    recording.stream_path, err
                );
            }
        }
    }

    let exited_gracefully = if requested {
        wait_for_process_exit(&mut recording.child, 40, 50)
    } else {
        false
    };

    if !exited_gracefully {
        println!(
            "FFmpeg did not exit in time for {} ({}), forcing termination",
            recording.stream_path, context
        );
        if let Err(err) = recording.child.kill() {
            println!(
                "Warning: failed to kill FFmpeg process for {}: {}",
                recording.stream_path, err
            );
        }
    }

    if let Err(err) = recording.child.wait() {
        println!(
            "Warning: waiting for FFmpeg process completion failed for {}: {}",
            recording.stream_path, err
        );
    }

    recording.stdin = None;
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
    let mut recordings_state = state.lock().unwrap();

    if let Some(recording) = recordings_state.active_recordings.get_mut(stream_key) {
        let pipeline = recording.video_pipeline.clone();
        let rtsp_url = recording.rtsp_url.clone();
        let segment_duration = recording.segment_duration;
        let camera_name = recording.camera_name.clone();
        let stream_path = recording.stream_path.clone();
        let quality_label = recording.quality_label.clone();
        let recordings_dir = recording.recordings_dir.clone();

        // Stop current FFmpeg process
        gracefully_stop_recording_process(recording, "segment rotation");

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

        println!(
            "Recording pipeline for next segment {}: {}",
            recording.current_segment,
            pipeline.description()
        );

        // Start new FFmpeg process for next segment
        let ffmpeg_args =
            build_segment_ffmpeg_args(&rtsp_url, segment_duration, &new_segment_file, &pipeline);
        println!(
            "FFmpeg args for next recording segment {}: {:?}",
            recording.current_segment, ffmpeg_args
        );

        let ffmpeg_cmd = resolve_ffmpeg_command();
        let mut cmd = StdCommand::new(&ffmpeg_cmd);
        cmd.args(&ffmpeg_args)
            .stdin(Stdio::piped())
            .stdout(Stdio::null())
            .stderr(Stdio::null());

        #[cfg(windows)]
        {
            const CREATE_NO_WINDOW: u32 = 0x08000000;
            const DETACHED_PROCESS: u32 = 0x00000008;
            cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
        }

        match cmd.spawn() {
            Ok(mut child) => {
                recording.stdin = child.stdin.take();
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

    let entries = fs::read_dir(&recordings_dir)
        .map_err(|e| format!("Failed to read recordings directory: {}", e))?;

    let expected_date = parsed_date.format("%Y%m%d").to_string();
    let camera_prefix = sanitize_filename(&camera_name);
    let camera_prefix_lower = camera_prefix.to_lowercase();

    for entry in entries {
        let entry = entry.map_err(|e| format!("Failed to read directory entry: {}", e))?;
        let path = entry.path();

        if !path.is_file() {
            continue;
        }

        let filename = match path.file_name().and_then(|n| n.to_str()) {
            Some(name) if name.ends_with(".mp4") => name,
            _ => continue,
        };

        let filename_lower = filename.to_lowercase();

        let metadata =
            fs::metadata(&path).map_err(|e| format!("Failed to get file metadata: {}", e))?;

        let name_without_ext = filename.strip_suffix(".mp4").unwrap_or(filename);

        let (prefix, suffix) = match name_without_ext.rsplit_once("_seg") {
            Some(parts) => parts,
            None => continue,
        };

        let prefix_lower = prefix.to_lowercase();
        if !prefix_lower.starts_with(&camera_prefix_lower) {
            continue;
        }

        let mut remainder = &prefix[camera_prefix.len()..];
        remainder = remainder.strip_prefix('_').unwrap_or(remainder);

        let (stream_part, quality_part) = match remainder.rsplit_once('_') {
            Some(parts) => parts,
            None => continue,
        };

        let suffix = suffix.trim_start_matches('_');
        let mut suffix_parts = suffix.split('_');
        let segment_part = suffix_parts.next();
        let date_part = suffix_parts.next();
        let time_part = suffix_parts.next();

        let date_str = date_part.unwrap_or("");
        let time_str = time_part.unwrap_or("");

        if !date_str.eq_ignore_ascii_case(&expected_date)
            && !filename_lower.contains(&expected_date)
        {
            continue;
        }

        let start_time = if time_str.len() == 6 && time_str.chars().all(|c| c.is_ascii_digit()) {
            format!(
                "{}T{}:{}:{}Z",
                parsed_date.format("%Y-%m-%d"),
                &time_str[0..2],
                &time_str[2..4],
                &time_str[4..6]
            )
        } else {
            format!("{}T00:00:00Z", parsed_date.format("%Y-%m-%d"))
        };

        println!(
            "Recording match -> camera='{}' stream='{}' quality='{}' segment='{:?}' file='{}'",
            camera_prefix, stream_part, quality_part, segment_part, filename
        );

        recordings.push(RecordingInfo {
            filename: filename.to_string(),
            start_time,
            end_time: None,
            size: metadata.len(),
            duration: None,
        });
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
async fn open_camera_file_manager(
    host: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<(), String> {
    let host = host.trim().to_string();
    if host.is_empty() {
        return Err("host is required".to_string());
    }

    let username = username
        .map(|value| value.trim().to_string())
        .filter(|value| !value.is_empty())
        .unwrap_or_else(|| "root".to_string());

    let (password_opt, password_source) =
        resolve_password_with_store(&host, password_plain, password_enc)?;
    let password = password_opt.unwrap_or_default();

    println!(
        "[ssh] password resolved via {} (length={})",
        password_source.as_str(),
        password.chars().count()
    );

    if let Some(explicit) = port {
        if explicit != 0 && explicit != 22 {
            println!(
                "[ssh] ignoring configured port {} for {} (forcing port 22 for SSH)",
                explicit, host
            );
        }
    }
    let ssh_port: u16 = 22;

    spawn_blocking(move || {
        let executable = find_winscp_executable().ok_or_else(|| {
            "WinSCP executable not found. Please install WinSCP or add it to PATH.".to_string()
        })?;

        let mut auth = urlencoding::encode(&username).into_owned();
        if !password.is_empty() {
            let encoded_pass = urlencoding::encode(&password);
            auth.push(':');
            auth.push_str(&encoded_pass);
        }

        let mut url = format!("sftp://{}@{}", auth, host);
        if ssh_port != 22 {
            url.push(':');
            url.push_str(&ssh_port.to_string());
        }
        if !url.ends_with('/') {
            url.push('/');
        }

        let mut cmd = StdCommand::new(executable);
        cmd.arg(url);

        #[cfg(windows)]
        {
            const CREATE_NO_WINDOW: u32 = 0x08000000;
            const DETACHED_PROCESS: u32 = 0x00000008;
            cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
        }

        cmd.spawn()
            .map(|_| ())
            .map_err(|err| format!("Failed to launch WinSCP: {}", err))
    })
    .await
    .map_err(|err| format!("Failed to launch file manager task: {}", err))??;

    Ok(())
}

#[tauri::command]
async fn run_camera_ssh_command(
    host: String,
    command: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<SshCommandResult, String> {
    let host = host.trim().to_string();
    if host.is_empty() {
        return Err("host is required".to_string());
    }

    let command = command.trim().to_string();
    if command.is_empty() {
        return Err("command is required".to_string());
    }

    let username = username
        .map(|value| value.trim().to_string())
        .filter(|value| !value.is_empty())
        .unwrap_or_else(|| "root".to_string());

    let (password_opt, password_source) =
        resolve_password_with_store(&host, password_plain, password_enc)?;
    let password =
        password_opt.ok_or_else(|| "Camera password is required for SSH command".to_string())?;

    println!(
        "[ssh] password resolved via {} (length={})",
        password_source.as_str(),
        password.chars().count()
    );

    if let Some(explicit) = port {
        if explicit != 0 && explicit != 22 {
            println!(
                "[ssh] ignoring configured port {} for {} (forcing port 22 for SSH)",
                explicit, host
            );
        }
    }
    let port = 22;
    let connect_target = format!("{}:{}", host, port);

    let result = spawn_blocking(move || -> Result<SshCommandResult, String> {
        let tcp = TcpStream::connect(&connect_target)
            .map_err(|err| format!("Failed to connect to {}: {}", connect_target, err))?;
        let _ = tcp.set_read_timeout(Some(Duration::from_secs(10)));
        let _ = tcp.set_write_timeout(Some(Duration::from_secs(10)));

        let mut session =
            Session::new().map_err(|err| format!("Failed to create SSH session: {}", err))?;
        session.set_tcp_stream(tcp);
        session.set_timeout(10_000);
        session
            .handshake()
            .map_err(|err| format!("SSH handshake failed: {}", err))?;

        session
            .userauth_password(&username, &password)
            .map_err(|err| format!("SSH authentication failed: {}", err))?;

        if !session.authenticated() {
            return Err("SSH authentication failed".to_string());
        }

        let mut channel = session
            .channel_session()
            .map_err(|err| format!("Failed to create SSH channel: {}", err))?;
        channel
            .exec(&command)
            .map_err(|err| format!("Failed to execute command: {}", err))?;

        let mut stdout = String::new();
        channel
            .read_to_string(&mut stdout)
            .map_err(|err| format!("Failed to read stdout: {}", err))?;

        let mut stderr = String::new();
        channel
            .stderr()
            .read_to_string(&mut stderr)
            .map_err(|err| format!("Failed to read stderr: {}", err))?;

        channel
            .wait_close()
            .map_err(|err| format!("Failed to close SSH channel: {}", err))?;

        let exit_status = channel.exit_status().unwrap_or(-1);

        Ok(SshCommandResult {
            stdout,
            stderr,
            exit_status,
        })
    })
    .await
    .map_err(|err| format!("SSH task join error: {}", err))??;

    Ok(result)
}

#[tauri::command]
async fn camera_ssh_shell_open(
    host: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
    shell_state: State<'_, Arc<StdMutex<SshShellManager>>>,
) -> Result<SshShellOpenResponse, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let username = username
        .map(|value| {
            let trimmed = value.trim().to_string();
            if trimmed.is_empty() {
                None
            } else {
                Some(trimmed)
            }
        })
        .flatten();

    let password_enc = password_enc.filter(|value| !value.trim().is_empty());
    let password_plain = password_plain.filter(|value| !value.trim().is_empty());

    let shell_state = Arc::clone(shell_state.inner());

    let response = spawn_blocking(move || -> Result<SshShellOpenResponse, String> {
        let (shell, mut welcome) = SshShellSession::establish(
            &host_trimmed,
            port,
            username,
            password_plain,
            password_enc,
        )?;

        if welcome.is_empty() {
            welcome = "Connected.".to_string();
        }

        let session_id: String = rand::thread_rng()
            .sample_iter(&Alphanumeric)
            .take(24)
            .map(char::from)
            .collect();

        let mut guard = shell_state
            .lock()
            .map_err(|_| "SSH shell state poisoned".to_string())?;
        guard.insert(session_id.clone(), shell);

        Ok(SshShellOpenResponse {
            session_id,
            output: welcome,
        })
    })
    .await
    .map_err(|err| format!("SSH shell task join error: {}", err))?;

    response
}

#[tauri::command]
async fn camera_ssh_shell_send(
    session_id: String,
    command: String,
    shell_state: State<'_, Arc<StdMutex<SshShellManager>>>,
) -> Result<SshShellDataResponse, String> {
    if session_id.trim().is_empty() {
        return Err("sessionId is required".to_string());
    }

    let shell_state = Arc::clone(shell_state.inner());

    let response = spawn_blocking(move || -> Result<SshShellDataResponse, String> {
        let mut guard = shell_state
            .lock()
            .map_err(|_| "SSH shell state poisoned".to_string())?;
        let session = guard
            .get_mut(session_id.trim())
            .ok_or_else(|| "SSH session not found".to_string())?;

        let output = session.send_command(&command)?;
        Ok(SshShellDataResponse { output })
    })
    .await
    .map_err(|err| format!("SSH shell task join error: {}", err))?;

    response
}

#[tauri::command]
async fn camera_ssh_shell_close(
    session_id: String,
    shell_state: State<'_, Arc<StdMutex<SshShellManager>>>,
) -> Result<bool, String> {
    if session_id.trim().is_empty() {
        return Ok(false);
    }

    let shell_state = Arc::clone(shell_state.inner());

    let response = spawn_blocking(move || -> Result<bool, String> {
        let mut guard = shell_state
            .lock()
            .map_err(|_| "SSH shell state poisoned".to_string())?;
        if let Some(mut session) = guard.remove(session_id.trim()) {
            session.close();
            Ok(true)
        } else {
            Ok(false)
        }
    })
    .await
    .map_err(|err| format!("SSH shell task join error: {}", err))?;

    response
}

#[tauri::command]
async fn local_fs_list(path: Option<String>) -> Result<Vec<LocalFsEntry>, String> {
    let provided = path.unwrap_or_default();
    let trimmed = provided.trim().to_string();

    spawn_blocking(move || -> Result<Vec<LocalFsEntry>, String> {
        let target_path = if trimmed.is_empty() {
            env::current_dir()
                .map_err(|err| format!("Failed to resolve current directory: {}", err))?
        } else {
            PathBuf::from(&trimmed)
        };

        if !target_path.exists() {
            return Err(format!(
                "Directory not found: {}",
                target_path.to_string_lossy()
            ));
        }

        if !target_path.is_dir() {
            return Err(format!(
                "Not a directory: {}",
                target_path.to_string_lossy()
            ));
        }

        let mut entries: Vec<LocalFsEntry> = Vec::new();
        let iterator = fs::read_dir(&target_path).map_err(|err| {
            format!(
                "Failed to read directory {}: {}",
                target_path.to_string_lossy(),
                err
            )
        })?;

        for item in iterator {
            let entry = match item {
                Ok(value) => value,
                Err(err) => {
                    println!("[local-fs] Skipping entry due to error: {}", err);
                    continue;
                }
            };

            let entry_path = entry.path();
            let path_string = entry_path
                .to_str()
                .map(|s| s.to_string())
                .unwrap_or_else(|| entry_path.to_string_lossy().to_string());

            let name = entry_path
                .file_name()
                .and_then(|n| n.to_str())
                .map(|s| s.to_string())
                .unwrap_or_else(|| path_string.clone());

            if name.is_empty() {
                continue;
            }

            let file_type = entry.file_type().ok();
            let metadata = entry.metadata().ok();

            let is_dir_flag = metadata
                .as_ref()
                .map(|meta| meta.is_dir())
                .or_else(|| file_type.map(|ft| ft.is_dir()))
                .unwrap_or(false);

            let size_value = metadata
                .as_ref()
                .filter(|meta| meta.is_file())
                .map(|meta| meta.len())
                .unwrap_or(0);

            let modified_value = metadata
                .as_ref()
                .and_then(|meta| meta.modified().ok())
                .and_then(|mtime| mtime.duration_since(UNIX_EPOCH).ok())
                .map(|duration| duration.as_secs() as i64);

            entries.push(LocalFsEntry {
                name,
                path: path_string,
                is_dir: is_dir_flag,
                size: size_value,
                modified: modified_value,
            });
        }

        entries.sort_by(|a, b| match (a.is_dir, b.is_dir) {
            (true, false) => Ordering::Less,
            (false, true) => Ordering::Greater,
            _ => a.name.to_lowercase().cmp(&b.name.to_lowercase()),
        });

        Ok(entries)
    })
    .await
    .map_err(|err| format!("Local FS list task join error: {}", err))?
}

#[tauri::command]
async fn local_fs_ensure_dir(path: String) -> Result<bool, String> {
    let trimmed = path.trim().to_string();
    if trimmed.is_empty() {
        return Err("path is required".to_string());
    }

    let target_path = PathBuf::from(&trimmed);

    spawn_blocking(move || -> Result<bool, String> {
        fs::create_dir_all(&target_path)
            .map_err(|err| format!("Failed to create directory {}: {}", trimmed, err))?;
        Ok(true)
    })
    .await
    .map_err(|err| format!("Local ensure dir task join error: {}", err))?
}

#[tauri::command]
async fn local_fs_delete(path: String) -> Result<bool, String> {
    let trimmed = path.trim().to_string();
    if trimmed.is_empty() {
        return Err("path is required".to_string());
    }

    let target_path = PathBuf::from(&trimmed);
    if !target_path.exists() {
        return Err(format!("Path not found: {}", trimmed));
    }

    if target_path.parent().is_none() {
        return Err("Refusing to delete root path".to_string());
    }

    spawn_blocking(move || -> Result<bool, String> {
        if target_path.is_dir() {
            fs::remove_dir_all(&target_path)
                .map_err(|err| format!("Failed to remove directory {}: {}", trimmed, err))?;
        } else {
            fs::remove_file(&target_path)
                .map_err(|err| format!("Failed to remove file {}: {}", trimmed, err))?;
        }
        Ok(true)
    })
    .await
    .map_err(|err| format!("Local delete task join error: {}", err))?
}

#[tauri::command]
async fn local_reveal_path(path: String) -> Result<bool, String> {
    let trimmed = path.trim().to_string();
    if trimmed.is_empty() {
        return Err("path is required".to_string());
    }

    let target_path = PathBuf::from(&trimmed);
    if !target_path.exists() {
        return Err(format!("Path not found: {}", trimmed));
    }

    let is_file = target_path.is_file();

    spawn_blocking(move || -> Result<bool, String> {
        #[cfg(target_os = "windows")]
        {
            let mut command = StdCommand::new("explorer.exe");
            if is_file {
                let arg = format!("/select,{}", target_path.display());
                command.arg(arg);
            } else {
                command.arg(target_path.clone());
            }
            const CREATE_NO_WINDOW: u32 = 0x08000000;
            const DETACHED_PROCESS: u32 = 0x00000008;
            command.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
            command
                .status()
                .map_err(|err| format!("Failed to open Explorer: {}", err))?;
            return Ok(true);
        }

        #[cfg(target_os = "macos")]
        {
            let mut command = if is_file {
                let mut cmd = StdCommand::new("open");
                cmd.arg("-R").arg(&target_path);
                cmd
            } else {
                let mut cmd = StdCommand::new("open");
                cmd.arg(&target_path);
                cmd
            };
            command
                .status()
                .map_err(|err| format!("Failed to reveal path: {}", err))?;
            return Ok(true);
        }

        #[cfg(target_os = "linux")]
        {
            let target_to_open = if is_file {
                target_path
                    .parent()
                    .map(|p| p.to_path_buf())
                    .unwrap_or_else(|| target_path.clone())
            } else {
                target_path.clone()
            };
            StdCommand::new("xdg-open")
                .arg(target_to_open)
                .status()
                .map_err(|err| format!("Failed to open file manager: {}", err))?;
            return Ok(true);
        }

        #[allow(unreachable_code)]
        {
            Err("Reveal operation is not supported on this platform".to_string())
        }
    })
    .await
    .map_err(|err| format!("Reveal path task join error: {}", err))?
}

#[tauri::command]
async fn camera_scp_list(
    host: String,
    path: Option<String>,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<Vec<SftpEntry>, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let target_path = path
        .map(|p| p.trim().to_string())
        .filter(|p| !p.is_empty())
        .unwrap_or_else(|| "/".to_string());

    let host_clone = host_trimmed.clone();
    let path_clone = target_path.clone();
    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    println!(
        "[scp:list] request host={} port={} path={}",
        host_trimmed,
        port.unwrap_or(22),
        target_path
    );

    spawn_blocking(move || -> Result<Vec<SftpEntry>, String> {
        let session = build_ssh_session(
            &host_clone,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        println!(
            "[scp:list] SSH session established host={} port={:?} path={}",
            host_clone, port_value, path_clone
        );

        let escaped_path = shell_escape(&path_clone);
        let list_commands = [
            format!("cd {} && ls -lA --full-time", escaped_path),
            format!("cd {} && ls -lA", escaped_path),
            format!("ls -lA --full-time {}", escaped_path),
            format!("ls -lA {}", escaped_path),
        ];

        let mut last_err: Option<String> = None;
        let mut listing_output = String::new();

        for command in &list_commands {
            println!("[scp:list] executing '{}'", command);
            match run_remote_command(&session, command) {
                Ok(output) => {
                    println!(
                        "[scp:list] command '{}' succeeded ({} bytes)",
                        command,
                        output.len()
                    );
                    listing_output = output;
                    last_err = None;
                    break;
                }
                Err(err) => {
                    println!("[scp:list] command '{}' failed -> {}", command, err);
                    last_err = Some(err);
                }
            }
        }

        if listing_output.is_empty() {
            println!("[scp:list] no output received");
            return Err(last_err.unwrap_or_else(|| "Failed to list remote directory".to_string()));
        }

        println!(
            "[scp:list] parsing listing ({} lines)",
            listing_output.lines().count()
        );

        let mut entries: Vec<SftpEntry> = Vec::new();

        for line in listing_output.lines() {
            let trimmed = line.trim();
            if trimmed.is_empty() || trimmed == "." || trimmed == ".." {
                continue;
            }
            if trimmed.starts_with("total ") {
                continue;
            }

            let parts: Vec<&str> = trimmed.split_whitespace().collect();
            if parts.len() < 9 {
                continue;
            }

            let perms = parts[0];
            let size = parts[4].parse::<u64>().unwrap_or(0);
            let mut name_parts = parts[8..].join(" ");
            if let Some((left, _right)) = name_parts.split_once(" -> ") {
                name_parts = left.to_string();
            }
            let name = name_parts.trim().trim_end_matches('/').to_string();
            if name.is_empty() || name == "." || name == ".." {
                continue;
            }

            let is_dir = perms.starts_with('d');
            let permissions = permissions_from_ls(perms);

            entries.push(SftpEntry {
                name,
                is_dir,
                size,
                modified: None,
                permissions,
            });
        }

        entries.sort_by(|a, b| match (a.is_dir, b.is_dir) {
            (true, false) => Ordering::Less,
            (false, true) => Ordering::Greater,
            _ => a.name.to_lowercase().cmp(&b.name.to_lowercase()),
        });

        println!(
            "[scp:list] parsed {} entries for host={} path={}",
            entries.len(),
            host_clone,
            path_clone
        );

        Ok(entries)
    })
    .await
    .map_err(|err| format!("SCP list task join error: {}", err))?
}

#[tauri::command]
async fn camera_scp_download(
    host: String,
    remote_path: String,
    local_path: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<bool, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let remote_trimmed = remote_path.trim().to_string();
    if remote_trimmed.is_empty() {
        return Err("remotePath is required".to_string());
    }

    let local_trimmed = local_path.trim().to_string();
    if local_trimmed.is_empty() {
        return Err("localPath is required".to_string());
    }

    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    spawn_blocking(move || -> Result<bool, String> {
        if let Some(parent) = Path::new(&local_trimmed).parent() {
            if !parent.exists() {
                fs::create_dir_all(parent).map_err(|err| {
                    format!("Failed to create local directory {:?}: {}", parent, err)
                })?;
            }
        }

        let session = build_ssh_session(
            &host_trimmed,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        let (mut remote_file, _stat) =
            session
                .scp_recv(Path::new(&remote_trimmed))
                .map_err(|err| {
                    format!(
                        "Failed to open remote file {} via SCP: {}",
                        remote_trimmed, err
                    )
                })?;

        let mut local_file = fs::File::create(&local_trimmed)
            .map_err(|err| format!("Failed to create local file {}: {}", local_trimmed, err))?;

        std::io::copy(&mut remote_file, &mut local_file)
            .map_err(|err| format!("Failed to copy data: {}", err))?;

        let _ = remote_file.send_eof();
        let _ = remote_file.wait_eof();
        let _ = remote_file.close();
        let _ = remote_file.wait_close();

        Ok(true)
    })
    .await
    .map_err(|err| format!("SCP download task join error: {}", err))?
}

#[tauri::command]
async fn camera_scp_upload(
    host: String,
    local_path: String,
    remote_path: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<bool, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let remote_trimmed = remote_path.trim().to_string();
    if remote_trimmed.is_empty() {
        return Err("remotePath is required".to_string());
    }

    let local_trimmed = local_path.trim().to_string();
    if local_trimmed.is_empty() {
        return Err("localPath is required".to_string());
    }

    if !Path::new(&local_trimmed).is_file() {
        return Err(format!("Local file not found: {}", local_trimmed));
    }

    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    spawn_blocking(move || -> Result<bool, String> {
        let session = build_ssh_session(
            &host_trimmed,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        let metadata = fs::metadata(&local_trimmed).map_err(|err| {
            format!(
                "Failed to read local file metadata {}: {}",
                local_trimmed, err
            )
        })?;
        let file_size = metadata.len();

        let file_mode: i32 = if metadata.permissions().readonly() {
            0o444
        } else {
            0o644
        };

        #[cfg(unix)]
        {
            let mode = metadata.permissions().mode() & 0o777;
            if mode != 0 {
                file_mode = mode as i32;
            }
        }

        let mut remote_file = session
            .scp_send(Path::new(&remote_trimmed), file_mode, file_size, None)
            .map_err(|err| {
                format!(
                    "Failed to open remote file {} via SCP: {}",
                    remote_trimmed, err
                )
            })?;

        let mut local_file = fs::File::open(&local_trimmed)
            .map_err(|err| format!("Failed to open local file {}: {}", local_trimmed, err))?;

        std::io::copy(&mut local_file, &mut remote_file)
            .map_err(|err| format!("Failed to upload data: {}", err))?;

        let _ = remote_file.send_eof();
        let _ = remote_file.wait_eof();
        let _ = remote_file.close();
        let _ = remote_file.wait_close();

        Ok(true)
    })
    .await
    .map_err(|err| format!("SCP upload task join error: {}", err))?
}

#[tauri::command]
async fn camera_remote_delete(
    host: String,
    remote_path: String,
    is_dir: bool,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<bool, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let remote_trimmed = remote_path.trim().to_string();
    if remote_trimmed.is_empty() {
        return Err("remotePath is required".to_string());
    }
    if remote_trimmed == "/" {
        return Err("Refusing to delete root directory".to_string());
    }
    if !remote_trimmed.starts_with('/') {
        return Err("remotePath must be absolute".to_string());
    }

    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    spawn_blocking(move || -> Result<bool, String> {
        let session = build_ssh_session(
            &host_trimmed,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        let escaped_path = shell_escape(&remote_trimmed);
        let command = if is_dir {
            format!("rm -rf {}", escaped_path)
        } else {
            format!("rm -f {}", escaped_path)
        };

        run_remote_command(&session, &command)?;
        Ok(true)
    })
    .await
    .map_err(|err| format!("Remote delete task join error: {}", err))?
}

#[tauri::command]
async fn camera_sftp_list(
    host: String,
    path: Option<String>,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<Vec<SftpEntry>, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let target_path = path
        .map(|p| p.trim().to_string())
        .filter(|p| !p.is_empty())
        .unwrap_or_else(|| "/".to_string());

    let host_for_session = host_trimmed.clone();
    let path_for_session = target_path.clone();
    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    spawn_blocking(move || -> Result<Vec<SftpEntry>, String> {
        let session = build_ssh_session(
            &host_for_session,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;
        let sftp = session
            .sftp()
            .map_err(|err| format!("Failed to initialize SFTP: {}", err))?;

        let mut entries: Vec<SftpEntry> = Vec::new();

        let dir_entries = sftp
            .readdir(Path::new(&path_for_session))
            .map_err(|err| format!("Failed to read directory {}: {}", path_for_session, err))?;

        for (item_path, stat) in dir_entries {
            let name = match item_path.file_name().and_then(|n| n.to_str()) {
                Some(value) if !value.is_empty() => value.to_string(),
                _ => continue,
            };

            if name == "." || name == ".." {
                continue;
            }

            let is_dir = is_directory_from_perm(stat.perm);
            let entry = SftpEntry {
                name,
                is_dir,
                size: stat.size.unwrap_or(0),
                modified: stat.mtime.map(|v| v as i64),
                permissions: stat.perm,
            };
            entries.push(entry);
        }

        entries.sort_by(|a, b| match (a.is_dir, b.is_dir) {
            (true, false) => Ordering::Less,
            (false, true) => Ordering::Greater,
            _ => a.name.to_lowercase().cmp(&b.name.to_lowercase()),
        });

        Ok(entries)
    })
    .await
    .map_err(|err| format!("SFTP list task join error: {}", err))?
}

#[tauri::command]
async fn camera_sftp_download(
    host: String,
    remote_path: String,
    local_path: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<bool, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let remote_trimmed = remote_path.trim().to_string();
    if remote_trimmed.is_empty() {
        return Err("remotePath is required".to_string());
    }

    let local_trimmed = local_path.trim().to_string();
    if local_trimmed.is_empty() {
        return Err("localPath is required".to_string());
    }

    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    spawn_blocking(move || {
        if let Some(parent) = Path::new(&local_trimmed).parent() {
            if !parent.exists() {
                fs::create_dir_all(parent).map_err(|err| {
                    format!("Failed to create local directory {:?}: {}", parent, err)
                })?;
            }
        }

        let session = build_ssh_session(
            &host_trimmed,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        let sftp = session
            .sftp()
            .map_err(|err| format!("Failed to initialize SFTP: {}", err))?;

        let mut remote_file = sftp
            .open(Path::new(&remote_trimmed))
            .map_err(|err| format!("Failed to open remote file {}: {}", remote_trimmed, err))?;
        let mut local_file = fs::File::create(&local_trimmed)
            .map_err(|err| format!("Failed to create local file {}: {}", local_trimmed, err))?;

        std::io::copy(&mut remote_file, &mut local_file)
            .map_err(|err| format!("Failed to copy data: {}", err))?;

        Ok(true)
    })
    .await
    .map_err(|err| format!("SFTP download task join error: {}", err))?
}

#[tauri::command]
async fn camera_sftp_upload(
    host: String,
    local_path: String,
    remote_path: String,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
) -> Result<bool, String> {
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let remote_trimmed = remote_path.trim().to_string();
    if remote_trimmed.is_empty() {
        return Err("remotePath is required".to_string());
    }

    let local_trimmed = local_path.trim().to_string();
    if local_trimmed.is_empty() {
        return Err("localPath is required".to_string());
    }

    if !Path::new(&local_trimmed).is_file() {
        return Err(format!("Local file not found: {}", local_trimmed));
    }

    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    spawn_blocking(move || {
        let session = build_ssh_session(
            &host_trimmed,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        let sftp = session
            .sftp()
            .map_err(|err| format!("Failed to initialize SFTP: {}", err))?;

        let mut remote_file = sftp
            .create(Path::new(&remote_trimmed))
            .map_err(|err| format!("Failed to create remote file {}: {}", remote_trimmed, err))?;
        let mut local_file = fs::File::open(&local_trimmed)
            .map_err(|err| format!("Failed to open local file {}: {}", local_trimmed, err))?;

        std::io::copy(&mut local_file, &mut remote_file)
            .map_err(|err| format!("Failed to upload data: {}", err))?;

        Ok(true)
    })
    .await
    .map_err(|err| format!("SFTP upload task join error: {}", err))?
}

#[tauri::command]
async fn camera_collect_recent_files(
    host: String,
    base_path: Option<String>,
    username: Option<String>,
    password_enc: Option<String>,
    password_plain: Option<String>,
    port: Option<u16>,
    window_seconds: Option<u64>,
    extra_margin_seconds: Option<u64>,
    max_files: Option<u32>,
    max_total_bytes: Option<u64>,
) -> Result<Vec<RemoteRecentFile>, String> {
    // Traverse the camera recordings directory and collect files that were modified recently.
    // The frontend applies additional filters (time window, limits) before downloading files.
    let host_trimmed = host.trim().to_string();
    if host_trimmed.is_empty() {
        return Err("host is required".to_string());
    }

    let mut normalized_base = base_path
        .unwrap_or_else(|| "/".to_string())
        .trim()
        .to_string();

    if normalized_base.is_empty() {
        normalized_base = "/".to_string();
    }

    if normalized_base != "/" {
        normalized_base = format!("/{}", normalized_base.trim_start_matches('/'));
        normalized_base = normalized_base.trim_end_matches('/').to_string();
    }

    let username_clone = username.clone();
    let password_enc_clone = password_enc.clone();
    let password_plain_clone = password_plain.clone();
    let port_value = port;

    let effective_window = window_seconds
        .unwrap_or(0)
        .saturating_add(extra_margin_seconds.unwrap_or(0));

    spawn_blocking(move || -> Result<Vec<RemoteRecentFile>, String> {
        let session = build_ssh_session(
            &host_trimmed,
            port_value,
            username_clone,
            password_plain_clone,
            password_enc_clone,
        )?;

        let sftp = session
            .sftp()
            .map_err(|err| format!("Failed to initialize SFTP: {}", err))?;

        let base_path_buf = if normalized_base == "/" {
            PathBuf::from("/")
        } else {
            PathBuf::from(&normalized_base)
        };

        let mut queue: VecDeque<PathBuf> = VecDeque::new();
        queue.push_back(base_path_buf.clone());

        let mut visited: HashSet<String> = HashSet::new();

        let now_secs = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|duration| duration.as_secs() as i64)
            .unwrap_or(0);

        let effective_window_i64 = if effective_window == 0 {
            0i64
        } else if effective_window >= i64::MAX as u64 {
            i64::MAX
        } else {
            effective_window as i64
        };

        let threshold = if effective_window_i64 > 0 {
            now_secs.saturating_sub(effective_window_i64)
        } else {
            i64::MIN
        };

        let mut candidates: Vec<RemoteRecentFile> = Vec::new();

        while let Some(current_dir) = queue.pop_front() {
            let dir_string = current_dir.to_string_lossy().to_string();
            if !visited.insert(dir_string.clone()) {
                continue;
            }

            let entries = match sftp.readdir(&current_dir) {
                Ok(value) => value,
                Err(err) => {
                    println!(
                        "[sftp:recent] failed to read dir {} -> {}",
                        current_dir.display(),
                        err
                    );
                    continue;
                }
            };

            for (entry_path, stat) in entries {
                let name = match entry_path.file_name().and_then(|n| n.to_str()) {
                    Some(value) if !value.is_empty() => value.to_string(),
                    _ => continue,
                };

                if name == "." || name == ".." {
                    continue;
                }

                let is_dir = is_directory_from_perm(stat.perm);
                if is_dir {
                    queue.push_back(entry_path.clone());
                    continue;
                }

                let modified = stat.mtime.map(|v| v as i64);
                if let Some(modified_ts) = modified {
                    if threshold != i64::MIN && modified_ts < threshold {
                        continue;
                    }

                    let absolute_path = entry_path.to_string_lossy().to_string();
                    let relative_path = entry_path
                        .strip_prefix(&base_path_buf)
                        .ok()
                        .and_then(|p| p.to_str())
                        .map(|s| s.trim_start_matches('/').to_string())
                        .filter(|s| !s.is_empty())
                        .unwrap_or_else(|| name.clone());

                    let size = stat.size.unwrap_or(0);

                    candidates.push(RemoteRecentFile {
                        absolute_path,
                        relative_path,
                        size,
                        modified: modified_ts,
                    });
                }
            }
        }

        candidates.sort_by(|a, b| a.modified.cmp(&b.modified));

        let mut limited: Vec<RemoteRecentFile> = Vec::new();
        let mut accumulated_bytes: u64 = 0;

        for entry in candidates.into_iter() {
            if let Some(max_bytes) = max_total_bytes {
                if accumulated_bytes.saturating_add(entry.size) > max_bytes {
                    continue;
                }
            }

            accumulated_bytes = accumulated_bytes.saturating_add(entry.size);
            limited.push(RemoteRecentFile {
                absolute_path: entry.absolute_path,
                relative_path: entry.relative_path,
                size: entry.size,
                modified: entry.modified,
            });

            if let Some(limit) = max_files {
                if limited.len() >= limit as usize {
                    break;
                }
            }
        }

        Ok(limited)
    })
    .await
    .map_err(|err| format!("Collect recent files task join error: {}", err))?
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
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        const DETACHED_PROCESS: u32 = 0x00000008;
        cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
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
            let ffmpeg_cmd = resolve_ffmpeg_command();
            let mut cmd = StdCommand::new(&ffmpeg_cmd);
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
                const CREATE_NO_WINDOW: u32 = 0x08000000;
                const DETACHED_PROCESS: u32 = 0x00000008;
                cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
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
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        const DETACHED_PROCESS: u32 = 0x00000008;
        cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
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

#[allow(dead_code)]
#[tauri::command]
async fn check_mediamtx_status(
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<serde_json::Value, String> {
    let settings = load_streaming_settings().await?;
    match fetch_go2rtc_paths(&go2rtc_state, &settings).await {
        Ok(statuses) => Ok(serde_json::json!({
            "provider": "go2rtc",
            "items": statuses,
        })),
        Err(err) => Ok(serde_json::json!({
            "provider": "go2rtc",
            "items": [],
            "error": err,
        })),
    }
}

#[tauri::command]
async fn check_rtsp_stream(url: String) -> Result<bool, String> {
    println!("Checking RTSP stream availability: {}", url);

    // First check if ffmpeg is available
    let ffmpeg_path = resolve_ffmpeg_command();
    let mut cmd = tokio::process::Command::new(&ffmpeg_path);
    cmd.args(&["-version"])
        .stdout(std::process::Stdio::null())
        .stderr(std::process::Stdio::null());

    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        const DETACHED_PROCESS: u32 = 0x00000008;
        cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
    }

    match cmd.output().await {
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
    let mut ffmpeg_cmd = tokio::process::Command::new(&ffmpeg_path);
    ffmpeg_cmd
        .args(&["-rtsp_transport", "tcp", "-i", &url, "-f", "null", "-"])
        .stdout(std::process::Stdio::null())
        .stderr(std::process::Stdio::piped());

    #[cfg(windows)]
    {
        const CREATE_NO_WINDOW: u32 = 0x08000000;
        const DETACHED_PROCESS: u32 = 0x00000008;
        ffmpeg_cmd.creation_flags(CREATE_NO_WINDOW | DETACHED_PROCESS);
    }

    match tokio::time::timeout(std::time::Duration::from_secs(10), ffmpeg_cmd.output()).await {
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

#[tauri::command]
async fn check_go2rtc_stream_online(
    stream_name: String,
    go2rtc_state: State<'_, Arc<StdMutex<Go2RtcState>>>,
) -> Result<bool, String> {
    let settings = load_streaming_settings().await?;
    let statuses = fetch_go2rtc_paths(&go2rtc_state, &settings).await?;
    
    // Check if the stream exists in the list of active streams
    for item in statuses {
        if item.name == stream_name {
            return Ok(true);
        }
    }
    
    Ok(false)
}
