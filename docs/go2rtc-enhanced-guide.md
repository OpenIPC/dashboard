# go2rtc Enhanced Integration Guide

## Overview

This project now includes comprehensive integration with go2rtc, unlocking powerful streaming capabilities including multiple transport protocols, two-way audio, real-time monitoring, adaptive bitrate, and more.

## Features

### 1. Multiple Transport Protocols

go2rtc supports various streaming transports with automatic fallback:

- **WebRTC** (Primary) - Ultra-low latency, best for real-time viewing
- **HLS** - Broad compatibility, good for mobile devices
- **MSE** - Low latency without WebRTC complexity (future)
- **MJPEG** - Universal fallback, works everywhere
- **RTSP** - Direct RTSP streaming
- **RTMP** - For external broadcasting

### 2. Two-Way Audio

Enable bidirectional audio communication for intercoms and interactive cameras:

```typescript
import TwoWayAudioControl from './components/TwoWayAudioControl';

<TwoWayAudioControl
  streamName="cam1_0"
  enabled={true}
  pushToTalk={false}  // or true for PTT mode
  onAudioStateChange={(active) => console.log('Audio:', active)}
/>
```

Features:
- Push-to-talk or continuous mode
- Echo cancellation
- Noise suppression
- Auto gain control
- Volume control

### 3. Real-Time Stream Monitoring

Monitor stream health and statistics in real-time:

```typescript
import StreamMonitor from './components/StreamMonitor';

<StreamMonitor
  streamName="cam1_0"
  updateInterval={2000}
  compact={false}
  showDetails={true}
/>
```

Displays:
- Live status
- Bitrate (Mbps)
- Video codec and resolution
- Audio codec
- Number of viewers
- Latency
- Signal strength

### 4. Enhanced Video Player Hook

Use the advanced video streaming hook with auto-transport selection:

```typescript
import { useEnhancedVideoStream } from './hooks/useEnhancedVideoStream';

const MyVideoPlayer = ({ streamName }) => {
  const {
    videoRef,
    currentTransport,
    streamInfo,
    isLoading,
    error,
    switchTransport,
    takeSnapshot,
    reconnect,
  } = useEnhancedVideoStream({
    streamName,
    preferredTransport: 'webrtc',
    enableAdaptiveBitrate: true,
    enableMonitoring: true,
    onStatsUpdate: (stats) => console.log('Stats:', stats),
  });

  return (
    <div>
      <video ref={videoRef} autoPlay playsInline />
      <button onClick={() => switchTransport('hls')}>Switch to HLS</button>
      <button onClick={takeSnapshot}>Take Snapshot</button>
    </div>
  );
};
```

### 5. Quick Snapshots

Capture high-quality snapshots without loading full stream:

```typescript
import SnapshotButton from './components/SnapshotButton';

<SnapshotButton
  streamName="cam1_0"
  width={1920}
  height={1080}
  quality={95}
  autoDownload={true}
  filename="camera-snapshot.jpg"
/>
```

### 6. Adaptive Bitrate Streaming

Automatically adjust quality based on network conditions:

```typescript
const { videoRef } = useEnhancedVideoStream({
  streamName: 'cam1_0',
  enableAdaptiveBitrate: true,  // Enable adaptive switching
  // Switches to HLS if bitrate < 500 kbps for 15 seconds
  // Switches back to WebRTC if bitrate > 1500 kbps
});
```

### 7. go2rtc Service API

Direct access to go2rtc functionality:

```typescript
import { getGo2RtcService } from './services/go2rtc';

const service = getGo2RtcService();

// Get stream statistics
const stats = await service.getStreamStats('cam1_0');
console.log('Consumers:', stats.consumers);

// Get snapshot
const blob = await service.getSnapshot('cam1_0', {
  width: 1920,
  height: 1080,
  quality: 90,
});

// Monitor stream
const cleanup = service.startMonitoring('cam1_0', (info) => {
  console.log('Bitrate:', info.bitrateKbps);
  console.log('Online:', info.online);
  console.log('Viewers:', info.consumerCount);
}, 2000);

// Check stream status
const isOnline = await service.isStreamOnline('cam1_0');

// Get all streams
const streams = await service.getAllStreams();

// Cleanup
cleanup();
service.dispose();
```

### 8. Backend Commands (Rust/Tauri)

New Tauri commands for go2rtc integration:

```typescript
import { invoke } from '@tauri-apps/api/core';

// Get stream statistics
const stats = await invoke('get_go2rtc_stream_stats', {
  streamName: 'cam1_0'
});

// Get snapshot
const imageData = await invoke('get_go2rtc_snapshot', {
  streamName: 'cam1_0',
  width: 1920,
  height: 1080,
  quality: 90,
});

// Get all streams
const streams = await invoke('get_go2rtc_all_streams');

// Check if stream is online
const isOnline = await invoke('check_go2rtc_stream_online', {
  streamName: 'cam1_0'
});

// Get server info
const serverInfo = await invoke('get_go2rtc_server_info');
```

## Advanced Features

### Stream Filters

Apply on-the-fly video transformations:

```typescript
const service = getGo2RtcService();

const url = service.buildStreamUrl('cam1_0', 'webrtc', {
  rotate: 90,  // Rotate 0, 90, 180, or 270 degrees
  width: 1280,
  height: 720,
  crop: { x: 100, y: 100, width: 800, height: 600 },
  overlay: {
    text: 'Front Door Camera',
    position: 'top-left',
    fontSize: 24,
    color: 'white',
  },
});
```

### Transport Preferences

Customize transport selection:

```typescript
const transport = await service.getOptimalTransport('cam1_0', {
  primary: 'webrtc',
  fallbacks: ['mse', 'hls', 'mjpeg'],
  autoSwitch: true,
});
```

### WebRTC with Audio Config

Create WebRTC connection with custom audio settings:

```typescript
const pc = await service.createWebRTCConnection('cam1_0', {
  enabled: true,
  codec: 'opus',
  sampleRate: 48000,
  channels: 1,
  echoCancellation: true,
  noiseSuppression: true,
  autoGainControl: true,
});
```

## Configuration

### go2rtc Configuration File

The application automatically manages go2rtc configuration at:
- Windows: `%LOCALAPPDATA%/vms-dashboard/go2rtc/go2rtc.yaml`
- Linux: `~/.local/share/vms-dashboard/go2rtc/go2rtc.yaml`
- macOS: `~/Library/Application Support/vms-dashboard/go2rtc/go2rtc.yaml`

Example configuration:

```yaml
streams:
  cam1_0: rtsp://admin:password@192.168.1.100:554/stream1
  cam1_1: rtsp://admin:password@192.168.1.100:554/stream2

api:
  listen: :1984

webrtc:
  listen: :8555

hls:
  listen: :8556

rtsp:
  listen: :8554
```

### Application Settings

Configure go2rtc in application settings:

```json
{
  "streaming": {
    "provider": "go2rtc",
    "enableOnDemand": true,
    "restartOnConfigChange": true,
    "go2rtc": {
      "apiAddresses": [
        "http://127.0.0.1:1984"
      ]
    }
  }
}
```

## Best Practices

### 1. Transport Selection

- **Use WebRTC** for live viewing (lowest latency ~100-300ms)
- **Use HLS** for mobile devices or when WebRTC fails
- **Use MJPEG** as universal fallback

### 2. Bitrate Management

- Enable adaptive bitrate for unstable networks
- Monitor stream health for debugging
- Use lower quality streams for bandwidth-limited scenarios

### 3. Snapshot Performance

- Use go2rtc snapshots instead of canvas captures
- Specify resolution to reduce bandwidth
- Cache snapshots for preview grids

### 4. Two-Way Audio

- Test echo cancellation settings
- Use push-to-talk for noisy environments
- Monitor audio levels in UI

### 5. Resource Management

- Call cleanup/dispose when unmounting components
- Stop monitoring when not needed
- Close peer connections properly

## Troubleshooting

### WebRTC Connection Fails

1. Check ICE candidate gathering
2. Verify firewall settings
3. Enable fallback to HLS
4. Check browser WebRTC support

### High Latency

1. Switch to WebRTC if using HLS
2. Check network bandwidth
3. Monitor stream statistics
4. Reduce video resolution

### Audio Issues

1. Check browser microphone permissions
2. Verify audio codec compatibility
3. Test echo cancellation settings
4. Check sample rate support

### Stream Not Loading

1. Verify go2rtc is running: `invoke('start_go2rtc')`
2. Check stream configuration
3. Test RTSP source directly
4. Review go2rtc logs

## Performance Tips

1. **Use WebRTC for <10 simultaneous viewers** - Best quality and latency
2. **Use HLS for >10 viewers** - Better scalability
3. **Enable monitoring only when needed** - Reduces overhead
4. **Use compact UI components** - Faster rendering
5. **Implement lazy loading** - Load streams on demand

## Examples

See the following components for complete examples:

- `src/components/VideoStreamPlayer.tsx` - Full-featured player
- `src/components/StreamMonitor.tsx` - Monitoring widget
- `src/components/TwoWayAudioControl.tsx` - Audio interaction
- `src/components/SnapshotButton.tsx` - Quick snapshots
- `src/hooks/useEnhancedVideoStream.ts` - Advanced streaming hook
- `src/services/go2rtc.ts` - Service API

## API Reference

### Go2RtcService Methods

- `getStreamStats(streamName)` - Get real-time statistics
- `getStreamInfo(streamName)` - Get processed stream information
- `getSnapshot(streamName, options)` - Capture snapshot
- `getSnapshotDataUrl(streamName, options)` - Get base64 snapshot
- `startMonitoring(streamName, callback, interval)` - Start monitoring
- `stopMonitoring(streamName)` - Stop monitoring
- `getOptimalTransport(streamName, preference)` - Get best transport
- `buildStreamUrl(streamName, transport, filters)` - Build stream URL
- `createWebRTCConnection(streamName, audioConfig)` - Create WebRTC PC
- `connectWebRTC(streamName, videoElement, audioConfig)` - Full WebRTC setup
- `getAllStreams()` - List all streams
- `isStreamOnline(streamName)` - Check online status
- `dispose()` - Cleanup resources

### Tauri Commands

- `start_go2rtc()` - Start go2rtc server
- `stop_go2rtc()` - Stop go2rtc server
- `get_go2rtc_stream_stats(streamName)` - Get statistics
- `get_go2rtc_snapshot(streamName, width, height, quality)` - Get snapshot
- `get_go2rtc_all_streams()` - List streams
- `check_go2rtc_stream_online(streamName)` - Check status
- `get_go2rtc_server_info()` - Get server info

## License

Same as main project license.
