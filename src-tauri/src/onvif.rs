use base64::{engine::general_purpose, Engine as _};
use chrono::Utc;
use lazy_static::lazy_static;
use rand::Rng;
use regex::Regex;
use reqwest::{Client, Url};
use sha1::{Digest, Sha1};
use std::collections::HashMap;
use std::sync::Mutex;

#[derive(Clone)]
struct OnvifDeviceCache {
    ptz_service_url: Option<String>,
    media_service_url: Option<String>,
    imaging_service_url: Option<String>,
    profile_token: Option<String>,
    video_source_token: Option<String>,
    preferred_port: Option<u16>,
    dead_ports: Vec<u16>,
}

fn new_cache_entry() -> OnvifDeviceCache {
    OnvifDeviceCache {
        ptz_service_url: None,
        media_service_url: None,
        imaging_service_url: None,
        profile_token: None,
        video_source_token: None,
        preferred_port: None,
        dead_ports: Vec::new(),
    }
}

lazy_static! {
    static ref ONVIF_CACHE: Mutex<HashMap<String, OnvifDeviceCache>> = Mutex::new(HashMap::new());
    static ref CLIENT: Client = Client::builder()
        .timeout(std::time::Duration::from_secs(5))
        .no_proxy()
        .http1_only()
        .tcp_nodelay(true)
        .user_agent("curl/8.14.1")
        .build()
        .expect("Failed to create ONVIF HTTP client");
}

fn cache_key(ip: &str, port: u16) -> String {
    format!("{}:{}", ip, port)
}

fn cache_snapshot(ip: &str, port: u16) -> Option<OnvifDeviceCache> {
    ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| {
            cache
                .get(&cache_key(ip, port))
                .cloned()
                .or_else(|| {
                    let prefix = format!("{}:", ip);
                    cache
                        .iter()
                        .find(|(k, _)| k.starts_with(&prefix))
                        .map(|(_, entry)| entry.clone())
                })
        })
}

fn mark_preferred_port(ip: &str, port: u16, preferred_port: u16) {
    if let Ok(mut cache) = ONVIF_CACHE.lock() {
        let key = cache_key(ip, port);
        cache
            .entry(key.clone())
            .or_insert_with(new_cache_entry);

        let prefix = format!("{}:", ip);
        for (k, entry) in cache.iter_mut() {
            if k == &key || k.starts_with(&prefix) {
                entry.preferred_port = Some(preferred_port);
                entry.dead_ports.retain(|p| *p != preferred_port);
            }
        }
    }
}

fn mark_dead_port(ip: &str, port: u16, dead_port: u16) {
    if let Ok(mut cache) = ONVIF_CACHE.lock() {
        let key = cache_key(ip, port);
        cache
            .entry(key.clone())
            .or_insert_with(new_cache_entry);

        let prefix = format!("{}:", ip);
        for (k, entry) in cache.iter_mut() {
            if k == &key || k.starts_with(&prefix) {
                if !entry.dead_ports.contains(&dead_port) {
                    entry.dead_ports.push(dead_port);
                }
                if entry.preferred_port == Some(dead_port) {
                    entry.preferred_port = None;
                }
            }
        }
    }
}

fn extract_port(url: &str) -> Option<u16> {
    Url::parse(url)
        .ok()
        .and_then(|parsed| parsed.port_or_known_default())
}

fn is_unreachable_error(err: &str) -> bool {
    let lowered = err.to_ascii_lowercase();
    lowered.contains("timed out")
        || lowered.contains("connection refused")
        || lowered.contains("could not connect")
        || lowered.contains("connection reset")
        || lowered.contains("network is unreachable")
        || lowered.contains("no route to host")
}

#[tauri::command]
pub async fn get_rtsp_url(
    ip: String,
    _port: u16,
    user: String,
    pass: String,
) -> Result<String, String> {
    let u = urlencoding::encode(&user);
    let p = urlencoding::encode(&pass);
    let standard_url = format!("rtsp://{}:{}@{}:554/stream=0", u, p, ip);

    println!(
        "Using standard RTSP URL: rtsp://{}:****@{}:554/stream=0",
        u, ip
    );
    Ok(standard_url)
}

fn create_security_header(user: &str, pass: &str) -> String {
    let mut rng = rand::thread_rng();
    let nonce_bytes: [u8; 16] = rng.gen();
    let nonce_b64 = general_purpose::STANDARD.encode(nonce_bytes);
    let created = Utc::now().format("%Y-%m-%dT%H:%M:%S.000Z").to_string();

    let mut hasher = Sha1::new();
    hasher.update(&nonce_bytes);
    hasher.update(created.as_bytes());
    hasher.update(pass.as_bytes());
    let password_digest = general_purpose::STANDARD.encode(hasher.finalize());

    format!(
        r#"<Security s:mustUnderstand="1" xmlns="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd">
    <UsernameToken>
      <Username>{}</Username>
      <Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest">{}</Password>
      <Nonce EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary">{}</Nonce>
      <Created xmlns="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd">{}</Created>
    </UsernameToken>
  </Security>"#,
        user, password_digest, nonce_b64, created
    )
}

async fn send_soap(
    url: &str,
    body_content: &str,
    user: &str,
    pass: &str,
    action: &str,
    service_hint: &str,
) -> Result<String, String> {
    let client = &*CLIENT;
    let security_header = if !user.is_empty() {
        Some(create_security_header(user, pass))
    } else {
        None
    };

    let header_block = security_header
        .as_deref()
        .map(|header| format!("<s:Header>{}</s:Header>", header))
        .unwrap_or_default();

    let soap12 = format!(
        r#"<?xml version="1.0" encoding="UTF-8"?>
<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope">
  {}
  <s:Body xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
    {}
  </s:Body>
</s:Envelope>"#,
        header_block,
        body_content
    );

    let res12 = client
        .post(url)
        .header("Content-Type", "application/soap+xml; charset=utf-8")
        .header("Connection", "close")
        .header("User-Agent", "curl/8.14.1")
        .header("Accept", "*/*")
        .body(soap12)
        .send()
        .await;

    if let Ok(resp) = res12 {
        if resp.status().is_success() {
            return resp.text().await.map_err(|e| e.to_string());
        }
        println!(
            "SOAP 1.2 failed for {} with status: {}",
            service_hint,
            resp.status()
        );
        if let Ok(text) = resp.text().await {
            println!("SOAP 1.2 error body: {}", text);
        }
    } else if let Err(e) = &res12 {
        println!("SOAP 1.2 failed for {} with error: {}", service_hint, e);
    }

    let soap11 = format!(
        r#"<?xml version="1.0" encoding="UTF-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
    {}
  <s:Body xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
    {}
  </s:Body>
</s:Envelope>"#,
                header_block,
        body_content
    );

    let res11 = client
        .post(url)
        .header("Content-Type", "text/xml; charset=utf-8")
        .header("SOAPAction", action)
        .header("Connection", "close")
        .header("User-Agent", "curl/8.14.1")
        .header("Accept", "*/*")
        .body(soap11)
        .send()
        .await;

    match res11 {
        Ok(resp) => {
            if resp.status().is_success() {
                resp.text().await.map_err(|e| e.to_string())
            } else {
                let status = resp.status();
                let text = resp.text().await.unwrap_or_default();
                println!(
                    "SOAP 1.1 failed for {} with status: {} body: {}",
                    service_hint, status, text
                );
                Err(format!(
                    "SOAP 1.1 failed with status: {} body: {}",
                    status, text
                ))
            }
        }
        Err(e) => {
            println!("SOAP 1.1 failed for {} with error: {}", service_hint, e);
            Err(e.to_string())
        }
    }
}

fn candidate_ports(ip: &str, preferred: u16) -> Vec<u16> {
    let mut ports = vec![preferred];
    for fallback in [8899u16, 8999, 8080, 8000, 85, 8001, 81] {
        if !ports.contains(&fallback) {
            ports.push(fallback);
        }
    }

    if let Some(entry) = cache_snapshot(ip, preferred) {
        if let Some(pref_port) = entry.preferred_port {
            if let Some(pos) = ports.iter().position(|p| *p == pref_port) {
                if pos != 0 {
                    ports.remove(pos);
                    ports.insert(0, pref_port);
                }
            } else {
                ports.insert(0, pref_port);
            }
        }

        if !entry.dead_ports.is_empty() {
            ports.retain(|p| !entry.dead_ports.contains(p));
            for dead in entry.dead_ports {
                if !ports.contains(&dead) {
                    ports.push(dead);
                }
            }
        }
    }

    ports
}

fn cached_media_url(ip: &str, port: u16) -> Option<String> {
    let key = cache_key(ip, port);
    if let Ok(cache) = ONVIF_CACHE.lock() {
        if let Some(entry) = cache.get(&key) {
            if let Some(url) = &entry.media_service_url {
                return Some(url.clone());
            }
        }

        let prefix = format!("{}:", ip);
        if let Some((_, entry)) = cache.iter().find(|(k, _)| k.starts_with(&prefix)) {
            if let Some(url) = &entry.media_service_url {
                return Some(url.clone());
            }
        }
    }
    None
}

fn derive_service_url(base: &str, replacements: &[(&str, &str)]) -> Option<String> {
    for (needle, replacement) in replacements {
        if base.contains(needle) {
            return Some(base.replacen(needle, replacement, 1));
        }
    }
    None
}

async fn get_ptz_service_url(
    ip: &str,
    port: u16,
    user: &str,
    pass: &str,
) -> Result<String, String> {
    let key = cache_key(ip, port);
    if let Some(url) = ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| cache.get(&key).and_then(|c| c.ptz_service_url.clone()))
    {
        return Ok(url);
    }

    let paths = [
        "/onvif/device_service",
        "/device_service",
        "/onvif/device",
    ];

    let body = r#"<GetCapabilities xmlns="http://www.onvif.org/ver10/device/wsdl">
      <Category>PTZ</Category>
    </GetCapabilities>"#;
    let action = "http://www.onvif.org/ver10/device/wsdl/GetCapabilities";

    for candidate_port in candidate_ports(ip, port) {
        let mut unreachable = false;
        for path in paths {
            let url = format!("http://{}:{}{}", ip, candidate_port, path);
            println!("Trying Device Service URL: {}", url);

            match send_soap(
                &url,
                body,
                user,
                pass,
                action,
                &format!("device:{}:{}:PTZ", candidate_port, path),
            )
            .await
            {
                Ok(resp) => {
                    let re = Regex::new(r"(?is)<[^:>]*:PTZ[^>]*>.*?<[^:>]*:XAddr>([^<]+)</[^:>]*:XAddr>")
                        .map_err(|e| e.to_string())?;
                    if let Some(caps) = re.captures(&resp) {
                        let xaddr = caps[1].to_string();
                        println!("Found PTZ Service URL: {}", xaddr);

                        if let Ok(mut cache) = ONVIF_CACHE.lock() {
                            let entry = cache.entry(key.clone()).or_insert_with(new_cache_entry);
                            entry.ptz_service_url = Some(xaddr.clone());
                        }

                        mark_preferred_port(ip, port, candidate_port);
                        if let Some(service_port) = extract_port(&xaddr) {
                            mark_preferred_port(ip, port, service_port);
                        }

                        return Ok(xaddr);
                    }
                }
                Err(err) => {
                    if is_unreachable_error(&err) {
                        mark_dead_port(ip, port, candidate_port);
                        unreachable = true;
                        break;
                    }
                }
            }
        }

        if unreachable {
            continue;
        }
    }

    if let Some(media_url) = cached_media_url(ip, port) {
        if let Some(derived) = derive_service_url(
            &media_url,
            &[
                ("/Media", "/PTZ"),
                ("/MediaService", "/PTZ"),
                ("/media_service", "/ptz_service"),
                ("/media", "/ptz"),
                ("Media", "PTZ"),
            ],
        ) {
            println!(
                "Derived PTZ Service URL {} from Media Service {}",
                derived, media_url
            );

            if let Ok(mut cache) = ONVIF_CACHE.lock() {
                let entry = cache.entry(key.clone()).or_insert_with(new_cache_entry);
                entry.ptz_service_url = Some(derived.clone());
            }

            if let Some(service_port) = extract_port(&derived) {
                mark_preferred_port(ip, port, service_port);
            }

            return Ok(derived);
        }
    }

    Err("PTZ Service URL not found".to_string())
}

async fn get_media_service_url(
    ip: &str,
    port: u16,
    user: &str,
    pass: &str,
) -> Result<String, String> {
    let key = cache_key(ip, port);
    if let Some(url) = ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| cache.get(&key).and_then(|c| c.media_service_url.clone()))
    {
        return Ok(url);
    }

    let paths = [
        "/onvif/device_service",
        "/device_service",
        "/onvif/device",
    ];

    let body = r#"<GetCapabilities xmlns="http://www.onvif.org/ver10/device/wsdl">
      <Category>Media</Category>
    </GetCapabilities>"#;
    let action = "http://www.onvif.org/ver10/device/wsdl/GetCapabilities";

    for candidate_port in candidate_ports(ip, port) {
        let mut unreachable = false;
        for path in paths {
            let url = format!("http://{}:{}{}", ip, candidate_port, path);
            println!("Trying Device Service URL for Media: {}", url);

            match send_soap(
                &url,
                body,
                user,
                pass,
                action,
                &format!("device:{}:{}:Media", candidate_port, path),
            )
            .await
            {
                Ok(resp) => {
                    let re = Regex::new(r"(?is)<[^:>]*:Media[^>]*>.*?<[^:>]*:XAddr>([^<]+)</[^:>]*:XAddr>")
                        .map_err(|e| e.to_string())?;
                    if let Some(caps) = re.captures(&resp) {
                        let xaddr = caps[1].to_string();
                        println!("Found Media Service URL: {}", xaddr);

                        if let Ok(mut cache) = ONVIF_CACHE.lock() {
                            let entry = cache.entry(key.clone()).or_insert_with(new_cache_entry);
                            entry.media_service_url = Some(xaddr.clone());
                        }

                        mark_preferred_port(ip, port, candidate_port);
                        if let Some(service_port) = extract_port(&xaddr) {
                            mark_preferred_port(ip, port, service_port);
                        }

                        return Ok(xaddr);
                    }
                }
                Err(err) => {
                    if is_unreachable_error(&err) {
                        mark_dead_port(ip, port, candidate_port);
                        unreachable = true;
                        break;
                    }
                }
            }
        }

        if unreachable {
            continue;
        }
    }

    Err("Media Service URL not found".to_string())
}

fn extract_profile_and_cache(key: &str, text: &str) -> Result<Option<String>, String> {
    let re = Regex::new(r#"Profiles[^>]*token=\"([^\"]+)\""#).map_err(|e| e.to_string())?;
    if let Some(caps) = re.captures(text) {
        let token = caps[1].to_string();
        println!("Found Profile Token: {}", token);

        let vs_token = if let Ok(re_vs) =
            Regex::new(r"(?s)VideoSourceConfiguration.*?SourceToken>([^<]+)<")
        {
            re_vs.captures(text).map(|c| c[1].to_string())
        } else {
            Regex::new(r"(?i)SourceToken>([^<]+)<").ok()
                .and_then(|re| re.captures(text).map(|c| c[1].to_string()))
        };

        if let Ok(mut cache) = ONVIF_CACHE.lock() {
            let entry = cache.entry(key.to_string()).or_insert_with(new_cache_entry);
            entry.profile_token = Some(token.clone());
            if let Some(vt) = vs_token {
                let vt = vt.trim().to_string();
                println!("Found Video Source Token: {}", vt);
                entry.video_source_token = Some(vt);
            }
        }

        return Ok(Some(token));
    }

    Ok(None)
}

async fn get_first_profile_token(
    ip: &str,
    port: u16,
    user: &str,
    pass: &str,
) -> Result<String, String> {
    let key = cache_key(ip, port);
    if let Some(token) = ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| cache.get(&key).and_then(|c| c.profile_token.clone()))
    {
        return Ok(token);
    }

    let body = r#"<GetProfiles xmlns="http://www.onvif.org/ver10/media/wsdl"/>"#;
    let action = "http://www.onvif.org/ver10/media/wsdl/GetProfiles";

    if let Ok(media_url) = get_media_service_url(ip, port, user, pass).await {
        println!("Using discovered Media Service URL: {}", media_url);
        if let Ok(text) = send_soap(
            &media_url,
            body,
            user,
            pass,
            action,
            "media:get_profiles:cached",
        )
        .await
        {
            println!("GetProfiles Response: {}", text);
            if let Some(token) = extract_profile_and_cache(&key, &text)? {
                return Ok(token);
            }
        }
    }

    let paths = [
        "/onvif/media_service",
        "/onvif/media",
        "/media_service",
    ];

    for candidate_port in candidate_ports(ip, port) {
        let mut unreachable = false;
        for path in paths {
            let url = format!("http://{}:{}{}", ip, candidate_port, path);
            println!("Trying Media Service URL: {}", url);

            match send_soap(
                &url,
                body,
                user,
                pass,
                action,
                &format!("media:get_profiles:{}:{}", candidate_port, path),
            )
            .await
            {
                Ok(text) => {
                    println!("GetProfiles Response: {}", text);

                    if let Some(token) = extract_profile_and_cache(&key, &text)? {
                        mark_preferred_port(ip, port, candidate_port);
                        return Ok(token);
                    }
                }
                Err(err) => {
                    if is_unreachable_error(&err) {
                        mark_dead_port(ip, port, candidate_port);
                        unreachable = true;
                        break;
                    }
                }
            }
        }

        if unreachable {
            continue;
        }
    }

    Err("No Profile Token found".to_string())
}

async fn get_video_source_token(
    ip: &str,
    port: u16,
    user: &str,
    pass: &str,
) -> Result<String, String> {
    let key = cache_key(ip, port);
    if let Some(token) = ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| cache.get(&key).and_then(|c| c.video_source_token.clone()))
    {
        return Ok(token);
    }

    get_first_profile_token(ip, port, user, pass).await?;

    ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| cache.get(&key).and_then(|c| c.video_source_token.clone()))
        .ok_or_else(|| "Video Source Token not found".to_string())
}

async fn get_imaging_service_url(
    ip: &str,
    port: u16,
    user: &str,
    pass: &str,
) -> Result<String, String> {
    let key = cache_key(ip, port);
    if let Some(url) = ONVIF_CACHE
        .lock()
        .ok()
        .and_then(|cache| cache.get(&key).and_then(|c| c.imaging_service_url.clone()))
    {
        return Ok(url);
    }

    let paths = [
        "/onvif/device_service",
        "/device_service",
        "/onvif/device",
    ];

    let body = r#"<GetCapabilities xmlns="http://www.onvif.org/ver10/device/wsdl">
      <Category>Imaging</Category>
    </GetCapabilities>"#;
    let action = "http://www.onvif.org/ver10/device/wsdl/GetCapabilities";

    for candidate_port in candidate_ports(ip, port) {
        let mut unreachable = false;
        for path in paths {
            let url = format!("http://{}:{}{}", ip, candidate_port, path);
            println!("Trying Device Service URL for Imaging: {}", url);

            match send_soap(
                &url,
                body,
                user,
                pass,
                action,
                &format!("device:{}:{}:imaging", candidate_port, path),
            )
            .await
            {
                Ok(resp) => {
                    let re = Regex::new(r"(?is)<[^:>]*:Imaging[^>]*>.*?<[^:>]*:XAddr>([^<]+)</[^:>]*:XAddr>")
                        .map_err(|e| e.to_string())?;
                    if let Some(caps) = re.captures(&resp) {
                        let xaddr = caps[1].to_string();
                        println!("Found Imaging Service URL: {}", xaddr);

                        if let Ok(mut cache) = ONVIF_CACHE.lock() {
                            let entry = cache.entry(key.clone()).or_insert_with(new_cache_entry);
                            entry.imaging_service_url = Some(xaddr.clone());
                        }

                        mark_preferred_port(ip, port, candidate_port);
                        if let Some(service_port) = extract_port(&xaddr) {
                            mark_preferred_port(ip, port, service_port);
                        }

                        return Ok(xaddr);
                    }
                }
                Err(err) => {
                    if is_unreachable_error(&err) {
                        mark_dead_port(ip, port, candidate_port);
                        unreachable = true;
                        break;
                    }
                }
            }
        }

        if unreachable {
            continue;
        }
    }

    if let Some(media_url) = cached_media_url(ip, port) {
        if let Some(derived) = derive_service_url(
            &media_url,
            &[
                ("/Media", "/Imaging"),
                ("/MediaService", "/Imaging"),
                ("/media_service", "/imaging_service"),
                ("/media", "/imaging"),
                ("Media", "Imaging"),
            ],
        ) {
            println!(
                "Derived Imaging Service URL {} from Media Service {}",
                derived, media_url
            );

            if let Ok(mut cache) = ONVIF_CACHE.lock() {
                let entry = cache.entry(key.clone()).or_insert_with(new_cache_entry);
                entry.imaging_service_url = Some(derived.clone());
            }

            if let Some(service_port) = extract_port(&derived) {
                mark_preferred_port(ip, port, service_port);
            }

            return Ok(derived);
        }
    }

    Err("Imaging Service URL not found".to_string())
}

fn focus_mode_body(video_source_token: &str, mode: &str, include_force_persistence: bool) -> String {
    let persistence_attr = if include_force_persistence {
        " ForcePersistence=\"false\""
    } else {
        ""
    };

    format!(
        r#"<SetImagingSettings xmlns="http://www.onvif.org/ver20/imaging/wsdl"{}>
            <VideoSourceToken>{}</VideoSourceToken>
            <ImagingSettings xmlns:tt="http://www.onvif.org/ver10/schema">
                <tt:Focus>
                    <tt:AutoFocusMode>{}</tt:AutoFocusMode>
                </tt:Focus>
            </ImagingSettings>
        </SetImagingSettings>"#,
        persistence_attr, video_source_token, mode
    )
}

async fn apply_focus_mode(
    imaging_url: &str,
    video_source_token: &str,
    user: &str,
    pass: &str,
    mode: &str,
) -> Result<(), String> {
    let action = "http://www.onvif.org/ver20/imaging/wsdl/SetImagingSettings";

    let body_with_force = focus_mode_body(video_source_token, mode, true);
    match send_soap(
        imaging_url,
        &body_with_force,
        user,
        pass,
        action,
        &format!("imaging:set_focus_mode:{}", mode),
    )
    .await
    {
        Ok(_) => Ok(()),
        Err(err) => {
            let err_lower = err.to_ascii_lowercase();
            if err_lower.contains("forcepersistence") {
                println!(
                    "Camera rejected ForcePersistence flag, retrying SetImagingSettings without it"
                );
                let fallback_body = focus_mode_body(video_source_token, mode, false);
                println!("Running fallback SetImagingSettings request without ForcePersistence");
                return send_soap(
                    imaging_url,
                    &fallback_body,
                    user,
                    pass,
                    action,
                    &format!("imaging:set_focus_mode:{}:fallback", mode),
                )
                .await
                .map(|_| ())
                .map_err(|fallback_err| {
                    println!(
                        "Fallback imaging request without ForcePersistence failed: {}",
                        fallback_err
                    );
                    fallback_err
                });
            }

            Err(err)
        }
    }
}

    async fn fetch_focus_mode(
        imaging_url: &str,
        video_source_token: &str,
        user: &str,
        pass: &str,
    ) -> Result<String, String> {
        let body = format!(
            r#"<GetImagingSettings xmlns="http://www.onvif.org/ver20/imaging/wsdl">
          <VideoSourceToken>{}</VideoSourceToken>
        </GetImagingSettings>"#,
            video_source_token
        );
        let action = "http://www.onvif.org/ver20/imaging/wsdl/GetImagingSettings";

        let response = send_soap(
            imaging_url,
            &body,
            user,
            pass,
            action,
            "imaging:get_focus_mode",
        )
        .await?;

        let mode_re = Regex::new(r"(?i)<AutoFocusMode>([^<]+)</AutoFocusMode>")
            .map_err(|e| e.to_string())?;
        if let Some(caps) = mode_re.captures(&response) {
            return Ok(caps[1].trim().to_ascii_uppercase());
        }

        Err("AutoFocusMode not found".to_string())
    }

    #[tauri::command]
    pub async fn get_focus_mode(
        ip: String,
        port: u16,
        user: String,
        pass: String,
    ) -> Result<String, String> {
        let video_token = get_video_source_token(&ip, port, &user, &pass)
            .await
            .unwrap_or_else(|_| {
                println!("Using default Video Source Token: VideoSource_1");
                "VideoSource_1".to_string()
            });

        let imaging_url = get_imaging_service_url(&ip, port, &user, &pass)
            .await
            .unwrap_or_else(|_| {
                let alt_port = candidate_ports(&ip, port)
                    .into_iter()
                    .find(|p| *p != port)
                    .unwrap_or(port);
                let fallback = format!("http://{}:{}/onvif/imaging_service", ip, alt_port);
                println!("Using fallback Imaging URL: {}", fallback);
                fallback
            });

        fetch_focus_mode(&imaging_url, &video_token, &user, &pass).await
    }

    #[tauri::command]
    pub async fn set_focus_mode(
        ip: String,
        port: u16,
        user: String,
        pass: String,
        mode: String,
    ) -> Result<(), String> {
        let normalized = mode.to_ascii_uppercase();
        if normalized != "AUTO" && normalized != "MANUAL" {
            return Err(format!("Unsupported focus mode: {}", mode));
        }

        let video_token = get_video_source_token(&ip, port, &user, &pass)
            .await
            .unwrap_or_else(|_| {
                println!("Using default Video Source Token: VideoSource_1");
                "VideoSource_1".to_string()
            });

        let imaging_url = get_imaging_service_url(&ip, port, &user, &pass)
            .await
            .unwrap_or_else(|_| {
                let alt_port = candidate_ports(&ip, port)
                    .into_iter()
                    .find(|p| *p != port)
                    .unwrap_or(port);
                let fallback = format!("http://{}:{}/onvif/imaging_service", ip, alt_port);
                println!("Using fallback Imaging URL: {}", fallback);
                fallback
            });

        apply_focus_mode(&imaging_url, &video_token, &user, &pass, &normalized).await
    }

#[tauri::command]
pub async fn ptz_move(
    ip: String,
    port: u16,
    user: String,
    pass: String,
    x: f32,
    y: f32,
    zoom: f32,
) -> Result<(), String> {
    let token = get_first_profile_token(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            println!("Using default Profile Token: Profile_1");
            "Profile_1".to_string()
        });

    let ptz_url = get_ptz_service_url(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            let alt_port = candidate_ports(&ip, port)
                .into_iter()
                .find(|p| *p != port)
                .unwrap_or(port);
            let fallback = format!("http://{}:{}/onvif/ptz_service", ip, alt_port);
            println!("Using fallback PTZ URL: {}", fallback);
            fallback
        });

    let body = format!(
        r#"<ContinuousMove xmlns="http://www.onvif.org/ver20/ptz/wsdl">
      <ProfileToken>{}</ProfileToken>
      <Velocity>
        <PanTilt x="{}" y="{}" space="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace" xmlns="http://www.onvif.org/ver10/schema"/>
        <Zoom x="{}" space="http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace" xmlns="http://www.onvif.org/ver10/schema"/>
      </Velocity>
    </ContinuousMove>"#,
        token, x, y, zoom
    );
    let action = "http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove";

    send_soap(&ptz_url, &body, &user, &pass, action, "ptz:continuous_move")
        .await
        .map(|_| ())
}

#[tauri::command]
pub async fn ptz_stop(
    ip: String,
    port: u16,
    user: String,
    pass: String,
) -> Result<(), String> {
    let token = get_first_profile_token(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            println!("Using default Profile Token: Profile_1");
            "Profile_1".to_string()
        });

    let ptz_url = get_ptz_service_url(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            let alt_port = candidate_ports(&ip, port)
                .into_iter()
                .find(|p| *p != port)
                .unwrap_or(port);
            let fallback = format!("http://{}:{}/onvif/ptz_service", ip, alt_port);
            println!("Using fallback PTZ URL: {}", fallback);
            fallback
        });

    let body = format!(
        r#"<Stop xmlns="http://www.onvif.org/ver20/ptz/wsdl">
      <ProfileToken>{}</ProfileToken>
      <PanTilt>true</PanTilt>
      <Zoom>true</Zoom>
    </Stop>"#,
        token
    );
    let action = "http://www.onvif.org/ver20/ptz/wsdl/Stop";

    send_soap(&ptz_url, &body, &user, &pass, action, "ptz:stop")
        .await
        .map(|_| ())
}

#[tauri::command]
pub async fn ptz_focus(
    ip: String,
    port: u16,
    user: String,
    pass: String,
    speed: f32,
) -> Result<(), String> {
    let video_token = get_video_source_token(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            println!("Using default Video Source Token: VideoSource_1");
            "VideoSource_1".to_string()
        });

    let imaging_url = get_imaging_service_url(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            let alt_port = candidate_ports(&ip, port)
                .into_iter()
                .find(|p| *p != port)
                .unwrap_or(port);
            let fallback = format!("http://{}:{}/onvif/imaging_service", ip, alt_port);
            println!("Using fallback Imaging URL: {}", fallback);
            fallback
        });

    if let Err(err) = apply_focus_mode(&imaging_url, &video_token, &user, &pass, "MANUAL").await {
        println!("Failed to switch camera to manual focus mode: {}", err);
    }

        let action = "http://www.onvif.org/ver20/imaging/wsdl/Move";
        let clamped = speed.clamp(-1.0, 1.0);
        if clamped.abs() < 0.01 {
                return Ok(());
        }

        // Many ONVIF cameras expect Relative focus moves; scale UI speed into a safe distance step.
        let distance = (clamped * 0.3).clamp(-1.0, 1.0);
        let relative_speed = clamped.abs().max(0.05);

        let relative_body = format!(
                r#"<Move xmlns="http://www.onvif.org/ver20/imaging/wsdl" xmlns:tt="http://www.onvif.org/ver10/schema">
            <VideoSourceToken>{}</VideoSourceToken>
            <Focus>
                <tt:Relative>
                    <tt:Distance>{}</tt:Distance>
                    <tt:Speed>{}</tt:Speed>
                </tt:Relative>
            </Focus>
        </Move>"#,
                video_token, distance, relative_speed
        );

        match send_soap(
                &imaging_url,
                &relative_body,
                &user,
                &pass,
                action,
                "imaging:focus_move:relative",
        )
        .await
        {
                Ok(_) => Ok(()),
                Err(relative_err) => {
                        println!(
                                "Relative focus move failed ({}), falling back to continuous move",
                                relative_err
                        );
                        let continuous_body = format!(
                                r#"<Move xmlns="http://www.onvif.org/ver20/imaging/wsdl" xmlns:tt="http://www.onvif.org/ver10/schema">
            <VideoSourceToken>{}</VideoSourceToken>
            <Focus>
                <tt:Continuous>
                    <tt:Speed>{}</tt:Speed>
                </tt:Continuous>
            </Focus>
        </Move>"#,
                                video_token, clamped
                        );

                        send_soap(
                                &imaging_url,
                                &continuous_body,
                                &user,
                                &pass,
                                action,
                                "imaging:focus_move:continuous",
                        )
                        .await
                        .map(|_| ())
                        .map_err(|continuous_err| {
                                println!(
                                        "Continuous focus move fallback failed as well: {}",
                                        continuous_err
                                );
                                continuous_err
                        })
                }
        }
}

#[tauri::command]
pub async fn focus_stop(
    ip: String,
    port: u16,
    user: String,
    pass: String,
) -> Result<(), String> {
    let video_token = get_video_source_token(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| "VideoSource_1".to_string());

    let imaging_url = get_imaging_service_url(&ip, port, &user, &pass)
        .await
        .unwrap_or_else(|_| {
            let alt_port = candidate_ports(&ip, port)
                .into_iter()
                .find(|p| *p != port)
                .unwrap_or(port);
            format!("http://{}:{}/onvif/imaging_service", ip, alt_port)
        });

    let body = format!(
        r#"<Stop xmlns="http://www.onvif.org/ver20/imaging/wsdl">
      <VideoSourceToken>{}</VideoSourceToken>
    </Stop>"#,
        video_token
    );
    let action = "http://www.onvif.org/ver20/imaging/wsdl/Stop";

    send_soap(&imaging_url, &body, &user, &pass, action, "imaging:focus_stop")
        .await
        .map(|_| ())
}
