// --- START OF FILE src/main/ffmpeg-builder.js ---
// --- ФАЙЛ: src/main/ffmpeg-builder.js (ФИНАЛЬНАЯ ОПТИМИЗИРОВАННАЯ ВЕРСИЯ) ---

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
        
        const { decoderArgs, vfArgs } = this.getHwAccelOptions(streamId);

        const args = [
            ...decoderArgs,
            '-loglevel', 'error',
            '-rtsp_transport', 'tcp',
            '-i', streamUrl,
            '-progress', 'pipe:2',
            '-f', 'mpegts',
            '-c:v', 'mpeg1video',
            ...vfArgs,
            '-q:v', String(this.settings.qscale || 8),
            '-r', String(this.settings.fps || 20),
            '-bf', '0',
            '-c:a', 'mp2', 
            '-b:a', '128k', 
            '-ar', '44100', 
            '-ac', '1',
            '-'
        ];

        return { command: ffmpegPath, args: args.filter(Boolean) }; // Убираем пустые элементы
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
            '-c:v', 'copy',
            '-c:a', 'aac',
            '-b:a', '128k',
            '-movflags', '+faststart',
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
            '-c', 'copy',
            outputPath
        ];
        return { command: ffmpegPath, args };
    }

    /**
     * Формирует аргументы для нарезки архивного файла в HLS.
     * @param {string} sourcePath - Путь к исходному файлу .mp4.
     * @param {string} outputPath - Путь к папке, куда будут складываться .m3u8 и .ts файлы.
     * @param {number} [startTime=0] - Время в секундах, с которого начать нарезку.
     * @param {string|null} [sourceCodec=null] - Имя кодека исходного файла ('h24' или 'hevc').
     * @returns {{ command: string, args: string[] }}
     */
    buildForHls(sourcePath, outputPath, startTime = 0, sourceCodec = null) {
        const playlistPath = path.join(outputPath, 'playlist.m3u8');
        
        let videoCodecArgs = [];
        if (sourceCodec === 'hevc' || sourceCodec === 'h265') {
            console.log(`[HLS Builder] Source is H.265. Transcoding to H.264...`);
            videoCodecArgs = ['-c:v', 'libx264', '-preset', 'ultrafast', '-tune', 'zerolatency'];
        } else {
            console.log(`[HLS Builder] Source is H.264 or unknown. Using stream copy.`);
            videoCodecArgs = ['-c:v', 'copy'];
        }

        const args = [
            // -ss ДО -i для быстрого поиска по ключевым кадрам
            '-ss', String(startTime),
            '-i', sourcePath,

            ...videoCodecArgs,
            '-c:a', 'copy',

            // Этот набор флагов говорит FFmpeg нарезать ВЕСЬ оставшийся файл
            // максимально быстро и создать конечный плейлист (VOD - Video on Demand).
            // Процесс FFmpeg завершится после выполнения задачи, что является правильным поведением.
            '-f', 'hls',
            '-hls_time', '4',
            '-hls_playlist_type', 'vod',
            '-hls_segment_filename', path.join(outputPath, 'segment%03d.ts'),
            playlistPath
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
    getHwAccelOptions(streamId) {
        const preference = this.settings.hwAccel || 'auto';
        const isSD = streamId === 1;

        let decoderArgs = [];
        let vfArgs = ['-vf'];
        let vfString = 'format=yuv420p'; // Базовый формат, необходимый для mpeg1video

        if (preference === 'nvidia') {
            console.log(`[FFMPEG Builder] Using HW Accel: NVIDIA (CUVID)`);
            decoderArgs = ['-c:v', 'h264_cuvid']; // Используем декодер CUVID
            if (isSD) decoderArgs.push('-resize', '640x360'); // Встроенный ресайз
        } else if (preference === 'intel') {
            console.log(`[FFMPEG Builder] Using HW Accel: Intel (QSV)`);
            decoderArgs = ['-c:v', 'h264_qsv']; // Используем декодер QSV
            if (isSD) vfString = `scale_qsv=w=640:h=-2,${vfString}`; // Ресайз через фильтр QSV
        } else {
            // Для 'auto' или 'none' используем CPU. Это самый надежный вариант.
            console.log(`[FFMPEG Builder] Using CPU decoding.`);
            if (isSD) vfString = `scale=w=640:h=-2,${vfString}`; // Программный ресайз
        }

        vfArgs.push(vfString);
        return { decoderArgs, vfArgs };
    }
}

module.exports = FfmpegCommandBuilder;
// --- END OF FILE src/main/ffmpeg-builder.js ---