use url::Url;

/// Utility function to fix common issues with RTSP URLs
pub fn fix_rtsp_url(input_url: &str) -> Result<String, String> {
    // Check if input is a valid RTSP URL
    if !input_url.starts_with("rtsp://") {
        return Err("URL must start with rtsp://".to_string());
    }

    println!("Fixing RTSP URL: Input URL starts with rtsp://");

    // Проверяем, есть ли уже закодированные символы в URL (например, %40 вместо @)
    // Если да, то сначала декодируем URL, чтобы избежать двойного кодирования
    let decoded_url = if input_url.contains('%') {
        println!(
            "URL contains percent-encoded characters, decoding first to avoid double-encoding"
        );
        match urlencoding::decode(input_url) {
            Ok(decoded) => {
                println!("Successfully decoded URL");
                decoded.to_string()
            }
            Err(e) => {
                println!("Failed to decode URL: {}, using original", e);
                input_url.to_string() // Если декодирование не удалось, используем исходный URL
            }
        }
    } else {
        println!("URL doesn't contain percent-encoded characters, using as-is");
        input_url.to_string()
    };

    // Теперь разбираем URL на части
    // First, separate the scheme from the rest
    let parts: Vec<&str> = decoded_url.splitn(2, "://").collect();
    if parts.len() != 2 {
        return Err("Invalid URL format".to_string());
    }

    let scheme = parts[0]; // "rtsp"
    let remainder = parts[1];

    // Find the last @ to separate auth from host
    // This handles cases where there are @ symbols in the username or password
    let mut last_at_pos = None;

    // Find the rightmost @ symbol (should be the auth/host delimiter)
    for (i, c) in remainder.chars().enumerate() {
        if c == '@' {
            last_at_pos = Some(i);
        }
    }

    // Process auth and host parts
    let (auth, host_and_path) = if let Some(idx) = last_at_pos {
        (&remainder[0..idx], &remainder[idx + 1..])
    } else {
        ("", remainder) // No auth part
    };

    // If we have authentication info, parse and encode it
    let fixed_url = if !auth.is_empty() {
        // Split auth into username and password at first colon
        let (username, password) = if let Some(idx) = auth.find(':') {
            (&auth[0..idx], &auth[idx + 1..])
        } else {
            (auth, "") // No password
        };

        // Проверяем, содержит ли пароль специальные символы, которые требуют кодирования
        println!("Processing auth: username and password found");

        // Кодируем только самые проблемные символы, чтобы минимизировать риск ошибок
        // ВАЖНО: мы должны быть осторожны с @ символом, который разделяет
        // аутентификационную часть и адресную часть URL
        let encoded_username =
            if username.contains('@') || username.contains(' ') || username.contains('/') {
                println!("Username contains special characters, encoding them");
                username
                    .replace('@', "%40")
                    .replace(' ', "%20")
                    .replace('/', "%2F")
            } else {
                username.to_string()
            };

        // Для пароля мы кодируем больше специальных символов, так как в них чаще встречаются спецсимволы
        let encoded_password = if password.contains('@')
            || password.contains(' ')
            || password.contains('/')
            || password.contains(':')
        {
            println!("Password contains special characters, encoding them");
            password
                .replace('@', "%40")
                .replace('/', "%2F")
                .replace(' ', "%20")
                .replace(':', "%3A")
                .replace('&', "%26")
                .replace('#', "%23")
        } else {
            password.to_string()
        };

        format!(
            "{}://{}:{}@{}",
            scheme, encoded_username, encoded_password, host_and_path
        )
    } else {
        format!("{}://{}", scheme, host_and_path)
    };

    // Валидируем URL но не применяем стандартное URL-кодирование,
    // так как FFmpeg имеет особенности в работе с кодированными URL
    let valid = match Url::parse(&fixed_url) {
        Ok(_) => true,
        Err(_) => false,
    };

    if valid {
        Ok(fixed_url)
    } else {
        // Если URL все еще неверный, вернем исходный
        println!(
            "Warning: URL validation failed after fixes, using original: {}",
            input_url
        );
        Ok(input_url.to_string())
    }
}
