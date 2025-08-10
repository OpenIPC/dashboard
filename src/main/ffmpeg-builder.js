// --- ФАЙЛ: src/main/ffmpeg-builder.js (НОВЫЙ) ---

const path = require('path');
const ffmpeg = require('@ffmpeg-installer/ffmpeg');

// Путь к ffmpeg с учетом asar-упаковки
const ffmpegPath = ffmpeg.path.replace('app.asar', 'app.asar.unpacked');

class FfmpegCommandBuilder {
    constructor(appSettings) {
        this.settings = appSettings;
    }

    /**
     * Формирует аргументы для запуска стриминга в JSMpeg.
     * @param {object} credentials - Полные данные камеры, включая пароль.
     * @param {number} streamId - ID потока (0 для HD, 1 для SD).
     * @returns {{ command: string, args: string[] }}
     */
    buildForStream(credentials, streamId) {
        const streamPath = streamId === 0 ? (credentials.streamPath0 || '/stream0') : (credentials.streamPath1 || '/stream1');
        const streamUrl = this.buildRtspUrl(credentials, streamPath);
        
        // Используем getHwAccelOptions для определения декодера и фильтров
        const { decoderArgs, vfString } = this.getHwAccelOptions(credentials.codec || 'h264', streamId);

        const args = [
            ...decoderArgs,
            '-loglevel', 'error',
            '-rtsp_transport', 'tcp',
            '-err_detect', 'ignore_err',
            '-i', streamUrl,
            '-progress', 'pipe:2', // Для получения статистики
            '-f', 'mpegts',
            '-c:v', 'mpeg1video',
            '-preset', 'ultrafast',
            '-vf', vfString,
            '-q:v', String(this.settings.qscale || 8),
            '-r', String(this.settings.fps || 20),
            '-bf', '0',
            '-ignore_unknown', 
            '-c:a', 'mp2', 
            '-b:a', '128k', 
            '-ar', '44100', 
            '-ac', '1',
            '-' // Вывод в stdout
        ];

        return { command: ffmpegPath, args };
    }

    /**
     * Формирует аргументы для записи потока в файл.
     * @param {object} credentials - Полные данные камеры, включая пароль.
     * @param {string} outputPath - Путь к выходному файлу.
     * @returns {{ command: string, args: string[] }}
     */
    buildForRecording(credentials, outputPath) {
        const streamUrl = this.buildRtspUrl(credentials, credentials.streamPath0 || '/stream0');
        const args = [
            '-rtsp_transport', 'tcp',
            '-i', streamUrl,
            '-c:v', 'copy',      // Простое копирование видеопотока без перекодирования
            '-c:a', 'aac',       // Перекодирование аудио в AAC
            '-b:a', '128k',
            '-movflags', '+faststart', // Для возможности просмотра до полной загрузки
            outputPath
        ];
        return { command: ffmpegPath, args };
    }

    /**
     * Формирует аргументы для экспорта фрагмента из архива.
     * @param {string} sourcePath - Путь к исходному файлу.
     * @param {number} startTime - Время начала фрагмента в секундах.
     * @param {number} duration - Длительность фрагмента в секундах.
     * @param {string} outputPath - Путь к выходному файлу.
     * @returns {{ command: string, args: string[] }}
     */
    buildForExport(sourcePath, startTime, duration, outputPath) {
        const args = [
            '-i', sourcePath,
            '-ss', String(startTime),
            '-t', String(duration),
            '-c', 'copy', // Копируем без перекодирования
            outputPath
        ];
        return { command: ffmpegPath, args };
    }

    /**
     * Вспомогательная функция для сборки RTSP URL.
     * @private
     */
    buildRtspUrl(credentials, streamPath) {
        return `rtsp://${encodeURIComponent(credentials.username)}:${encodeURIComponent(credentials.password || '')}@${credentials.ip}:${credentials.port || 554}${streamPath}`;
    }

    /**
     * Вспомогательная функция для выбора опций аппаратного ускорения.
     * @private
     */
    getHwAccelOptions(codec, streamId) {
        const preference = this.settings.hwAccel || 'auto';
        const isSD = streamId === 1;

        if (preference === 'nvidia') {
            const decoder = codec === 'h264' ? 'h264_cuvid' : 'hevc_cuvid';
            const decoderArgs = ['-c:v', decoder];
            // Встроенный ресайз в декодере NVIDIA CUVID
            if (isSD) decoderArgs.push('-resize', '640x360'); 
            console.log(`[FFMPEG Builder] Using HW Accel: ${decoder} ${isSD ? 'with built-in resize' : 'for HD'}`);
            return { decoderArgs, vfString: 'format=yuv420p' };
        }

        if (preference === 'intel') {
            const decoder = codec === 'h264' ? 'h264_qsv' : 'hevc_qsv';
            let vfString = 'hwdownload,format=yuv420p';
            if (isSD) vfString = 'scale_qsv=w=640:h=-2,' + vfString;
            console.log(`[FFMPEG Builder] Using HW Accel: ${decoder} ${isSD ? 'with QSV scaler' : 'for HD'}`);
            return { decoderArgs: ['-c:v', decoder], vfString };
        }

        let decoderArgs = [];
        let platformMsg = '';
        if (preference === 'auto') {
            switch (process.platform) {
                case 'win32': decoderArgs = ['-hwaccel', 'd3d11va']; platformMsg = 'Auto-selecting d3d11va'; break;
                case 'darwin': decoderArgs = ['-hwaccel', 'videotoolbox']; platformMsg = 'Auto-selecting videotoolbox'; break;
                default: platformMsg = 'Auto-selection on Linux: Using CPU for stability.'; break;
            }
        } else {
            platformMsg = 'Hardware acceleration disabled.';
        }

        let vfString = 'format=yuv420p';
        if (isSD) vfString = 'scale=w=640:h=-2,' + vfString;
        console.log(`[FFMPEG Builder] ${platformMsg}. ${isSD ? 'Using CPU scaler for SD.' : ''}`);
        return { decoderArgs, vfString };
    }
}

module.exports = FfmpegCommandBuilder;