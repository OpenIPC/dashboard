# RTSP URL Test Cases

This document contains a list of test cases for validating RTSP URL handling with special characters in usernames and passwords.

## Test Cases

| Case | Description | URL | Expected Result |
|------|-------------|-----|----------------|
| 1 | Basic URL (no special chars) | `rtsp://admin:password@192.168.1.100:554/stream` | Works without modification |
| 2 | @ Symbol in password | `rtsp://admin:pass@word@192.168.1.100:554/stream` | Fixed and works correctly |
| 3 | Space in password | `rtsp://admin:my password@192.168.1.100:554/stream` | Spaces encoded and works |
| 4 | Multiple @ symbols | `rtsp://admin:p@ss@w@rd@192.168.1.100:554/stream` | All @ in password encoded, works |
| 5 | Forward slash in password | `rtsp://admin:pass/word@192.168.1.100:554/stream` | / encoded and works |
| 6 | Special chars in username | `rtsp://admin@home:password@192.168.1.100:554/stream` | @ in username encoded, works |
| 7 | Multiple special chars | `rtsp://admin:p@ss$#%^&@192.168.1.100:554/stream` | All special chars encoded, works |
| 8 | Pre-encoded characters | `rtsp://admin:pass%40word@192.168.1.100:554/stream` | No double encoding, works |

## Testing Instructions

1. Open the DirectRTSPPlayer component with each test URL
2. Check browser console for any errors
3. Verify that the stream plays correctly
4. Check backend logs for URL processing details
5. Confirm no 401 authentication errors occur

## Troubleshooting

If a test fails:

1. Check the backend logs to see how the URL was processed
2. Look for "Fixed RTSP URL" entries to see the final URL that was sent
3. Verify that special characters are properly encoded in the URL
4. Check for any double-encoding of percent symbols (% becoming %25)
5. Ensure the authentication information is correctly separated from the host

## Expected Backend Log Format

```
Processing RTSP URL for direct playback
URL contains special characters, decoding first to avoid double-encoding
Successfully decoded URL
Fixing RTSP URL: Input URL starts with rtsp://
Password contains special characters, encoding them
Fixed RTSP URL: rtsp://***:***@192.168.1.100:554/stream
Decoded URL for FFmpeg (credentials masked)
Returning URL for processing (credentials masked)
```