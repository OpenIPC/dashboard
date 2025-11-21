use base64::{engine::general_purpose::STANDARD as BASE64_STANDARD, Engine as _};
use std::collections::HashMap;
use std::io::{BufRead, BufReader, Read, Write};
use std::net::{TcpStream, ToSocketAddrs};
use std::time::Duration;
use url::Url;

const CRLF: &str = "\r\n";
const DEFAULT_RTSP_PORT: u16 = 554;
const DEFAULT_TCP_TRANSPORT: &str = "RTP/AVP/TCP;unicast;interleaved=0-1";
const DEFAULT_UDP_VIDEO_TRANSPORT: &str = "RTP/AVP/UDP;unicast;client_port=5000";
const DEFAULT_UDP_AUDIO_TRANSPORT: &str = "RTP/AVP/UDP;unicast;client_port=5002";

#[derive(Debug, Clone)]
pub enum TransportProfile {
    Tcp,
    Udp,
}

impl TransportProfile {
    fn transport_header_video(&self) -> &'static str {
        match self {
            TransportProfile::Tcp => DEFAULT_TCP_TRANSPORT,
            TransportProfile::Udp => DEFAULT_UDP_VIDEO_TRANSPORT,
        }
    }

    fn transport_header_audio(&self) -> &'static str {
        match self {
            TransportProfile::Tcp => DEFAULT_TCP_TRANSPORT,
            TransportProfile::Udp => DEFAULT_UDP_AUDIO_TRANSPORT,
        }
    }
}

#[derive(Debug, Clone)]
pub struct RtspHandshakeParams {
    pub url: String,
    pub username: Option<String>,
    pub password: Option<String>,
    pub transport: TransportProfile,
    pub include_audio: bool,
    pub timeout: Duration,
}

impl Default for RtspHandshakeParams {
    fn default() -> Self {
        Self {
            url: String::new(),
            username: None,
            password: None,
            transport: TransportProfile::Tcp,
            include_audio: true,
            timeout: Duration::from_secs(3),
        }
    }
}

#[derive(Debug, Clone)]
pub struct RtspTrackResult {
    pub control_uri: String,
    #[allow(dead_code)]
    pub response_status: u16,
    pub response_headers: HashMap<String, String>,
}

#[derive(Debug, Clone)]
pub struct RtspHandshakeResult {
    pub base_uri: String,
    pub video: RtspTrackResult,
    pub audio: Option<RtspTrackResult>,
    pub session: Option<String>,
    pub sdp: Option<String>,
    pub log: Vec<String>,
}

#[derive(Debug)]
pub enum RtspError {
    UrlParse(String),
    ResolveHost(String),
    Connection(String),
    Io(String),
    Protocol(String),
    UnexpectedStatus(u16, String),
}

type RtspResult<T> = Result<T, RtspError>;

fn log_push(log: &mut Vec<String>, entry: impl Into<String>) {
    log.push(entry.into());
}

fn build_authorization(username: &str, password: &str) -> String {
    let token = format!("{}:{}", username, password);
    format!("Basic {}", BASE64_STANDARD.encode(token))
}

fn write_request(
    stream: &mut TcpStream,
    method: &str,
    uri: &str,
    cseq: u32,
    headers: &HashMap<&str, String>,
    body: Option<&[u8]>,
) -> RtspResult<()> {
    let mut request = format!("{} {} RTSP/1.0{}", method, uri, CRLF);
    request.push_str(&format!("CSeq: {}{}", cseq, CRLF));
    for (key, value) in headers {
        request.push_str(key);
        request.push_str(": ");
        request.push_str(value);
        request.push_str(CRLF);
    }

    if let Some(payload) = body {
        request.push_str(&format!(
            "Content-Length: {}{}{}",
            payload.len(),
            CRLF,
            CRLF
        ));
        stream
            .write_all(request.as_bytes())
            .map_err(|e| RtspError::Io(e.to_string()))?;
        stream
            .write_all(payload)
            .map_err(|e| RtspError::Io(e.to_string()))?;
        stream
            .write_all(CRLF.as_bytes())
            .map_err(|e| RtspError::Io(e.to_string()))?;
    } else {
        request.push_str(CRLF);
        stream
            .write_all(request.as_bytes())
            .map_err(|e| RtspError::Io(e.to_string()))?;
    }

    stream.flush().map_err(|e| RtspError::Io(e.to_string()))
}

fn read_response(stream: &mut TcpStream) -> RtspResult<(u16, HashMap<String, String>, Vec<u8>)> {
    let mut reader = BufReader::new(
        stream
            .try_clone()
            .map_err(|e| RtspError::Io(e.to_string()))?,
    );
    let mut status_line = String::new();
    reader
        .read_line(&mut status_line)
        .map_err(|e| RtspError::Io(e.to_string()))?;
    if status_line.is_empty() {
        return Err(RtspError::Protocol("empty response".into()));
    }

    let status_parts: Vec<&str> = status_line.trim_end().splitn(3, ' ').collect();
    if status_parts.len() < 2 {
        return Err(RtspError::Protocol(format!(
            "invalid status line: {}",
            status_line
        )));
    }
    let status_code = status_parts[1]
        .parse::<u16>()
        .map_err(|_| RtspError::Protocol(format!("invalid status code: {}", status_line)))?;

    let mut headers = HashMap::new();
    loop {
        let mut line = String::new();
        reader
            .read_line(&mut line)
            .map_err(|e| RtspError::Io(e.to_string()))?;
        let trimmed = line.trim_end();
        if trimmed.is_empty() {
            break;
        }
        if let Some((name, value)) = trimmed.split_once(':') {
            headers.insert(name.trim().to_ascii_lowercase(), value.trim().to_string());
        }
    }

    let mut body = Vec::new();
    if let Some(content_length) = headers.get("content-length") {
        if let Ok(len) = content_length.parse::<usize>() {
            let mut buffer = vec![0u8; len];
            reader
                .read_exact(&mut buffer)
                .map_err(|e| RtspError::Io(e.to_string()))?;
            body.extend_from_slice(&buffer[..]);
            // Consume trailing CRLF if present
            if len > 0 {
                let mut tail = [0u8; 2];
                if reader.read_exact(&mut tail).is_ok() {
                    if &tail != CRLF.as_bytes() {
                        body.extend_from_slice(&tail);
                    }
                }
            }
        }
    } else {
        // Some servers do not send Content-Length for zero-sized bodies
        // Drain until no more bytes are immediately available (best effort)
        let mut buf = [0u8; 1024];
        loop {
            match reader.read(&mut buf) {
                Ok(0) => break,
                Ok(n) => body.extend_from_slice(&buf[..n]),
                Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => break,
                Err(e) => return Err(RtspError::Io(e.to_string())),
            }
            if reader.buffer().is_empty() {
                break;
            }
        }
    }

    Ok((status_code, headers, body))
}

fn build_control_uri(base: &Url, control: &str) -> String {
    if control.starts_with("rtsp://") {
        control.to_string()
    } else if control.starts_with('/') {
        format!(
            "rtsp://{}:{}{}",
            base.host_str().unwrap_or_default(),
            base.port_or_known_default().unwrap_or(DEFAULT_RTSP_PORT),
            control
        )
    } else {
        let mut new_base = base.clone();
        let mut path = base.path().trim_end_matches('/').to_string();
        if !path.ends_with('/') {
            path.push('/');
        }
        path.push_str(control);
        new_base.set_path(&path);
        new_base.to_string()
    }
}

fn parse_sdp_tracks(base: &Url, sdp: &str) -> (Option<String>, Option<String>) {
    let mut current_media: Option<String> = None;
    let mut video_control: Option<String> = None;
    let mut audio_control: Option<String> = None;

    for line in sdp.lines() {
        let trimmed = line.trim();
        if trimmed.starts_with('m') {
            current_media = trimmed
                .split_whitespace()
                .nth(0)
                .and_then(|token| token.split_once('='))
                .map(|(_, value)| value.to_lowercase());
            continue;
        }
        if trimmed.starts_with("a=control:") {
            if let Some(media) = &current_media {
                if let Some((_, ctrl)) = trimmed.split_once(':') {
                    let full = build_control_uri(base, ctrl);
                    if media.starts_with("video") && video_control.is_none() {
                        video_control = Some(full);
                    } else if media.starts_with("audio") && audio_control.is_none() {
                        audio_control = Some(full);
                    }
                }
            }
        }
    }

    (video_control, audio_control)
}

fn perform_describe(
    stream: &mut TcpStream,
    base_uri: &str,
    cseq: u32,
    headers: &HashMap<&str, String>,
    log: &mut Vec<String>,
) -> RtspResult<(HashMap<String, String>, Vec<u8>)> {
    log_push(log, format!("DESCRIBE {}", base_uri));
    write_request(stream, "DESCRIBE", base_uri, cseq, headers, None)?;
    let (status, resp_headers, body) = read_response(stream)?;
    if status != 200 {
        let message = String::from_utf8_lossy(&body).into_owned();
        return Err(RtspError::UnexpectedStatus(status, message));
    }
    Ok((resp_headers, body))
}

fn perform_setup(
    stream: &mut TcpStream,
    track_uri: &str,
    cseq: u32,
    session: Option<&str>,
    transport: &str,
    headers: &HashMap<&str, String>,
    log: &mut Vec<String>,
) -> RtspResult<(u16, HashMap<String, String>)> {
    let mut setup_headers: HashMap<&str, String> = HashMap::new();
    setup_headers.insert("Transport", transport.to_string());
    for (key, value) in headers {
        setup_headers.entry(key).or_insert_with(|| value.clone());
    }
    if let Some(session_id) = session {
        setup_headers.insert("Session", session_id.to_string());
    }

    log_push(log, format!("SETUP {}", track_uri));
    write_request(stream, "SETUP", track_uri, cseq, &setup_headers, None)?;
    let (status, resp_headers, body) = read_response(stream)?;
    if status != 200 {
        let message = String::from_utf8_lossy(&body).into_owned();
        return Err(RtspError::UnexpectedStatus(status, message));
    }

    Ok((status, resp_headers))
}

fn perform_play(
    stream: &mut TcpStream,
    base_uri: &str,
    cseq: u32,
    session: &str,
    headers: &HashMap<&str, String>,
    log: &mut Vec<String>,
) -> RtspResult<()> {
    let mut play_headers: HashMap<&str, String> = HashMap::new();
    play_headers.insert("Session", session.to_string());
    for (key, value) in headers {
        play_headers.entry(key).or_insert_with(|| value.clone());
    }

    log_push(log, format!("PLAY {}", base_uri));
    write_request(stream, "PLAY", base_uri, cseq, &play_headers, None)?;
    let (status, resp_headers, body) = read_response(stream)?;
    if status != 200 {
        let message = String::from_utf8_lossy(&body).into_owned();
        return Err(RtspError::UnexpectedStatus(status, message));
    }
    log_push(log, format!("PLAY response headers: {:?}", resp_headers));
    log_push(log, format!("PLAY response body: {} bytes", body.len()));
    Ok(())
}

pub fn perform_handshake(params: RtspHandshakeParams) -> RtspResult<RtspHandshakeResult> {
    let parsed_url = Url::parse(&params.url).map_err(|e| RtspError::UrlParse(e.to_string()))?;
    if parsed_url.scheme() != "rtsp" {
        return Err(RtspError::Protocol("URL must use rtsp scheme".into()));
    }

    let host = parsed_url
        .host_str()
        .ok_or_else(|| RtspError::Protocol("RTSP URL missing host".into()))?;
    let port = parsed_url.port().unwrap_or(DEFAULT_RTSP_PORT);
    let addr = format!("{}:{}", host, port);
    let mut addrs = addr
        .to_socket_addrs()
        .map_err(|e| RtspError::ResolveHost(e.to_string()))?;
    let socket_addr = addrs
        .next()
        .ok_or_else(|| RtspError::ResolveHost("no address resolved".into()))?;

    let mut stream = TcpStream::connect_timeout(&socket_addr, params.timeout)
        .map_err(|e| RtspError::Connection(e.to_string()))?;
    stream
        .set_read_timeout(Some(params.timeout))
        .map_err(|e| RtspError::Io(e.to_string()))?;
    stream
        .set_write_timeout(Some(params.timeout))
        .map_err(|e| RtspError::Io(e.to_string()))?;

    let mut log = Vec::new();
    log_push(&mut log, format!("Connected to {}", addr));

    let mut base_uri = parsed_url.clone();
    base_uri.set_query(None);

    let mut common_headers: HashMap<&str, String> = HashMap::new();
    if let Some(user) = params
        .username
        .as_deref()
        .filter(|value| !value.is_empty())
        .or_else(|| {
            let name = parsed_url.username();
            (!name.is_empty()).then_some(name)
        })
    {
        let password = params
            .password
            .as_deref()
            .or_else(|| parsed_url.password())
            .unwrap_or("");
        let auth = build_authorization(user, password);
        common_headers.insert("Authorization", auth);
    }

    let mut cseq = 1u32;
    let (_describe_headers, describe_body) = perform_describe(
        &mut stream,
        base_uri.as_str(),
        cseq,
        &common_headers,
        &mut log,
    )?;
    cseq += 1;

    let sdp =
        String::from_utf8(describe_body.clone()).map_err(|e| RtspError::Protocol(e.to_string()))?;
    log_push(
        &mut log,
        format!("DESCRIBE returned SDP ({} bytes)", sdp.len()),
    );
    let (video_control, audio_control) = parse_sdp_tracks(&base_uri, &sdp);
    let video_control_uri = video_control
        .ok_or_else(|| RtspError::Protocol("SDP missing video control attribute".into()))?;

    let mut session: Option<String> = None;
    let (setup_status, video_headers) = perform_setup(
        &mut stream,
        &video_control_uri,
        cseq,
        session.as_deref(),
        params.transport.transport_header_video(),
        &common_headers,
        &mut log,
    )?;
    cseq += 1;
    log_push(&mut log, format!("Video SETUP status {}", setup_status));
    if session.is_none() {
        if let Some(value) = video_headers.get("session") {
            session = Some(value.clone());
        }
    }
    let session_id = session
        .as_deref()
        .ok_or_else(|| RtspError::Protocol("SETUP response missing Session header".into()))?;

    let audio_result = if params.include_audio {
        if let Some(audio_uri) = audio_control {
            match perform_setup(
                &mut stream,
                &audio_uri,
                cseq,
                Some(session_id),
                params.transport.transport_header_audio(),
                &common_headers,
                &mut log,
            ) {
                Ok((_status, audio_headers)) => {
                    cseq += 1;
                    Some(RtspTrackResult {
                        control_uri: audio_uri,
                        response_status: 200,
                        response_headers: audio_headers,
                    })
                }
                Err(err) => {
                    log_push(
                        &mut log,
                        format!("Audio SETUP skipped due to error: {}", err),
                    );
                    None
                }
            }
        } else {
            log_push(&mut log, "SDP missing audio control attribute");
            None
        }
    } else {
        None
    };

    perform_play(
        &mut stream,
        base_uri.as_str(),
        cseq,
        session_id,
        &common_headers,
        &mut log,
    )?;

    Ok(RtspHandshakeResult {
        base_uri: base_uri.to_string(),
        video: RtspTrackResult {
            control_uri: video_control_uri,
            response_status: 200,
            response_headers: video_headers,
        },
        audio: audio_result,
        session: session.map(|s| s.split(';').next().unwrap_or(&s).to_string()),
        sdp: Some(sdp),
        log,
    })
}

impl std::fmt::Display for RtspError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            RtspError::UrlParse(err) => write!(f, "URL parse error: {}", err),
            RtspError::ResolveHost(err) => write!(f, "resolve error: {}", err),
            RtspError::Connection(err) => write!(f, "connection error: {}", err),
            RtspError::Io(err) => write!(f, "io error: {}", err),
            RtspError::Protocol(err) => write!(f, "protocol error: {}", err),
            RtspError::UnexpectedStatus(code, msg) => {
                write!(f, "unexpected status {}: {}", code, msg)
            }
        }
    }
}

impl std::error::Error for RtspError {}
