mod auth;
mod camera_store;
mod discovery;
mod ffmpeg;
#[cfg(feature = "hevc-export")]
mod hevc_export;
mod http_server;
mod onvif;
mod rtsp_client;
mod rtsp_utils;

use std::cmp::Ordering;
use std::collections::{BTreeSet, HashMap, HashSet};
use std::env;
use std::fs;
use std::io::{BufRead, BufReader, Read, Write};
use std::net::{TcpStream, ToSocketAddrs};
use std::path::{Path, PathBuf};
use std::process::{Child as StdChild, ChildStdin, Command as StdCommand, Stdio};
use std::sync::{Arc, Mutex as StdMutex};
use std::time::{Duration, Instant, UNIX_EPOCH};

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
const MEDIAMTX_BINARY_NAME: &str = "mediamtx.exe";
#[cfg(not(windows))]
const MEDIAMTX_BINARY_NAME: &str = "mediamtx";

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
        let mut session = build_ssh_session(host, port, username, password_plain, password_enc)?;

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
            welcome =
                shell.collect_output(Duration::from_secs(2), Duration::from_millis(100))?;
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

struct MediaMtxState {
    child: Option<StdChild>,
    mediamtx_dir: PathBuf,
    config_path: PathBuf,
    exe_path: PathBuf,
}

impl MediaMtxState {
    fn new(app_handle: &AppHandle) -> Self {
        let base_dir = app_handle.path().app_local_data_dir().unwrap_or_else(|_| {
            dirs_next::data_local_dir()
                .or_else(dirs_next::data_dir)
                .unwrap_or_else(|| env::current_dir().unwrap_or_else(|_| PathBuf::from(".")))
        });

        let mediamtx_dir = base_dir.join("mediamtx");
        if !mediamtx_dir.exists() {
            if let Err(err) = fs::create_dir_all(&mediamtx_dir) {
                println!(
                    "Failed to create MediaMTX directory {:?}: {}",
                    mediamtx_dir, err
                );
            }
        }

        let config_path = mediamtx_dir.join("mediamtx.yml");
        let exe_path = mediamtx_dir.join(MEDIAMTX_BINARY_NAME);

        if !exe_path.exists() {
            Self::prepare_binary(app_handle, &exe_path);
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

    fn prepare_binary(app_handle: &AppHandle, destination: &Path) {
        if destination.exists() {
            return;
        }

        let mut copied = false;

        if let Ok(resource_root) = app_handle.path().resource_dir() {
            for candidate in Self::resource_candidates() {
                let source = resource_root.join(candidate);
                if source.exists() {
                    match fs::copy(&source, destination) {
                        Ok(_) => {
                            println!(
                                "Copied MediaMTX binary from {:?} to {:?}",
                                source, destination
                            );
                            copied = true;
                            break;
                        }
                        Err(err) => {
                            println!("Failed to copy MediaMTX binary from {:?}: {}", source, err);
                        }
                    }
                }
            }
        }

        if !copied {
            if let Ok(current_dir) = env::current_dir() {
                let bundled = current_dir
                    .join("src-tauri")
                    .join("mediamtx")
                    .join(MEDIAMTX_BINARY_NAME);
                if bundled.exists() {
                    if let Err(err) = fs::copy(&bundled, destination) {
                        println!("Failed to copy MediaMTX binary from {:?}: {}", bundled, err);
                    } else {
                        println!(
                            "Copied MediaMTX binary from {:?} to {:?}",
                            bundled, destination
                        );
                        copied = true;
                    }
                }
            }
        }

        if !copied {
            println!(
                "MediaMTX binary not found in bundled resources; expected at {:?}",
                destination
            );
        }
    }

    #[cfg(windows)]
    fn resource_candidates() -> &'static [&'static str] {
        &[
            "mediamtx/mediamtx.exe",
            "binaries/mediamtx.exe",
            "binaries/mediamtx-x86_64-pc-windows-msvc.exe",
        ]
    }

    #[cfg(not(windows))]
    fn resource_candidates() -> &'static [&'static str] {
        &["mediamtx/mediamtx", "binaries/mediamtx"]
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
    fn description(&self) -> String {
        match self {
            LiveStreamPipeline::Direct { .. } => "direct source passthrough".into(),
            LiveStreamPipeline::Transcode { encoder, .. } => {
                format!("FFmpeg on-demand transcode ({})", encoder)
            }
        }
    }

    fn origin_url(&self) -> &str {
        match self {
            LiveStreamPipeline::Direct { source_url } => source_url,
            LiveStreamPipeline::Transcode { origin_url, .. } => origin_url,
        }
    }
}

async fn decide_live_pipeline(
    stream_name: &str,
    origin_url: &str,
    ffmpeg_cmd: &str,
    hw_config: &ffmpeg::HwAccelConfig,
) -> LiveStreamPipeline {
    let _ = (stream_name, ffmpeg_cmd, hw_config);
    LiveStreamPipeline::Direct {
        source_url: origin_url.trim().to_string(),
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

fn default_ssh_username(username: Option<String>) -> String {
    username
        .and_then(|u| if u.trim().is_empty() { None } else { Some(u) })
        .unwrap_or_else(|| "root".to_string())
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

    let (password_opt, password_source) = resolve_password_with_store(host, password_plain, password_enc)?;
    let password = password_opt.unwrap_or_default();

    println!(
        "[ssh] password resolved via {} (length={})",
        password_source.as_str(),
        password.chars().count()
    );
    let username = default_ssh_username(username);
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
            host_trimmed, candidate_port, username
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

        match session.userauth_password(&username, &password) {
            Ok(()) => {
                if session.authenticated() {
                    println!(
                        "[ssh] authentication succeeded for {}@{}:{}",
                        username, host_trimmed, candidate_port
                    );
                    return Ok(session);
                }
                println!(
                    "[ssh] authentication reported failure for {}@{}:{} despite Ok",
                    username, host_trimmed, candidate_port
                );
                return Err("SSH authentication failed".to_string());
            }
            Err(err) => {
                println!(
                    "[ssh] authentication failed for {}@{}:{} -> {}",
                    username, host_trimmed, candidate_port, err
                );
                if password.is_empty() {
                    return Err(format!(
                        "SSH authentication failed for {}@{}:{}: no password provided",
                        username, host_trimmed, candidate_port
                    ));
                }

                println!(
                    "[ssh] trying keyboard-interactive auth for {}@{}:{}",
                    username, host_trimmed, candidate_port
                );

                let mut prompt_handler = PasswordPrompt {
                    password: &password,
                };

                match session.userauth_keyboard_interactive(&username, &mut prompt_handler) {
                    Ok(()) => {
                        if session.authenticated() {
                            println!(
                                "[ssh] keyboard-interactive authentication succeeded for {}@{}:{}",
                                username, host_trimmed, candidate_port
                            );
                            return Ok(session);
                        }
                        println!(
                            "[ssh] keyboard-interactive authentication reported failure for {}@{}:{} despite Ok",
                            username, host_trimmed, candidate_port
                        );
                        return Err("SSH authentication failed".to_string());
                    }
                    Err(fallback_err) => {
                        println!(
                            "[ssh] keyboard-interactive authentication failed for {}@{}:{} -> {}",
                            username, host_trimmed, candidate_port, fallback_err
                        );
                        return Err(format!(
                            "SSH authentication failed for {}@{}:{}: {} (keyboard-interactive fallback: {})",
                            username, host_trimmed, candidate_port, err, fallback_err
                        ));
                    }
                }
            }
        }
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

            let mediamtx_state = Arc::new(StdMutex::new(MediaMtxState::new(&app_handle)));
            app.manage(mediamtx_state);

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
            resolve_stream_source,
            rtsp_handshake,
            probe_hevc_export,
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
            camera_remote_delete,
            local_fs_delete,
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
            }
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}

#[tauri::command]
async fn list_mediamtx_paths(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<Vec<StreamPathStatus>, String> {
    fetch_mediamtx_paths(&mediamtx_state).await
}

async fn fetch_mediamtx_paths(
    mediamtx_state: &State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<Vec<StreamPathStatus>, String> {
    let bases = load_mediamtx_api_bases(mediamtx_state);
    if bases.is_empty() {
        return Ok(Vec::new());
    }

    let client = Client::builder()
        .timeout(Duration::from_secs(3))
        .build()
        .map_err(|e| format!("Failed to build HTTP client: {}", e))?;

    let mut last_error: Option<String> = None;

    for base in bases {
        let trimmed = base.trim();
        if trimmed.is_empty() {
            continue;
        }

        let endpoints = [
            format!("{}/v2/paths/list", trimmed),
            format!("{}/v1/paths/list", trimmed),
            format!("{}/paths/list", trimmed),
        ];

        for endpoint in &endpoints {
            match client.get(endpoint).send().await {
                Ok(response) if response.status().is_success() => {
                    let payload: serde_json::Value = response
                        .json()
                        .await
                        .map_err(|e| format!("Failed to parse MediaMTX response: {}", e))?;
                    if let Some(items) = collect_mediamtx_paths(&payload) {
                        return Ok(items);
                    }
                }
                Ok(response) => {
                    last_error = Some(format!(
                        "MediaMTX endpoint {} returned status {}",
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

fn collect_mediamtx_paths(payload: &serde_json::Value) -> Option<Vec<StreamPathStatus>> {
    if let Some(items) = payload.get("items").and_then(|v| v.as_array()) {
        let mut result = Vec::new();
        for item in items {
            if let Some(entry) = map_mediamtx_path(item) {
                result.push(entry);
            }
        }
        return Some(result);
    }

    if let Some(item) = payload.get("item") {
        if let Some(entry) = map_mediamtx_path(item) {
            return Some(vec![entry]);
        }
    }

    if let Some(array) = payload.as_array() {
        let mut result = Vec::new();
        for item in array {
            if let Some(entry) = map_mediamtx_path(item) {
                result.push(entry);
            }
        }
        if !result.is_empty() {
            return Some(result);
        }
    }

    None
}

fn map_mediamtx_path(item: &serde_json::Value) -> Option<StreamPathStatus> {
    let name = item
        .get("name")
        .and_then(|v| v.as_str())
        .or_else(|| item.get("id").and_then(|v| v.as_str()))?
        .to_string();

    let ready = item
        .get("ready")
        .and_then(|v| v.as_bool())
        .or_else(|| item.get("sourceReady").and_then(|v| v.as_bool()))
        .unwrap_or(false);

    let mut protocols: BTreeSet<String> = BTreeSet::new();
    let mut reader_count = 0usize;

    if let Some(readers) = item.get("readers").and_then(|v| v.as_array()) {
        reader_count = readers.len();
        for reader in readers {
            if let Some(proto) = reader
                .get("protocol")
                .or_else(|| reader.get("type"))
                .and_then(|v| v.as_str())
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
        .and_then(|v| v.get("onDemand").or_else(|| v.get("on_demand")))
        .and_then(|v| v.as_bool())
        .or_else(|| {
            item.get("onDemand")
                .or_else(|| item.get("on_demand"))
                .and_then(|v| v.as_bool())
        })
        .unwrap_or(true);

    Some(StreamPathStatus {
        name,
        ready,
        reader_count,
        active_protocols: protocols.into_iter().collect(),
        publisher_kind,
        source_url,
        on_demand,
    })
}

fn load_mediamtx_api_bases(state: &State<'_, Arc<StdMutex<MediaMtxState>>>) -> Vec<String> {
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

    if let Some(entries) = yaml.get("apiAddresses").and_then(|v| v.as_sequence()) {
        for entry in entries.iter().filter_map(|v| v.as_str()) {
            if let Some(base) = parse_address_to_base(entry) {
                bases.insert(base);
            }
        }
    }

    if let Some(webrtc_addr) = yaml.get("webrtcAddress").and_then(|v| v.as_str()) {
        if let Some(base) = parse_address_to_base(webrtc_addr) {
            bases.insert(base);
        }
    }

    bases.into_iter().collect()
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

fn ensure_paths_mapping<'a>(root: &'a mut serde_yaml::Value) -> &'a mut serde_yaml::Mapping {
    if !root.is_mapping() {
        *root = serde_yaml::Value::Mapping(serde_yaml::Mapping::new());
    }
    root.as_mapping_mut().unwrap()
}

fn ensure_stream_path(
    map: &mut serde_yaml::Value,
    stream_name: &str,
    pipeline: &LiveStreamPipeline,
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
        }
        LiveStreamPipeline::Transcode {
            origin_url,
            command,
            encoder,
        } => {
            let mut stream_map = serde_yaml::Mapping::new();
            stream_map.insert(
                Value::String("originUrl".into()),
                Value::String(origin_url.to_string()),
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
            stream_map.insert(
                Value::String("videoEncoder".into()),
                Value::String(encoder.to_string()),
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

fn extract_mediamtx_source(config: &serde_yaml::Value, stream_name: &str) -> Option<String> {
    let paths = config.get("paths")?.as_mapping()?;
    let entry = paths.get(&serde_yaml::Value::String(stream_name.to_string()))?;
    let entry_map = entry.as_mapping()?;

    if let Some(source) = entry_map
        .get(&serde_yaml::Value::String("source".into()))
        .and_then(|v| v.as_str())
    {
        return Some(source.to_string());
    }

    entry_map
        .get(&serde_yaml::Value::String("originUrl".into()))
        .and_then(|v| v.as_str())
        .map(|s| s.to_string())
}

fn set_mediamtx_transport(
    config: &mut serde_yaml::Value,
    stream_name: &str,
    transport: &str,
) -> bool {
    let paths_value = match config.get_mut("paths") {
        Some(value) => value,
        None => return false,
    };
    let paths_map = match paths_value.as_mapping_mut() {
        Some(map) => map,
        None => return false,
    };

    let key = serde_yaml::Value::String(stream_name.to_string());
    let entry_value = match paths_map.get_mut(&key) {
        Some(value) => value,
        None => return false,
    };

    let entry_map = match entry_value.as_mapping_mut() {
        Some(map) => map,
        None => return false,
    };

    let transport_key = serde_yaml::Value::String("rtspTransport".into());
    let desired = serde_yaml::Value::String(transport.to_string());

    match entry_map.get(&transport_key) {
        Some(existing) if existing == &desired => false,
        _ => {
            entry_map.insert(transport_key, desired);
            true
        }
    }
}

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

fn spawn_mediamtx_process(state: &mut MediaMtxState) -> Result<(), String> {
    ensure_mediamtx_files(state)?;

    let mut cmd = StdCommand::new(&state.exe_path);
    cmd.arg(&state.config_path)
        .current_dir(&state.mediamtx_dir)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped());

    #[cfg(windows)]
    {
        cmd.creation_flags(0x08000000);
    }

    let mut child = cmd
        .spawn()
        .map_err(|e| format!("Failed to start mediamtx: {}", e))?;

    if let Some(stdout) = child.stdout.take() {
        std::thread::spawn(move || {
            let reader = BufReader::new(stdout);
            for line in reader.lines().flatten() {
                println!("[mediamtx][out] {}", line);
            }
        });
    }

    if let Some(stderr) = child.stderr.take() {
        std::thread::spawn(move || {
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

fn restart_if_running(state: &mut MediaMtxState) -> Result<(), String> {
    if state.child.is_some() {
        restart_mediamtx(state)?;
    }
    Ok(())
}

fn upsert_single_stream(
    state: &mut MediaMtxState,
    stream_name: &str,
    pipeline: &LiveStreamPipeline,
) -> Result<bool, String> {
    ensure_mediamtx_files(state)?;
    let mut config = load_mediamtx_config(state)?;
    let updated = ensure_stream_path(&mut config, stream_name, pipeline);
    if updated {
        save_mediamtx_config(state, &config)?;
    }
    Ok(updated)
}

#[tauri::command]
async fn mediamtx_start(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<String, String> {
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
    Ok("started".into())
}

#[tauri::command]
async fn mediamtx_stop(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<String, String> {
    let mut guard = mediamtx_state
        .lock()
        .map_err(|_| "Failed to lock MediaMTX state".to_string())?;

    if let Some(mut child) = guard.child.take() {
        let _ = child.kill();
        let _ = child.wait();
        Ok("stopped".into())
    } else {
        Ok("not running".into())
    }
}

#[tauri::command]
async fn add_camera_to_mediamtx(
    name: String,
    url: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<bool, String> {
    let stream_name = sanitize_stream_key(&name);
    let pipeline = LiveStreamPipeline::Direct {
        source_url: url.trim().to_string(),
    };

    let mut guard = mediamtx_state
        .lock()
        .map_err(|_| "Failed to lock MediaMTX state".to_string())?;

    let updated = upsert_single_stream(&mut guard, &stream_name, &pipeline)?;
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

#[tauri::command]
async fn mediamtx_add_camera(
    name: String,
    rtsp: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<bool, String> {
    add_camera_to_mediamtx(name, rtsp, mediamtx_state).await
}

#[tauri::command]
async fn add_camera_streams(
    camera_id: u32,
    hd_url: String,
    sd_url: String,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<bool, String> {
    let hd_stream = format!("cam{}_0", camera_id);
    let sd_stream = format!("cam{}_1", camera_id);

    let hw_preference = match get_app_settings_internal().await {
        Ok(settings) => settings
            .get("hwAccel")
            .and_then(|v| v.as_str())
            .unwrap_or("auto")
            .to_string(),
        Err(err) => {
            println!(
                "Failed to load app settings for live pipeline decision ({}) using 'auto'",
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
        "Live streaming hardware acceleration choice: {} (encoder: {})",
        hw_decision.message, hw_decision.config.video_codec
    );

    let hd_pipeline =
        decide_live_pipeline(&hd_stream, &hd_url, ffmpeg_cmd, &hw_decision.config).await;
    println!(
        "MediaMTX stream '{}' configured for {} (source: {})",
        hd_stream,
        hd_pipeline.description(),
        hd_pipeline.origin_url()
    );

    let sd_pipeline =
        decide_live_pipeline(&sd_stream, &sd_url, ffmpeg_cmd, &hw_decision.config).await;
    println!(
        "MediaMTX stream '{}' configured for {} (source: {})",
        sd_stream,
        sd_pipeline.description(),
        sd_pipeline.origin_url()
    );

    let mut guard = mediamtx_state
        .lock()
        .map_err(|_| "Failed to lock MediaMTX state".to_string())?;

    ensure_mediamtx_files(&mut guard)?;

    let mut config = load_mediamtx_config(&guard)?;
    let mut updated = false;

    if ensure_stream_path(&mut config, &hd_stream, &hd_pipeline) {
        updated = true;
    }

    if ensure_stream_path(&mut config, &sd_stream, &sd_pipeline) {
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
) -> Result<bool, String> {
    let path = path_name.trim().to_string();
    if path.is_empty() {
        return Ok(false);
    }

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

    let config_str = match fs::read_to_string(&config_path) {
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
    rtsp_session: Option<String>,
    rtsp_transport: Option<String>,
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<String, String> {
    println!("[whep_play] Preparing WHEP endpoints for path '{}'", path);
    if let Some(session) = rtsp_session.as_ref() {
        println!("[whep_play] Using RTSP session hint: {}", session);
    }
    if let Some(transport) = rtsp_transport.as_ref() {
        println!("[whep_play] Using RTSP transport hint: {}", transport);
    }

    if let Some(transport_header) = rtsp_transport.as_ref() {
        if let Some(resolved_transport) = infer_rtsp_transport(transport_header) {
            match mediamtx_state.lock() {
                Ok(mut guard) => {
                    if let Err(err) = ensure_mediamtx_files(&mut guard) {
                        println!(
                            "[whep_play] Failed to ensure MediaMTX files before transport update: {}",
                            err
                        );
                    } else if let Ok(mut config) = load_mediamtx_config(&guard) {
                        if set_mediamtx_transport(&mut config, &path, resolved_transport) {
                            println!(
                                "[whep_play] Updating MediaMTX transport for '{}' to {}",
                                path, resolved_transport
                            );
                            if let Err(err) = save_mediamtx_config(&guard, &config) {
                                println!(
                                    "[whep_play] Failed to save MediaMTX config after transport update: {}",
                                    err
                                );
                            } else if let Err(err) = restart_if_running(&mut guard) {
                                println!(
                                    "[whep_play] Failed to restart MediaMTX after transport update: {}",
                                    err
                                );
                            }
                        }
                    }
                }
                Err(err) => println!(
                    "[whep_play] Failed to lock MediaMTX state for transport update: {}",
                    err
                ),
            }
        }
    }

    let base_urls = load_whep_base_urls(&mediamtx_state);
    let mut endpoints: Vec<String> = Vec::new();
    let mut seen: HashSet<String> = HashSet::new();
    let path_ref = &path;

    for base in base_urls {
        let normalized = base.trim_end_matches('/');
        let candidates = [
            format!("{}/api/webrtc?src={}&dst=whep", normalized, path_ref),
            format!("{}/api/webrtc?src={}", normalized, path_ref),
            format!("{}/api/webrtc?dst=whep&src={}", normalized, path_ref),
            format!("{}/whep/{}", normalized, path_ref),
            format!("{}/{}/whep", normalized, path_ref),
        ];

        for candidate in candidates {
            if seen.insert(candidate.clone()) {
                endpoints.push(candidate);
            }
        }
    }

    if endpoints.is_empty() {
        println!("[whep_play] No endpoints discovered; using default loopback WHEP endpoint");
        endpoints.push(format!(
            "http://127.0.0.1:8889/api/webrtc?src={}&dst=whep",
            path_ref
        ));
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
            !trimmed.is_empty() && !trimmed.to_ascii_lowercase().starts_with("rtsp://")
        }) {
            let trimmed = raw.trim();
            if !trimmed.is_empty() {
                let suffix = if trimmed.starts_with('/') {
                    trimmed.to_string()
                } else {
                    format!("/{}", trimmed)
                };
                return Some(format!("{}{}", base, suffix));
            }
        }

        return Some(format!("{}{}", base, camera_default_suffix(variant)));
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
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<Option<String>, String> {
    let stream_name = sanitize_stream_key(&path);

    let mut resolved: Option<String> = match mediamtx_state.lock() {
        Ok(mut guard) => {
            if let Err(err) = ensure_mediamtx_files(&mut guard) {
                println!(
                    "[resolve_stream_source] Failed to ensure MediaMTX files: {}",
                    err
                );
                None
            } else {
                match load_mediamtx_config(&guard) {
                    Ok(config) => extract_mediamtx_source(&config, &stream_name),
                    Err(err) => {
                        println!(
                            "[resolve_stream_source] Failed to load MediaMTX config: {}",
                            err
                        );
                        None
                    }
                }
            }
        }
        Err(err) => {
            println!(
                "[resolve_stream_source] Failed to lock MediaMTX state: {}",
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
) -> Result<HevcProbeResponse, String> {
    let trimmed = stream_path.trim();
    let rtsp_url = if trimmed.to_ascii_lowercase().starts_with("rtsp://") {
        trimmed.to_string()
    } else {
        resolve_stream_source(trimmed.to_string(), mediamtx_state)
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
async fn probe_hevc_export(
    _stream_path: String,
    _mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<HevcProbeResponse, String> {
    Err("HEVC export support is disabled at build time".to_string())
}

#[tauri::command]
async fn get_whep_endpoints(
    mediamtx_state: State<'_, Arc<StdMutex<MediaMtxState>>>,
) -> Result<Vec<String>, String> {
    Ok(load_whep_base_urls(&mediamtx_state))
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

async fn detect_stream_codec(rtsp_url: &str) -> Option<StreamVideoCodec> {
    let handshake = spawn_blocking({
        let url = rtsp_url.to_string();
        move || {
            let params = rtsp_client::RtspHandshakeParams {
                url,
                username: None,
                password: None,
                transport: rtsp_client::TransportProfile::Tcp,
                include_audio: false,
                timeout: Duration::from_millis(2500),
            };
            rtsp_client::perform_handshake(params)
        }
    })
    .await;

    match handshake {
        Ok(Ok(result)) => result.sdp.as_deref().and_then(parse_video_codec_from_sdp),
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

    let mut cmd = StdCommand::new(ffmpeg_cmd);
    cmd.args(&ffmpeg_args)
        .stdin(Stdio::piped())
        .stdout(Stdio::null())
        .stderr(Stdio::null());

    #[cfg(windows)]
    cmd.creation_flags(0x08000000);

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
        }
    }

    args.push("-movflags".into());
    args.push("+faststart".into());
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

        let ffmpeg_cmd = if cfg!(windows) {
            "ffmpeg.exe"
        } else {
            "ffmpeg"
        };
        let mut cmd = StdCommand::new(ffmpeg_cmd);
        cmd.args(&ffmpeg_args)
            .stdin(Stdio::piped())
            .stdout(Stdio::null())
            .stderr(Stdio::null());

        #[cfg(windows)]
        cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW

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

        StdCommand::new(executable)
            .arg(url)
            .spawn()
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
    let password = password_opt
        .ok_or_else(|| "Camera password is required for SSH command".to_string())?;

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
            return Err(format!("Not a directory: {}", target_path.to_string_lossy()));
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
            host_clone,
            port_value,
            path_clone
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
                    println!(
                        "[scp:list] command '{}' failed -> {}",
                        command, err
                    );
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
            entries.len(), host_clone, path_clone
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
