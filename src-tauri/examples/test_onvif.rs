use base64::Engine;
use chrono::Utc;
use rand::Rng;
use reqwest::Client;
use sha1::{Digest, Sha1};
use std::time::Duration;

fn create_security_header(user: &str, pass: &str) -> String {
    let nonce_raw: [u8; 16] = rand::thread_rng().gen();
    let nonce = base64::engine::general_purpose::STANDARD.encode(&nonce_raw);
    let created = Utc::now().to_rfc3339_opts(chrono::SecondsFormat::Millis, true);

    let mut hasher = Sha1::new();
    hasher.update(&nonce_raw);
    hasher.update(created.as_bytes());
    hasher.update(pass.as_bytes());
    let password_digest = base64::engine::general_purpose::STANDARD.encode(hasher.finalize());

    format!(
        r#"<wsse:Security xmlns:wsse="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd" xmlns:wsu="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd"><wsse:UsernameToken wsu:Id="UsernameToken-1"><wsse:Username>{}</wsse:Username><wsse:Password Type="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd#PasswordDigest">{}</wsse:Password><wsse:Nonce EncodingType="http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd#Base64Binary">{}</wsse:Nonce><wsu:Created>{}</wsu:Created></wsse:UsernameToken></wsse:Security>"#,
        user, password_digest, nonce, created
    )
}

#[tokio::main]
async fn main() {
    let ip = "192.168.3.11";
    let port = 8899;
    let user = "admin";
    let pass = "123456";
    let url = format!("http://{}:{}/onvif/PTZ", ip, port);

    println!("Testing connection to {}", url);

    let client = Client::builder()
        .timeout(Duration::from_secs(5))
        .no_proxy()
        .http1_only()
        .tcp_nodelay(true)
        .user_agent("curl/8.14.1") // Mimic curl
        .build()
        .unwrap();

    let security_header = create_security_header(user, pass);

    let body = format!(
        r#"<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope"><s:Header>{}</s:Header><s:Body xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema"><ContinuousMove xmlns="http://www.onvif.org/ver20/ptz/wsdl"><ProfileToken>Profile_000</ProfileToken><Velocity><PanTilt x="0.5" y="0.0" space="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace" xmlns="http://www.onvif.org/ver10/schema"/></Velocity><Timeout>PT10S</Timeout></ContinuousMove></s:Body></s:Envelope>"#,
        security_header
    );

    println!("Sending request...");
    let res = client.post(&url)
        .header("Content-Type", "application/soap+xml; charset=utf-8; action=\"http://www.onvif.org/ver20/ptz/wsdl/ContinuousMove\"")
        .body(body)
        .send()
        .await;

    match res {
        Ok(response) => {
            println!("Status: {}", response.status());
            let text = response.text().await.unwrap_or_default();
            println!("Body: {}", text);
        }
        Err(e) => {
            println!("Error: {}", e);
        }
    }
}
