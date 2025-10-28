use base64::{engine::general_purpose::STANDARD as BASE64_STANDARD, Engine};
use minimp4::Mp4Muxer;
use scuffle_h265::SpsNALUnit;
use std::io::{Cursor, Seek, Write};

/// Raw parameter sets (VPS/SPS/PPS) extracted from an SDP description.
#[derive(Debug, Clone, Default)]
pub struct HevcParameterSets {
    /// Video parameter sets provided by the encoder.
    pub vps: Vec<Vec<u8>>,
    /// Sequence parameter sets. The first entry is used to determine resolution.
    pub sps: Vec<Vec<u8>>,
    /// Picture parameter sets.
    pub pps: Vec<Vec<u8>>,
}

/// Minimal HEVC stream metadata required to configure a muxer.
#[derive(Debug, Clone)]
pub struct HevcStreamDescriptor {
    /// Cropped frame width derived from the SPS.
    pub width: u32,
    /// Cropped frame height derived from the SPS.
    pub height: u32,
    /// Optional frame rate hint parsed from the SDP.
    pub fps: Option<f32>,
    /// Raw VPS/SPS/PPS blobs.
    pub parameter_sets: HevcParameterSets,
}

impl HevcStreamDescriptor {
    /// Returns a usable FPS value with reasonable defaults and limits.
    pub fn fps_or_default(&self) -> u32 {
        const DEFAULT_FPS: f32 = 25.0;
        const MIN_FPS: f32 = 1.0;
        const MAX_FPS: f32 = 240.0;

        let candidate = self
            .fps
            .filter(|value| value.is_finite() && *value > 0.1)
            .unwrap_or(DEFAULT_FPS);
        let clamped = candidate.clamp(MIN_FPS, MAX_FPS).round();
        clamped as u32
    }
}

/// Parse an SDP description and extract HEVC parameter sets together with basic metadata.
pub fn parse_hevc_sdp(sdp: &str) -> Result<HevcStreamDescriptor, String> {
    let mut sets = HevcParameterSets::default();
    let mut fps: Option<f32> = None;

    for line in sdp.lines() {
        let trimmed = line.trim();
        if trimmed.is_empty() {
            continue;
        }

        if let Some(value) = trimmed.strip_prefix("a=framerate:") {
            if fps.is_none() {
                fps = parse_fps_value(value);
            }
        }

        // We only care about fmtp lines that carry HEVC parameter sets.
        if !trimmed.starts_with("a=fmtp:") {
            continue;
        }

        // Normalise case when looking for sprop- parameters.
        let lower = trimmed.to_ascii_lowercase();
        if !lower.contains("sprop-vps=") {
            continue;
        }

        // Remove the payload type prefix (e.g. "a=fmtp:96 ").
        let without_prefix = trimmed.trim_start_matches("a=fmtp:").trim_start();
        let params_str = if let Some((_, rest)) = without_prefix.split_once(' ') {
            rest
        } else if let Some((_, rest)) = without_prefix.split_once(';') {
            rest
        } else {
            ""
        };

        if params_str.is_empty() {
            continue;
        }

        for part in params_str.split(';') {
            let trimmed_part = part.trim();
            if trimmed_part.is_empty() {
                continue;
            }

            let (key_raw, value_raw) = match trimmed_part.split_once('=') {
                Some(pair) => pair,
                None => continue,
            };
            let key = key_raw.trim().to_ascii_lowercase();
            let value = value_raw.trim();

            match key.as_str() {
                "sprop-vps" => decode_nalus(value, &mut sets.vps)?,
                "sprop-sps" => decode_nalus(value, &mut sets.sps)?,
                "sprop-pps" => decode_nalus(value, &mut sets.pps)?,
                "max-fps" | "fps" | "maxfr" | "max-fr" => {
                    if fps.is_none() {
                        fps = parse_fps_value(value);
                    }
                }
                _ => {}
            }
        }
    }

    if sets.vps.is_empty() || sets.sps.is_empty() || sets.pps.is_empty() {
        return Err("SDP is missing HEVC parameter sets".to_string());
    }

    let first_sps = &sets.sps[0];
    let parsed_sps = SpsNALUnit::parse(Cursor::new(first_sps))
        .map_err(|err| format!("Failed to parse SPS from SDP: {}", err))?;
    let width = parsed_sps.rbsp.cropped_width() as u32;
    let height = parsed_sps.rbsp.cropped_height() as u32;

    Ok(HevcStreamDescriptor {
        width,
        height,
        fps,
        parameter_sets: sets,
    })
}

/// Produce Annex B start-code prefixed parameter sets in VPS/SPS/PPS order.
pub fn annexb_parameter_sets(sets: &HevcParameterSets) -> Vec<u8> {
    let mut out = Vec::new();
    for group in [&sets.vps, &sets.sps, &sets.pps] {
        for nal in group {
            if nal.is_empty() {
                continue;
            }
            out.extend_from_slice(&[0x00, 0x00, 0x00, 0x01]);
            out.extend_from_slice(nal);
        }
    }
    out
}

/// Instantiate a minimp4 muxer pre-configured for HEVC video.
pub fn instantiate_muxer<W: Write + Seek>(
    writer: W,
    descriptor: &HevcStreamDescriptor,
    track_name: &str,
) -> Mp4Muxer<W> {
    let mut muxer = Mp4Muxer::new(writer);
    muxer.init_video(
        descriptor.width as i32,
        descriptor.height as i32,
        true,
        track_name,
    );

    let header = annexb_parameter_sets(&descriptor.parameter_sets);
    if !header.is_empty() {
        let fps = descriptor.fps_or_default().max(1);
        muxer.write_video_with_fps(&header, fps);
    }

    muxer
}

fn decode_nalus(value: &str, target: &mut Vec<Vec<u8>>) -> Result<(), String> {
    for chunk in value.split(',') {
        let trimmed = chunk.trim_matches(|c: char| c.is_whitespace() || c == '\r');
        if trimmed.is_empty() {
            continue;
        }
        let decoded = BASE64_STANDARD
            .decode(trimmed)
            .map_err(|err| format!("Failed to decode HEVC parameter set: {}", err))?;
        if decoded.is_empty() {
            continue;
        }
        if !target.iter().any(|existing| existing == &decoded) {
            target.push(decoded);
        }
    }
    Ok(())
}

fn parse_fps_value(raw: &str) -> Option<f32> {
    let trimmed = raw.trim();
    if trimmed.is_empty() {
        return None;
    }

    if let Some((num, den)) = trimmed.split_once('/') {
        let num: f32 = num.trim().parse().ok()?;
        let den: f32 = den.trim().parse().ok()?;
        if den.abs() > f32::EPSILON {
            return Some((num / den).abs());
        }
        return None;
    }

    let value: f32 = trimmed.parse().ok()?;
    if !value.is_finite() || value <= 0.0 {
        return None;
    }
    if value > 1000.0 {
        Some(value / 1000.0)
    } else {
        Some(value)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const VPS_BYTES: &[u8] = &[
        0x40, 0x01, 0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x78,
    ];
    const SPS_BYTES: &[u8] = &[
        0x42, 0x01, 0x01, 0x01, 0x40, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x78, 0xa0, 0x03, 0xc0, 0x80, 0x11, 0x07, 0xcb, 0x96, 0xb4, 0xa4, 0x25, 0x92,
        0xe3, 0x01, 0x6a, 0x02, 0x02, 0x02, 0x08, 0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x03,
        0x00, 0xf3, 0x00, 0x2e, 0xf2, 0x88, 0x00, 0x02, 0x62, 0x5a, 0x00, 0x00, 0x13, 0x12, 0xd0,
        0x20,
    ];
    const PPS_BYTES: &[u8] = &[0x44, 0x01, 0xc1, 0x73, 0xc8, 0xb0, 0x22, 0x11];

    #[test]
    fn parse_hevc_sdp_extracts_metadata() {
        let vps_b64 = BASE64_STANDARD.encode(VPS_BYTES);
        let sps_b64 = BASE64_STANDARD.encode(SPS_BYTES);
        let pps_b64 = BASE64_STANDARD.encode(PPS_BYTES);

        let sdp = format!(
            "v=0\r\nm=video 0 RTP/AVP 96\r\na=rtpmap:96 H265/90000\r\na=fmtp:96 sprop-vps={};sprop-sps={};sprop-pps={};max-fps=30\r\na=control:streamid=0\r\n",
            vps_b64, sps_b64, pps_b64
        );

        let descriptor = parse_hevc_sdp(&sdp).expect("descriptor");
        assert_eq!(descriptor.width, 1920);
        assert_eq!(descriptor.height, 1080);
        assert_eq!(descriptor.fps, Some(30.0));
        assert_eq!(descriptor.parameter_sets.vps.len(), 1);
        assert_eq!(descriptor.parameter_sets.sps.len(), 1);
        assert_eq!(descriptor.parameter_sets.pps.len(), 1);

        let annexb = annexb_parameter_sets(&descriptor.parameter_sets);
        // Expect three start codes.
        assert_eq!(
            annexb
                .windows(4)
                .filter(|chunk| *chunk == [0, 0, 0, 1])
                .count(),
            3
        );

        let muxer = instantiate_muxer(std::io::Cursor::new(Vec::new()), &descriptor, "test");
        let writer = muxer.close();
        assert!(!writer.get_ref().is_empty());
    }

    #[test]
    fn parse_fps_variants() {
        assert_eq!(parse_fps_value("30"), Some(30.0));
        assert_eq!(
            parse_fps_value("30000/1001").map(|v| (v * 100.0).round() / 100.0),
            Some(29.97)
        );
        assert_eq!(parse_fps_value("30000"), Some(30.0));
        assert_eq!(parse_fps_value(""), None);
    }
}
