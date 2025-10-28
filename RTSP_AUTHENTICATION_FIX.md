# RTSP Authentication Fix

## Problem Description
The application was encountering an error when trying to play RTSP streams with credentials containing special characters:

```
Error: invalid args `payload` for command `play_direct_rtsp`: command play_direct_rtsp missing required key payload
```

Additionally, even after fixing this error, the application failed to properly handle RTSP URLs with special characters (like '@', '/', spaces) in usernames or passwords.

## Root Causes

### Issue 1: Parameter Mismatch
The first issue was a parameter mismatch between the frontend and backend:

1. In `DirectRTSPPlayer.tsx`, the function was being called with:
   ```typescript
   fixedUrl = await invoke('play_direct_rtsp', { sdp: src }) as string;
   ```

2. While in `lib.rs`, the function was expecting a structure named `payload`:
   ```rust
   async fn play_direct_rtsp(payload: PlayDirectRtspPayload) -> Result<String, String> {
       let url = payload.sdp;
       // ...
   }
   ```

### Issue 2: Incorrect URL Parsing with Special Characters
The second issue was that the `fix_rtsp_url` function didn't correctly handle URLs with multiple '@' symbols or other special characters in the credentials.

## Solutions

### Solution 1: Fix Parameter Mismatch
The backend function signature was changed to directly accept the `sdp` parameter:

```rust
#[tauri::command]
async fn play_direct_rtsp(sdp: String) -> Result<String, String> {
    let url = sdp;
    // ...
}
```

### Solution 2: Improve URL Parsing and Encoding
The `fix_rtsp_url` function in `rtsp_utils.rs` was rewritten to:

1. Correctly identify the last '@' symbol as the separator between credentials and host
2. Properly URL-encode usernames and passwords with special characters
3. Validate the final URL to ensure it's correctly formatted

```rust
pub fn fix_rtsp_url(input_url: &str) -> Result<String, String> {
    // ... implementation details ...
    
    // Find the last @ to separate auth from host
    // This handles cases where there are @ symbols in the username or password
    let mut last_at_pos = None;
    
    // Find the rightmost @ symbol (should be the auth/host delimiter)
    for (i, c) in remainder.chars().enumerate() {
        if c == '@' {
            last_at_pos = Some(i);
        }
    }
    
    // ... URL encoding and validation ...
}
```

## Expected Results
The fix ensures:
1. Proper parameter passing between frontend and backend
2. Correct handling of special characters in RTSP credentials
3. Consistent URL formatting for all RTSP streams

## Notes
- URLs with special characters in credentials are now correctly URL-encoded
- The solution maintains backward compatibility with existing code
- Error handling is improved with more descriptive error messages