// --- START OF FILE src/main/ffmpeg-builder.js ---
const path = require('path');

let ffmpegPath;
try {
    const ffmpeg = require('@ffmpeg-installer/ffmpeg');
    ffmpegPath = ffmpeg.path.replace('app.asar', 'app.asar.unpacked');
} catch (e) {
    ffmpegPath = 'ffmpeg'; // Use system ffmpeg
}

class FfmpegCommandBuilder {
    constructor(appSettings) {
        this.settings = appSettings;
    }

    buildForStream(credentials, streamId, statsPort) { // Добавлен statsPort
        const streamPath = streamId === 0 ? (credentials.streamPath0 || '/stream=0') : (credentials.streamPath1 || '/stream=1');
        const streamUrl = this.buildRtspUrl(credentials, streamPath);

        const args = [
            '-rtsp_transport', 'tcp',
            '-i', streamUrl,
            '-progress', `tcp://127.0.0.1:${statsPort}`,
            '-f', 'mpegts',
            '-c:v', 'copy',
            '-c:a', 'copy',
            '-'
        ];

        return { command: ffmpegPath, args };
    }

    buildForRecording(credentials, outputPath) {
    const streamUrl = this.buildRtspUrl(credentials, credentials.streamPath0 || '/stream=0');
        
        const args = [
            '-rtsp_transport', 'tcp',
            '-fflags', 'discardcorrupt',
            '-use_wallclock_as_timestamps', '1',
            '-i', streamUrl,
            '-c:v', 'copy',
            '-c:a', 'aac',
            '-b:a', '128k',
            '-movflags', '+faststart',
            outputPath
        ];
        return { command: ffmpegPath, args };
    }

    buildForExport(sourcePath, startTime, duration, outputPath) {
        const args = [
            '-i', sourcePath,
            '-ss', String(startTime),
            '-t', String(duration),
            '-c', 'copy',
            outputPath
        ];
        return { command: ffmpegPath, args };
    }
    
    buildForHls(sourcePath, outputPath, startTime = 0, sourceCodec = null) {
        const playlistPath = path.join(outputPath, 'playlist.m3u8');
        
        let videoCodecArgs = [];
        if (sourceCodec === 'hevc' || sourceCodec === 'h265') {
            videoCodecArgs = ['-c:v', 'libx264', '-preset', 'ultrafast', '-tune', 'zerolatency'];
        } else {
            videoCodecArgs = ['-c:v', 'copy'];
        }

        const args = [
            '-ss', String(startTime),
            '-i', sourcePath,
            ...videoCodecArgs,
            '-c:a', 'copy',
            '-f', 'hls',
            '-hls_time', '4',
            '-hls_playlist_type', 'vod',
            '-hls_segment_filename', path.join(outputPath, 'segment%03d.ts'),
            playlistPath
        ];
        return { command: ffmpegPath, args };
    }

    buildRtspUrl(credentials, streamPath) {
        return `rtsp://${encodeURIComponent(credentials.username)}:${encodeURIComponent(credentials.password || '')}@${credentials.ip}:${credentials.port || 554}${streamPath}`;
    }

    getHwAccelOptions(streamId) {
    const preference = this.settings.hwAccel || 'auto';
    const isSD = streamId == 1;
        let decoderArgs = [];
        let vfArgs = ['-vf'];
        let vfString = 'format=yuv420p';

        if (preference === 'nvidia') {
            decoderArgs = ['-c:v', 'h264_cuvid'];
            if (isSD) decoderArgs.push('-resize', '640x360');
        } else if (preference === 'intel') {
            decoderArgs = ['-c:v', 'h264_qsv'];
            if (isSD) vfString = `scale_qsv=w=640:h=-2,${vfString}`;
        } else {
            if (isSD) vfString = `scale=w=640:h=-2,${vfString}`;
        }

        vfArgs.push(vfString);
        return { decoderArgs, vfArgs };
    }
}

module.exports = FfmpegCommandBuilder;