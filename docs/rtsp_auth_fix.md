# RTSP Authentication Fix Documentation

## Problem Description

The dashboard application was experiencing issues with RTSP authentication when passwords contained special characters, particularly the `@` symbol. When users tried to connect to cameras using credentials with special characters, they would receive 401 Unauthorized errors even though the credentials were correct.

## Root Causes

We identified two main issues:

1. **Parameter Mismatch in API**: The frontend was using a parameter named `sdp` but the backend expected it in a struct with a field called `payload`.

2. **Double URL Encoding**: When a password contained special characters like `@` (which is encoded as `%40`), these characters were being encoded twice:
   - First encoding: `@` → `%40`
   - Second encoding: `%40` → `%2540`
   
   This double encoding made the credentials unrecognizable to the camera's authentication system.

## Solutions Implemented

### 1. Parameter Alignment

Modified the Rust backend function `play_direct_rtsp` to directly accept the `sdp` parameter instead of a payload struct.

```rust
// Before
#[tauri::command]
async fn play_direct_rtsp(payload: PlayDirectRtspPayload) -> Result<String, String> {
    let url = payload.sdp;
    // ...
}

// After
#[tauri::command]
async fn play_direct_rtsp(sdp: String) -> Result<String, String> {
    let url = sdp;
    // ...
}
```

### 2. URL Encoding Fix

Added proper handling of URL encoding in both the `rtsp_utils.rs` and `ffmpeg.rs` files:

1. **In `rtsp_utils.rs`**:
   - First decoded any already-encoded URL to avoid double encoding
   - Handled the `@` character properly in usernames and passwords
   - Applied encoding only where necessary for special characters

2. **In `ffmpeg.rs`**:
   - Added URL decoding for FFmpeg processing to handle special characters correctly
   - This prevents FFmpeg from receiving doubly encoded URLs

```rust
// In ffmpeg.rs
let decoded_url = if file_path_clone.contains('%') {
    urlencoding::decode(&file_path_clone).unwrap_or(file_path_clone.clone()).to_string()
} else {
    file_path_clone.clone()
};
```

3. **In `lib.rs`**:
   - Added explicit URL decoding before returning the fixed URL to the frontend
   ```rust
   let decoded_url = urlencoding::decode(&fixed_url).unwrap_or(fixed_url.clone()).to_string();
   Ok(decoded_url)
   ```

## Testing

To verify these fixes:

1. Test RTSP URLs with various special characters in passwords:
   - `@` symbol: `rtsp://admin:pass@word@192.168.1.100:554`
   - Spaces: `rtsp://admin:my password@192.168.1.100:554`
   - Other special characters: `rtsp://admin:pass$#%^&@192.168.1.100:554`

2. Check logs to ensure URLs are properly processed:
   - No double encoding occurs
   - Authentication succeeds with 200 OK responses instead of 401 Unauthorized

## Future Considerations

1. Consider implementing more robust URL parsing using dedicated libraries like `url-parser` for complex cases
2. Add more comprehensive logging of URL processing steps (with credentials masked)
3. Consider implementing a test suite for different RTSP URL formats with various special characters