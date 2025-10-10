# RTSP Authentication Fixes

This set of changes addresses issues with RTSP URL authentication in the Dashboard application.

## Problems Fixed

1. **Double @ Symbol in URLs** - When a username or password contained an @ symbol, it would break the URL parsing.
2. **Special Characters in Credentials** - Special characters in usernames and passwords weren't properly URL-encoded.
3. **Authentication Failures** - Better error handling for authentication-related issues.

## Key Changes

### Backend (Rust)

1. Added a new `rtsp_utils.rs` module with URL validation and fixing functionality
2. Improved the `play_direct_rtsp` command to handle special characters and malformed URLs
3. Enhanced URL handling in ONVIF-related code to properly encode credentials

### Frontend (React)

1. Improved error handling for authentication failures in `DirectRTSPPlayer.tsx`
2. Added proper URL encoding for credentials when constructing RTSP URLs in `DirectCameras.tsx`
3. Added more descriptive error messages for authentication issues

## Testing Scenarios

1. **Username with special characters**: `user@name:password@192.168.1.100`
2. **Password with special characters**: `username:pass/word123@192.168.1.100` 
3. **Both with special characters**: `user@name:pass/word123@192.168.1.100`

## Debugging Tips

If you're still experiencing RTSP authentication issues:

1. Check the Rust backend logs for any errors related to URL processing
2. Verify that the username and password are correct for the camera
3. Try connecting directly via an RTSP player like VLC to validate credentials
4. For cameras that don't support standard RTSP URLs, use the ONVIF discovery feature

## Known Limitations

1. Some cameras have non-standard RTSP URL formats that may still require manual configuration
2. Very complex passwords with multiple special characters may need manual URL encoding