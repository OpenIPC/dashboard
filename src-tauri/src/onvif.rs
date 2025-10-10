use regex::Regex;
use reqwest::Client;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct OnvifProfile {
    pub token: String,
    pub name: String,
}

#[tauri::command]
pub async fn get_rtsp_url(
    ip: String,
    port: u16,
    user: String,
    pass: String,
) -> Result<String, String> {
    // Быстро возвращаем стандартный URL без ONVIF запросов
    let u = urlencoding::encode(&user);
    let p = urlencoding::encode(&pass);
    let standard_url = format!("rtsp://{}:{}@{}:554/stream=0", u, p, ip);

    println!(
        "Using standard RTSP URL: rtsp://{}:****@{}:554/stream=0",
        u, ip
    );
    Ok(standard_url)
}

// Оставляем функцию для возможного использования в будущем, но не вызываем её
#[allow(dead_code)]
async fn get_onvif_profiles_async(
    ip: String,
    port: u16,
    user: String,
    pass: String,
) -> Result<Vec<OnvifProfile>, String> {
    let client = Client::builder()
        .timeout(std::time::Duration::from_secs(3))
        .build()
        .map_err(|e| format!("Failed to create HTTP client: {}", e))?;

    let soap_request = r#"<?xml version="1.0" encoding="UTF-8"?>
        <soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope" xmlns:trt="http://www.onvif.org/ver10/media/wsdl">
            <soap:Body>
                <trt:GetProfiles/>
            </soap:Body>
        </soap:Envelope>"#;

    let url = format!("http://{}:{}/onvif/media", ip, port);

    let response = client
        .post(&url)
        .basic_auth(&user, Some(&pass))
        .header("Content-Type", "application/soap+xml; charset=utf-8")
        .body(soap_request)
        .send()
        .await
        .map_err(|e| format!("Request failed: {}", e))?;

    if !response.status().is_success() {
        return Err(format!("HTTP error: {}", response.status()));
    }

    let body = response
        .text()
        .await
        .map_err(|e| format!("Failed to read response: {}", e))?;

    let profile_regex =
        Regex::new(r#"<trt:Profiles[^>]*token="([^"]+)"[^>]*>(?:.*?)<tt:Name>([^<]+)</tt:Name>"#)
            .map_err(|e| format!("Regex error: {}", e))?;

    let mut profiles = Vec::new();
    for cap in profile_regex.captures_iter(&body) {
        let token = cap[1].to_string();
        let name = cap[2].to_string();
        profiles.push(OnvifProfile { token, name });
    }

    if profiles.is_empty() {
        return Err("No profiles found".to_string());
    }

    Ok(profiles)
}
