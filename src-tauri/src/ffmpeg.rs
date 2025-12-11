use std::process::{Command, Stdio};
use tauri::command;

use crate::resolve_ffmpeg_command;

#[command]
pub async fn start_stream(_rtsp_url: String, _output_path: String) -> Result<(), String> {
    // Deprecated: streaming is handled by go2rtc. Register streams through add_camera_streams instead.
    Err("start_stream is deprecated; use add_camera_streams/go2rtc helpers instead".into())
}

#[command]
pub async fn stop_stream() -> Result<(), String> {
    Ok(())
}

// Import the HTTP server functionality so we can use it in play_recording
use crate::http_server;

#[command]
pub async fn play_recording(file_path: String) -> Result<String, String> {
    use chrono::Utc;
    use std::thread;
    use tokio::task;

    println!("Play recording called with path: {}", file_path);

    // Проверка входных данных
    if !file_path.starts_with("rtsp://") {
        return Err(format!("Invalid RTSP URL: {}", file_path));
    }

    // First ensure that the HTTP server is running before we start generating HLS content
    match http_server::check_http_server().await {
        Ok(true) => println!("HTTP server is already running, continuing with HLS conversion"),
        _ => {
            println!("HTTP server not detected, starting it...");
            http_server::start_http_server().await?;
        }
    }

    let (hw_preference, quality_setting, fps_setting) =
        match super::get_app_settings_internal().await {
            Ok(settings) => {
                let hw_pref = settings
                    .get("hwAccel")
                    .and_then(|v| v.as_str())
                    .unwrap_or("auto")
                    .to_string();

                let qscale_value = settings
                    .get("qscale")
                    .and_then(|v| v.as_i64())
                    .map(|v| v.clamp(2, 31) as u32)
                    .unwrap_or(8);

                let fps_value = settings
                    .get("fps")
                    .and_then(|v| v.as_i64())
                    .map(|v| v.clamp(5, 60) as u32)
                    .unwrap_or(20);

                (hw_pref, qscale_value, fps_value)
            }
            Err(err) => {
                println!(
                    "Failed to load app settings ({}), falling back to defaults",
                    err
                );
                ("auto".to_string(), 8, 20)
            }
        };

    println!(
        "Hardware acceleration preference: {}, streaming quality qscale={}, fps={}",
        hw_preference, quality_setting, fps_setting
    );

    // Выполняем блокирующие операции в отдельном потоке через tokio::task::spawn_blocking
    // Это поможет избежать ошибок типа "Cannot drop a runtime in a context where blocking is not allowed"
    let result = task::spawn_blocking(move || -> Result<String, String> {
        // Создаем базовую директорию для временных файлов
    let temp_dir = crate::default_recordings_dir().join("streams");
    println!("Using HLS output directory: {:?}", temp_dir);

        if !temp_dir.exists() {
            println!("Creating temp directory");
            std::fs::create_dir_all(&temp_dir).map_err(|e| {
                println!("Failed to create temp directory: {}", e);
                e.to_string()
            })?;
        } else {
            println!("Temp directory already exists");
        }

        // Директория temp_dir уже создана выше

        // Генерируем уникальное имя файла для HLS вывода
        let timestamp = Utc::now().timestamp();
        let output_name = format!("stream_{}.m3u8", timestamp);
        let output_path = temp_dir.join(&output_name);
        println!("Output path: {:?}", output_path);
        let _output_path_str = output_path.to_str().unwrap().to_string(); // Сохраняем с подчеркиванием для обозначения неиспользуемой переменной

        // Проверяем наличие FFmpeg
        let ffmpeg_cmd = resolve_ffmpeg_command();
        println!("Using FFmpeg command: {}", ffmpeg_cmd);

        // Сначала проверим, есть ли ffmpeg в системе
        println!("Checking if FFmpeg is installed");
        let mut version_cmd = Command::new(&ffmpeg_cmd);
        version_cmd
            .arg("-version")
            .stdout(Stdio::piped())
            .stderr(Stdio::piped());

        // На Windows скрываем консольное окно
        #[cfg(windows)]
        {
            use std::os::windows::process::CommandExt;
            version_cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW
        }

        let status = version_cmd.status();

        match &status {
            Ok(exit_status) => {
                if exit_status.success() {
                    println!("FFmpeg is installed and available");
                } else {
                    println!("FFmpeg check failed with exit code: {:?}", exit_status.code());
                }
            }
            Err(e) => println!("Failed to check FFmpeg: {}", e),
        }

        if status.is_err() || !status.unwrap().success() {
            return Err("FFmpeg не установлен или не найден в PATH. Установите FFmpeg для конвертации RTSP потоков.".to_string());
        }

    let hw_decision = determine_hw_accel_strategy(&ffmpeg_cmd, &hw_preference);
        println!("{}", hw_decision.message);
        println!("FFmpeg video encoder selected: {}", hw_decision.config.video_codec);

        // Запускаем FFmpeg в отдельном потоке
        let file_path_clone = file_path.clone();
        let temp_dir_clone = temp_dir.clone();
        let output_path_clone = output_path.to_str().unwrap().to_string();
        let hw_config_for_thread = hw_decision.config.clone();
    let stream_quality = quality_setting;
    let stream_fps = fps_setting;

        thread::spawn(move || {
            // Создаем строку для сегментов заранее, чтобы она жила достаточно долго
            let segment_path = format!("{}/segment_%03d.ts", temp_dir_clone.to_str().unwrap());

            // Декодируем URL для FFmpeg, чтобы избежать двойного кодирования
            // FFmpeg имеет особенности в обработке URL с кодированными символами
            let decoded_url = if file_path_clone.contains('%') {
                urlencoding::decode(&file_path_clone)
                    .unwrap_or(file_path_clone.clone().into())
                    .to_string()
            } else {
                file_path_clone.clone()
            };

            let mut ffmpeg_args: Vec<String> = Vec::new();
            ffmpeg_args.extend(hw_config_for_thread.pre_input.clone());
            ffmpeg_args.push("-rtsp_transport".into());
            ffmpeg_args.push("tcp".into());
            ffmpeg_args.push("-i".into());
            ffmpeg_args.push(decoded_url);
            ffmpeg_args.push("-c:v".into());
            ffmpeg_args.push(hw_config_for_thread.video_codec.to_string());
            ffmpeg_args.extend(hw_config_for_thread.video_args.clone());
            // Apply quality controls based on selected encoder
            if stream_quality > 0 {
                match hw_config_for_thread.video_codec.as_str() {
                    codec if codec.contains("nvenc") => {
                        ffmpeg_args.push("-cq".into());
                        ffmpeg_args.push(stream_quality.to_string());
                    }
                    codec if codec.contains("qsv") => {
                        ffmpeg_args.push("-global_quality".into());
                        ffmpeg_args.push(stream_quality.to_string());
                    }
                    _ => {
                        ffmpeg_args.push("-crf".into());
                        ffmpeg_args.push(stream_quality.to_string());
                    }
                }
            }
            if !hw_config_for_thread
                .video_args
                .iter()
                .any(|arg| arg == "-preset")
            {
                ffmpeg_args.push("-preset".into());
                ffmpeg_args.push("ultrafast".into());
            }
            if !hw_config_for_thread
                .video_args
                .iter()
                .any(|arg| arg == "-tune")
            {
                if hw_config_for_thread.video_codec == "libx264" {
                    ffmpeg_args.push("-tune".into());
                    ffmpeg_args.push("zerolatency".into());
                } else if hw_config_for_thread.video_codec.contains("nvenc") {
                    ffmpeg_args.push("-tune".into());
                    ffmpeg_args.push("ll".into());
                }
            }
            if stream_fps > 0 {
                ffmpeg_args.push("-r".into());
                ffmpeg_args.push(stream_fps.to_string());
            }
            ffmpeg_args.push("-c:a".into());
            ffmpeg_args.push("aac".into());
            ffmpeg_args.push("-ar".into());
            ffmpeg_args.push("48000".into());
            ffmpeg_args.push("-ac".into());
            ffmpeg_args.push("2".into());
            ffmpeg_args.push("-f".into());
            ffmpeg_args.push("hls".into());
            ffmpeg_args.push("-hls_time".into());
            ffmpeg_args.push("2".into());
            ffmpeg_args.push("-hls_list_size".into());
            ffmpeg_args.push("5".into());
            ffmpeg_args.push("-hls_flags".into());
            ffmpeg_args.push("delete_segments".into());
            ffmpeg_args.push("-hls_segment_type".into());
            ffmpeg_args.push("mpegts".into());
            ffmpeg_args.push("-hls_init_time".into());
            ffmpeg_args.push("1".into());
            ffmpeg_args.push("-hls_segment_filename".into());
            ffmpeg_args.push(segment_path.clone());
            ffmpeg_args.push(output_path_clone.clone());

            println!(
                "Запуск FFmpeg для конвертации RTSP -> HLS с аргументами: {:?}",
                ffmpeg_args
            );

            let mut cmd = Command::new(&ffmpeg_cmd);
            cmd.args(&ffmpeg_args)
                .stdin(Stdio::null())
                .stdout(Stdio::piped())
                .stderr(Stdio::piped());

            // На Windows скрываем консольное окно
            #[cfg(windows)]
            {
                use std::os::windows::process::CommandExt;
                cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW
            }

            match cmd.spawn() {
                Ok(mut child) => {
                    // Читаем вывод FFmpeg для отладки
                    if let Some(stderr) = child.stderr.take() {
                        std::thread::spawn(move || {
                            use std::io::{BufRead, BufReader};
                            let reader = BufReader::new(stderr);
                            for line in reader.lines() {
                                if let Ok(l) = line {
                                    println!("[ffmpeg] {}", l);
                                }
                            }
                        });
                    }

                    // Не ждем завершения процесса, так как он будет работать пока есть поток
                    println!("FFmpeg запущен с PID: {:?}", child.id());
                }
                Err(e) => eprintln!("Ошибка запуска FFmpeg: {}", e),
            }
        });

        // Ждем немного, чтобы FFmpeg успел запуститься и создать первые сегменты
        println!("Waiting for FFmpeg to start and create initial segments");

        // Используем standard library sleep вместо tokio sleep, чтобы избежать проблем с рантаймом
        std::thread::sleep(std::time::Duration::from_secs(2));

        // Проверяем, что выходной файл был создан или создается
        // Также проверяем временные файлы .tmp, которые FFmpeg может создавать
        let tmp_file_path = format!("{}.tmp", output_path.to_str().unwrap());
        let tmp_file_exists = std::fs::File::open(&tmp_file_path).is_ok();
        let file_exists = std::fs::File::open(&output_path).is_ok();

        if file_exists {
            println!("HLS manifest file created successfully");
        } else if tmp_file_exists {
            println!("HLS manifest file is being created (found .tmp file)");
            // Дадим немного больше времени для конвертации .tmp в финальный файл
            std::thread::sleep(std::time::Duration::from_secs(1));
        } else {
            println!(
                "WARNING: HLS manifest file not found at expected location: {:?}",
                output_path
            );

            // Проверим содержимое каталога для отладки
            match std::fs::read_dir(&temp_dir) {
                Ok(entries) => {
                    println!("Files in temp directory:");
                    let mut entries_vec = Vec::new();
                    for entry in entries {
                        if let Ok(entry) = entry {
                            entries_vec.push(entry.path());
                        }
                    }

                    // Сортируем файлы по времени модификации (новые в начале)
                    entries_vec.sort_by(|a, b| {
                        let time_a = std::fs::metadata(a).and_then(|m| m.modified()).ok();
                        let time_b = std::fs::metadata(b).and_then(|m| m.modified()).ok();
                        time_b.cmp(&time_a)
                    });

                    // Выводим только первые 10 самых новых файлов для читаемости
                    for (_i, path) in entries_vec.iter().take(10).enumerate() {
                        println!("  - {:?}", path);
                    }
                    if entries_vec.len() > 10 {
                        println!("  ... and {} more files", entries_vec.len() - 10);
                    }
                }
                Err(e) => println!("Failed to read temp directory: {}", e),
            }
        }

        // Ensure the HTTP server is running to serve HLS content
        use reqwest::blocking::Client;
        use std::time::Duration;

        // Instead of just checking, we'll ensure the HTTP server is started
        // We can't directly call the async function from this blocking context,
        // so we'll check first and print warning if not available

        let client = Client::new();
        let server_check = client
            .get("http://localhost:8080/health")
            .timeout(Duration::from_secs(1))
            .send();

        match server_check {
            Ok(_) => {
                println!("HTTP server on port 8080 is available");
            }
            Err(e) => {
                println!(
                    "Warning: HTTP server not available: {}. Starting HTTP server in background...",
                    e
                );
                // We can't start it directly here since we're in a blocking context,
                // but we'll alert the user to ensure it's started from the frontend
            }
        }

        // Возвращаем URL для доступа к HLS стриму через локальный HTTP сервер
        // Поскольку мы используем системный временный каталог вместо app_local_data_dir,
        // создадим отдельный маршрут и самостоятельно обслуживаем файлы
        let http_url = format!("http://localhost:8080/streams/{}", output_name);
        println!("HLS stream URL: {}", http_url);
        println!("NOTE: Make sure http server is running on port 8080 to serve HLS content");

        // Пробуем использовать go2rtc HLS URL как альтернативный вариант, если доступно
        if let Some(camera_name) = file_path
            .strip_prefix("rtsp://localhost:8554/")
            .or_else(|| file_path.strip_prefix("rtsp://127.0.0.1:8554/"))
        {
            let normalized = camera_name.trim_matches('/');
            if !normalized.is_empty() {
                let encoded = urlencoding::encode(normalized);
                let go2rtc_hls_url = format!(
                    "http://127.0.0.1:1984/api/hls/{}/index.m3u8",
                    encoded
                );
                println!("Alternative go2rtc HLS URL: {}", go2rtc_hls_url);
                return Ok(go2rtc_hls_url);
            }
        }

        // Проверяем, есть ли какие-либо существующие файлы m3u8 в каталоге
        // Если наш файл не создан, но есть другие свежие m3u8 файлы, используем самый новый из них
        if !std::fs::File::open(&output_path).is_ok() {
            if let Ok(entries) = std::fs::read_dir(&temp_dir) {
                let mut m3u8_files = Vec::new();
                for entry in entries.filter_map(Result::ok) {
                    if let Some(name) = entry.file_name().to_str() {
                        if name.ends_with(".m3u8") && !name.ends_with(".m3u8.tmp") {
                            m3u8_files.push(entry.path());
                        }
                    }
                }

                // Сортируем по времени модификации (новые в начале)
                m3u8_files.sort_by(|a, b| {
                    let time_a = std::fs::metadata(a).and_then(|m| m.modified()).ok();
                    let time_b = std::fs::metadata(b).and_then(|m| m.modified()).ok();
                    time_b.cmp(&time_a)
                });

                // Используем самый новый файл, если он есть
                if let Some(newest_m3u8) = m3u8_files.first() {
                    if let Some(file_name) = newest_m3u8.file_name() {
                        if let Some(name_str) = file_name.to_str() {
                            println!("Using existing m3u8 file: {}", name_str);
                            let alternative_url = format!(
                                "http://localhost:8080/streams/{}",
                                name_str
                            );
                            return Ok(alternative_url);
                        }
                    }
                }
            }
        }

        Ok(http_url)
    })
    .await
    .map_err(|e| format!("Task join error: {}", e))??;

    Ok(result)
}

#[derive(Clone, Debug)]
pub(crate) struct HwAccelConfig {
    /// Arguments that must appear before the input (decoder selection, device setup, etc.)
    pub(crate) pre_input: Vec<String>,
    /// Video encoder/decoder codec flag value for `-c:v`
    pub(crate) video_codec: String,
    /// Extra arguments that follow the `-c:v <codec>` pair
    pub(crate) video_args: Vec<String>,
}

#[derive(Clone, Debug)]
pub(crate) struct HwAccelDecision {
    /// Human readable description of the selected strategy
    pub(crate) message: String,
    /// The effective configuration to apply
    pub(crate) config: HwAccelConfig,
}

impl HwAccelConfig {
    pub(crate) fn software_default() -> Self {
        Self {
            pre_input: vec![],
            video_codec: "libx264".to_string(),
            video_args: vec![],
        }
    }
}

pub(crate) fn determine_hw_accel_strategy(ffmpeg_cmd: &str, preference: &str) -> HwAccelDecision {
    let preference_lc = preference.to_ascii_lowercase();
    let base_config = HwAccelConfig::software_default();

    match preference_lc.as_str() {
        "nvidia" => {
            let nv_config = HwAccelConfig {
                pre_input: vec![
                    "-hwaccel".into(),
                    "cuda".into(),
                    "-hwaccel_output_format".into(),
                    "cuda".into(),
                ],
                video_codec: "h264_nvenc".into(),
                video_args: vec!["-preset".into(), "llhp".into()],
            };

            if hwprobe::check_nvenc_support(ffmpeg_cmd) {
                return HwAccelDecision {
                    message: "Using NVIDIA NVENC hardware acceleration".into(),
                    config: nv_config,
                };
            }

            HwAccelDecision {
                message: "NVENC not available; falling back to software encoding".into(),
                config: base_config,
            }
        }
        "intel" => {
            let qsv_config = HwAccelConfig {
                pre_input: vec!["-hwaccel".into(), "qsv".into()],
                video_codec: "h264_qsv".into(),
                video_args: vec!["-preset".into(), "veryfast".into()],
            };

            if hwprobe::check_qsv_support(ffmpeg_cmd) {
                return HwAccelDecision {
                    message: "Using Intel QuickSync hardware acceleration".into(),
                    config: qsv_config,
                };
            }

            HwAccelDecision {
                message: "QuickSync not available; falling back to software encoding".into(),
                config: base_config,
            }
        }
        "none" => HwAccelDecision {
            message: "Hardware acceleration disabled by user; using software encoding".into(),
            config: base_config,
        },
        _ => {
            // Auto-detect preference: try NVIDIA first, then Intel
            if hwprobe::check_nvenc_support(ffmpeg_cmd) {
                let nv_config = HwAccelConfig {
                    pre_input: vec![
                        "-hwaccel".into(),
                        "cuda".into(),
                        "-hwaccel_output_format".into(),
                        "cuda".into(),
                    ],
                    video_codec: "h264_nvenc".into(),
                    video_args: vec!["-preset".into(), "llhq".into()],
                };

                HwAccelDecision {
                    message: "Auto-detected NVIDIA NVENC hardware acceleration".into(),
                    config: nv_config,
                }
            } else if hwprobe::check_qsv_support(ffmpeg_cmd) {
                let qsv_config = HwAccelConfig {
                    pre_input: vec!["-hwaccel".into(), "qsv".into()],
                    video_codec: "h264_qsv".into(),
                    video_args: vec!["-preset".into(), "veryfast".into()],
                };

                HwAccelDecision {
                    message: "Auto-detected Intel QuickSync hardware acceleration".into(),
                    config: qsv_config,
                }
            } else {
                HwAccelDecision {
                    message: "No supported hardware acceleration found; using software encoding"
                        .into(),
                    config: base_config,
                }
            }
        }
    }
}

mod hwprobe {
    use std::process::{Command, Stdio};

    fn run_probe(ffmpeg_cmd: &str, args: &[&str]) -> Option<String> {
        let mut cmd = Command::new(ffmpeg_cmd);
        cmd.args(args)
            .stdin(Stdio::null())
            .stdout(Stdio::piped())
            .stderr(Stdio::piped());

        #[cfg(windows)]
        {
            use std::os::windows::process::CommandExt;
            cmd.creation_flags(0x08000000); // CREATE_NO_WINDOW
        }

        match cmd.output() {
            Ok(output) => {
                let mut combined = String::from_utf8_lossy(&output.stdout).to_string();
                combined.push_str(&String::from_utf8_lossy(&output.stderr));
                Some(combined)
            }
            Err(_) => None,
        }
    }

    pub fn check_nvenc_support(ffmpeg_cmd: &str) -> bool {
        let hwaccels = run_probe(ffmpeg_cmd, &["-hide_banner", "-hwaccels"])
            .unwrap_or_default()
            .to_ascii_lowercase();
        if !(hwaccels.contains("cuda") || hwaccels.contains("nvdec")) {
            return false;
        }

        let encoders = run_probe(ffmpeg_cmd, &["-hide_banner", "-encoders"])
            .unwrap_or_default()
            .to_ascii_lowercase();
        encoders.contains("h264_nvenc") || encoders.contains("hevc_nvenc")
    }

    pub fn check_qsv_support(ffmpeg_cmd: &str) -> bool {
        let hwaccels = run_probe(ffmpeg_cmd, &["-hide_banner", "-hwaccels"])
            .unwrap_or_default()
            .to_ascii_lowercase();
        if !hwaccels.contains("qsv") {
            return false;
        }

        let encoders = run_probe(ffmpeg_cmd, &["-hide_banner", "-encoders"])
            .unwrap_or_default()
            .to_ascii_lowercase();
        encoders.contains("h264_qsv") || encoders.contains("hevc_qsv")
    }
}
