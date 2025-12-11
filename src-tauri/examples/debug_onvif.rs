use reqwest::header::{
    HeaderMap, HeaderValue, ACCEPT, CONNECTION, CONTENT_TYPE, EXPECT, USER_AGENT,
};
use std::time::Duration;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let url = "http://192.168.3.11:8899/onvif/device_service";
    let soap_xml = r#"<?xml version="1.0" encoding="UTF-8"?><s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:tds="http://www.onvif.org/ver10/device/wsdl"><s:Body><tds:GetCapabilities><tds:Category>All</tds:Category></tds:GetCapabilities></s:Body></s:Envelope>"#;

    println!("Sending request to: {}", url);

    let mut headers = HeaderMap::new();
    headers.insert(
        CONTENT_TYPE,
        HeaderValue::from_static("application/x-www-form-urlencoded"),
    );
    headers.insert(USER_AGENT, HeaderValue::from_static("curl/8.14.1"));
    headers.insert(ACCEPT, HeaderValue::from_static("*/*"));
    headers.insert(CONNECTION, HeaderValue::from_static("close"));
    // Remove Expect header if it exists (reqwest might add it)
    headers.insert(EXPECT, HeaderValue::from_static(""));

    let client = reqwest::Client::builder()
        .timeout(Duration::from_secs(5))
        .danger_accept_invalid_certs(true)
        .http1_only()
        .build()?;

    let response = client
        .post(url)
        .headers(headers)
        .body(soap_xml.to_string()) // Use String to ensure Content-Length is calculated
        .send()
        .await;

    match response {
        Ok(resp) => {
            println!("Status: {}", resp.status());
            println!("Headers: {:#?}", resp.headers());
            let text = resp.text().await?;
            println!("Body: {}", text);
        }
        Err(e) => {
            println!("Error: {:?}", e);
        }
    }

    Ok(())
}
