import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import OpenIPC

Dialog {
    id: dialog

    property string cameraName: ""
    property string cameraHost: ""
    property int cameraPort: 80
    property string cameraUser: "root"
    property string cameraPassword: ""

    property var originalConfig: ({})
    property var currentSchema: ({})
    property var fields: []
    property var capabilities: ({})
    property var draftValues: ({})
    property var groups: []
    property var pendingPatch: ({})
    property var pendingChanges: []
    property var requestIds: ({})
    property var rollbackConfig: ({})
    property var rollbackSchema: ({})
    property var rollbackChanges: []
    property bool rollbackAvailable: false
    property bool rollbackCritical: false
    property string rollbackReason: ""
    property string rollbackHealthState: "idle"
    property string rollbackHealthText: ""
    property bool rollbackWatchActive: false
    property int rollbackWatchTries: 0
    property int rollbackWatchMaxTries: 4
    property bool rollbackAutoAllowed: false
    property string activeRollbackId: ""
    property var backupRestoreConfig: ({})
    property var backupRestoreSchema: ({})
    property var backupRestoreChanges: []
    property string backupRestorePath: ""
    property string selectedGroupId: ""
    property bool pendingPipelineReloadNeeded: false
    property bool activeResetNeedsPipelineReload: false
    property int revision: 0
    property int dirtyCount: 0
    property bool loading: false
    property bool statusError: false
    property bool pipelineReloadNeeded: false
    property string statusText: I18n.t("Готово к подключению")
    property string activeLoadId: ""
    property string activeApplyId: ""
    property string activeResetId: ""
    property string activeMetricsId: ""
    property string activeReloadId: ""
    property bool autoReloadAfterApply: true
    property bool pendingAutoReloadAfterApply: false
    property string rtspMainProbeId: ""
    property string rtspSubProbeId: ""
    property string majesticApiProbeId: ""
    property string rtspMainProbeState: "idle"
    property string rtspSubProbeState: "idle"
    property string majesticApiProbeState: "idle"
    property string rtspMainProbeMessage: ""
    property string rtspSubProbeMessage: ""
    property string majesticApiProbeMessage: ""
    property int rtspMainProbeElapsedMs: 0
    property int rtspSubProbeElapsedMs: 0
    property int majesticApiProbeElapsedMs: 0
    property string metricsText: ""
    property string metricsFilterText: ""
    property string metricsUpdatedAt: ""
    property var metricsData: ({})
    property bool firmwareBusy: false
    property var firmwareStatus: ({})
    property var firmwareNetwork: ({})
    property var firmwareTime: ({})
    property var firmwareUpdateInfo: ({})
    property var firmwareWifiNetworks: []
    property string firmwareLogsText: ""
    property string firmwareLogsSource: "all"
    property bool firmwareLiveLogs: false
    property bool firmwareLogsPaused: false
    property string firmwareLogFilter: ""
    property int firmwareLogLineLimit: 2000
    property string firmwareUpgradeText: ""
    property bool firmwareUpgradeRebooting: false
    property bool firmwareArchiveUploaded: false
    property string firmwareArchivePath: ""
    property string firmwareArchiveName: ""
    property real firmwareArchiveSizeBytes: 0
    property var firmwareArchiveInspection: ({})
    property bool firmwarePowerSafetyConfirmed: false
    property bool firmwareDangerOptionsConfirmed: false
    property bool firmwarePostReturnProbeActive: false
    property bool firmwareUpdateKernel: true
    property bool firmwareUpdateRootfs: true
    property bool firmwareUpdateReset: false
    property bool firmwareUpdateForce: false
    property bool firmwareBackupSaved: false
    property bool firmwareReturnPolling: false
    property int firmwareReturnPollTries: 0
    property int firmwareReturnPollMaxTries: 60
    property string firmwareReturnPhase: "idle"
    property string firmwareReturnHealthText: ""
    readonly property bool firmwareWebSocketsAvailable: SystemController.firmwareClient.webSocketsAvailable
    property string activeFirmwareStatusId: ""
    property string activeFirmwareNetworkId: ""
    property string activeFirmwareNetworkSaveId: ""
    property string activeFirmwareTimeId: ""
    property string activeFirmwareTimeSaveId: ""
    property string activeFirmwareLogsId: ""
    property string activeFirmwareLiveLogsId: ""
    property string activeFirmwareBackupId: ""
    property string activeFirmwareRebootId: ""
    property string activeFirmwareUpdateId: ""
    property string activeFirmwareReturnProbeId: ""

    readonly property var majesticGroupText: ({
        image: { ru: "Изображение", en: "Image" },
        video: { ru: "Видео и аудио", en: "Video & Audio" },
        "video-audio": { ru: "Видео и аудио", en: "Video & Audio" },
        events: { ru: "События", en: "Events" },
        recording: { ru: "Запись", en: "Recording" },
        records: { ru: "Запись", en: "Recording" },
        network: { ru: "Сеть и интеграции", en: "Network & Integrations" },
        "network-integrations": { ru: "Сеть и интеграции", en: "Network & Integrations" },
        system: { ru: "Система", en: "System" }
    })
    readonly property var majesticSectionText: ({
        image: { ru: "Изображение", en: "Image" },
        isp: { ru: "ISP и сенсор", en: "ISP & Sensor" },
        video0: { ru: "Основной поток", en: "Main stream" },
        video1: { ru: "Дополнительный поток", en: "Sub stream" },
        jpeg: { ru: "JPEG-снимки", en: "JPEG snapshots" },
        audio: { ru: "Аудио", en: "Audio" },
        rtsp: { ru: "RTSP-сервер", en: "RTSP server" },
        hls: { ru: "HLS", en: "HLS" },
        mjpeg: { ru: "MJPEG", en: "MJPEG" },
        webrtc: { ru: "WebRTC", en: "WebRTC" },
        outgoing: { ru: "Исходящие трансляции", en: "Outgoing streams" },
        records: { ru: "Запись на носитель", en: "Storage recording" },
        motionDetect: { ru: "Детекция движения", en: "Motion detection" },
        nightMode: { ru: "День/ночь", en: "Day / Night" },
        osd: { ru: "Экранные надписи", en: "On-screen display" },
        system: { ru: "Система Majestic", en: "Majestic system" },
        netip: { ru: "Сеть", en: "Network" },
        onvif: { ru: "ONVIF", en: "ONVIF" },
        ipeye: { ru: "IPEYE", en: "IPEYE" },
        mqtt: { ru: "MQTT", en: "MQTT" },
        srt: { ru: "SRT", en: "SRT" }
    })
    readonly property var majesticKeyText: ({
        enabled: { ru: "Включено", en: "Enabled" },
        codec: { ru: "Кодек", en: "Codec" },
        size: { ru: "Разрешение", en: "Resolution" },
        fps: { ru: "Частота кадров", en: "Frame rate" },
        bitrate: { ru: "Битрейт", en: "Bitrate" },
        rcMode: { ru: "Режим битрейта", en: "Rate-control mode" },
        gopSize: { ru: "Размер GOP", en: "GOP size" },
        profile: { ru: "Профиль кодека", en: "Codec profile" },
        rotate: { ru: "Поворот", en: "Rotation" },
        mirror: { ru: "Отразить по горизонтали", en: "Mirror horizontally" },
        flip: { ru: "Отразить по вертикали", en: "Flip vertically" },
        luminance: { ru: "Яркость", en: "Brightness" },
        contrast: { ru: "Контраст", en: "Contrast" },
        saturation: { ru: "Насыщенность", en: "Saturation" },
        hue: { ru: "Оттенок", en: "Hue" },
        tuning: { ru: "Тюнинг изображения", en: "Image tuning" },
        qfactor: { ru: "Качество JPEG", en: "JPEG quality" },
        quality: { ru: "Качество", en: "Quality" },
        path: { ru: "Путь", en: "Path" },
        split: { ru: "Длительность файла", en: "File split duration" },
        maxUsage: { ru: "Максимум места", en: "Maximum storage usage" },
        port: { ru: "Порт", en: "Port" },
        username: { ru: "Пользователь", en: "Username" },
        user: { ru: "Пользователь", en: "User" },
        password: { ru: "Пароль", en: "Password" },
        server: { ru: "Сервер", en: "Server" },
        host: { ru: "Хост", en: "Host" },
        endpoint: { ru: "Адрес назначения", en: "Destination endpoint" },
        stream: { ru: "Поток", en: "Stream" },
        token: { ru: "Токен", en: "Token" },
        srate: { ru: "Частота дискретизации", en: "Sample rate" },
        volume: { ru: "Громкость микрофона", en: "Microphone volume" },
        outputEnabled: { ru: "Вывод на динамик", en: "Speaker output" },
        outputVolume: { ru: "Громкость динамика", en: "Speaker volume" },
        speakerPin: { ru: "GPIO динамика", en: "Speaker GPIO" },
        speakerPinInvert: { ru: "Инвертировать GPIO динамика", en: "Invert speaker GPIO" },
        dual: { ru: "Двустороннее аудио", en: "Two-way audio" },
        jitterBufferMs: { ru: "Jitter buffer", en: "Jitter buffer" },
        backlightPin: { ru: "GPIO ИК-подсветки", en: "IR light GPIO" },
        irCutPin1: { ru: "GPIO IR-cut 1", en: "IR-cut GPIO 1" },
        irCutPin2: { ru: "GPIO IR-cut 2", en: "IR-cut GPIO 2" },
        lightMonitor: { ru: "Автоматический день/ночь", en: "Automatic day/night" },
        threshold: { ru: "Порог", en: "Threshold" },
        sensitivity: { ru: "Чувствительность", en: "Sensitivity" },
        interval: { ru: "Интервал проверки", en: "Check interval" },
        timeout: { ru: "Таймаут", en: "Timeout" },
        roi: { ru: "Зоны интереса", en: "Regions of interest" },
        visualize: { ru: "Показывать зоны", en: "Visualize regions" },
        debug: { ru: "Отладка", en: "Debug" },
        detector: { ru: "Детектор", en: "Detector" },
        minArea: { ru: "Минимальная площадь", en: "Minimum area" },
        maxArea: { ru: "Максимальная площадь", en: "Maximum area" },
        template: { ru: "Шаблон текста", en: "Text template" },
        posX: { ru: "Позиция X", en: "X position" },
        posY: { ru: "Позиция Y", en: "Y position" },
        font: { ru: "Шрифт", en: "Font" },
        fontSize: { ru: "Размер шрифта", en: "Font size" },
        color: { ru: "Цвет", en: "Color" },
        alpha: { ru: "Прозрачность", en: "Opacity" },
        mode: { ru: "Режим", en: "Mode" },
        dhcp: { ru: "DHCP", en: "DHCP" },
        gateway: { ru: "Шлюз", en: "Gateway" },
        dns: { ru: "DNS", en: "DNS" },
        mac: { ru: "MAC-адрес", en: "MAC address" },
        sensorConfig: { ru: "Профиль сенсора", en: "Sensor profile" },
        antiflicker: { ru: "Антифликер", en: "Anti-flicker" },
        exposure: { ru: "Экспозиция", en: "Exposure" },
        whiteBalance: { ru: "Баланс белого", en: "White balance" },
        blkCnt: { ru: "Black level count", en: "Black level count" },
        dis: { ru: "Цифровая стабилизация", en: "Digital stabilization" },
        webAdmin: { ru: "Web-интерфейс", en: "Web admin" },
        httpPort: { ru: "HTTP-порт", en: "HTTP port" },
        logLevel: { ru: "Уровень логов", en: "Log level" },
        buffer: { ru: "Буфер", en: "Buffer" },
        bind: { ru: "Адрес привязки", en: "Bind address" },
        address: { ru: "Адрес", en: "Address" },
        ip: { ru: "IP-адрес", en: "IP address" },
        mask: { ru: "Маска сети", en: "Network mask" },
        netmask: { ru: "Маска сети", en: "Network mask" },
        interface: { ru: "Интерфейс", en: "Interface" },
        iface: { ru: "Интерфейс", en: "Interface" },
        device: { ru: "Устройство", en: "Device" },
        mtu: { ru: "MTU", en: "MTU" },
        protocol: { ru: "Протокол", en: "Protocol" },
        proto: { ru: "Протокол", en: "Protocol" },
        url: { ru: "URL", en: "URL" },
        uri: { ru: "URI", en: "URI" },
        format: { ru: "Формат", en: "Format" },
        method: { ru: "Метод", en: "Method" },
        duration: { ru: "Длительность", en: "Duration" },
        delay: { ru: "Задержка", en: "Delay" },
        prebuffer: { ru: "Предбуфер", en: "Pre-buffer" },
        postbuffer: { ru: "Постбуфер", en: "Post-buffer" },
        channel: { ru: "Канал", en: "Channel" },
        crop: { ru: "Кадрирование", en: "Crop" },
        source: { ru: "Источник", en: "Source" },
        output: { ru: "Выход", en: "Output" },
        payload: { ru: "Payload", en: "Payload" },
        payloadType: { ru: "Тип payload", en: "Payload type" },
        bframes: { ru: "B-кадры", en: "B-frames" },
        avcProfile: { ru: "Профиль AVC", en: "AVC profile" }
    })
    readonly property var majesticFieldText: ({
        "image.luminance": { ru: "Яркость изображения", en: "Image brightness" },
        "image.contrast": { ru: "Контраст изображения", en: "Image contrast" },
        "image.saturation": { ru: "Насыщенность изображения", en: "Image saturation" },
        "image.hue": { ru: "Оттенок изображения", en: "Image hue" },
        "image.mirror": { ru: "Отразить изображение по горизонтали", en: "Mirror image horizontally" },
        "image.flip": { ru: "Отразить изображение по вертикали", en: "Flip image vertically" },
        "image.rotate": { ru: "Повернуть изображение", en: "Rotate image" },
        "image.tuning": { ru: "Расширенный тюнинг изображения", en: "Advanced image tuning" },
        "video0.enabled": { ru: "Включить основной поток", en: "Enable main stream" },
        "video0.codec": { ru: "Кодек основного потока", en: "Main stream codec" },
        "video0.size": { ru: "Разрешение основного потока", en: "Main stream resolution" },
        "video0.fps": { ru: "FPS основного потока", en: "Main stream FPS" },
        "video0.bitrate": { ru: "Битрейт основного потока", en: "Main stream bitrate" },
        "video0.rcMode": { ru: "Режим битрейта основного потока", en: "Main stream rate control" },
        "video0.gopSize": { ru: "GOP основного потока", en: "Main stream GOP" },
        "video0.profile": { ru: "Профиль кодека основного потока", en: "Main stream codec profile" },
        "video1.enabled": { ru: "Включить дополнительный поток", en: "Enable sub stream" },
        "video1.codec": { ru: "Кодек дополнительного потока", en: "Sub stream codec" },
        "video1.size": { ru: "Разрешение дополнительного потока", en: "Sub stream resolution" },
        "video1.fps": { ru: "FPS дополнительного потока", en: "Sub stream FPS" },
        "video1.bitrate": { ru: "Битрейт дополнительного потока", en: "Sub stream bitrate" },
        "video1.rcMode": { ru: "Режим битрейта дополнительного потока", en: "Sub stream rate control" },
        "video1.gopSize": { ru: "GOP дополнительного потока", en: "Sub stream GOP" },
        "video1.profile": { ru: "Профиль кодека дополнительного потока", en: "Sub stream codec profile" },
        "video1.crop": { ru: "Кадрирование дополнительного потока", en: "Sub stream crop" },
        "jpeg.enabled": { ru: "Включить JPEG-снимки", en: "Enable JPEG snapshots" },
        "jpeg.size": { ru: "Разрешение JPEG-снимка", en: "JPEG snapshot resolution" },
        "jpeg.fps": { ru: "Частота JPEG-снимков", en: "JPEG snapshot rate" },
        "jpeg.qfactor": { ru: "Качество JPEG-снимка", en: "JPEG snapshot quality" },
        "audio.enabled": { ru: "Включить микрофон", en: "Enable microphone" },
        "audio.codec": { ru: "Аудиокодек", en: "Audio codec" },
        "audio.srate": { ru: "Частота аудио", en: "Audio sample rate" },
        "audio.volume": { ru: "Громкость микрофона", en: "Microphone volume" },
        "audio.outputEnabled": { ru: "Включить динамик", en: "Enable speaker" },
        "audio.outputVolume": { ru: "Громкость динамика", en: "Speaker volume" },
        "audio.dual": { ru: "Двустороннее аудио", en: "Two-way audio" },
        "audio.jitterBufferMs": { ru: "Аудио jitter buffer", en: "Audio jitter buffer" },
        "rtsp.enabled": { ru: "Включить RTSP", en: "Enable RTSP" },
        "rtsp.port": { ru: "RTSP-порт", en: "RTSP port" },
        "rtsp.username": { ru: "Пользователь RTSP", en: "RTSP username" },
        "rtsp.password": { ru: "Пароль RTSP", en: "RTSP password" },
        "hls.enabled": { ru: "Включить HLS", en: "Enable HLS" },
        "mjpeg.enabled": { ru: "Включить MJPEG", en: "Enable MJPEG" },
        "webrtc.enabled": { ru: "Включить WebRTC", en: "Enable WebRTC" },
        "records.enabled": { ru: "Включить запись", en: "Enable recording" },
        "records.path": { ru: "Путь записи", en: "Recording path" },
        "records.split": { ru: "Разбивать запись каждые", en: "Split recording every" },
        "records.maxUsage": { ru: "Лимит заполнения носителя", en: "Storage usage limit" },
        "motionDetect.enabled": { ru: "Включить детекцию движения", en: "Enable motion detection" },
        "motionDetect.roi": { ru: "Зоны детекции движения", en: "Motion detection regions" },
        "motionDetect.threshold": { ru: "Порог движения", en: "Motion threshold" },
        "motionDetect.sensitivity": { ru: "Чувствительность движения", en: "Motion sensitivity" },
        "motionDetect.visualize": { ru: "Показывать зоны движения", en: "Show motion regions" },
        "nightMode.enabled": { ru: "Ночной режим", en: "Night mode" },
        "nightMode.lightMonitor": { ru: "Автоматическое переключение день/ночь", en: "Automatic day/night switching" },
        "nightMode.irCutPin1": { ru: "IR-cut GPIO 1", en: "IR-cut GPIO 1" },
        "nightMode.irCutPin2": { ru: "IR-cut GPIO 2", en: "IR-cut GPIO 2" },
        "nightMode.backlightPin": { ru: "GPIO ИК-подсветки", en: "IR light GPIO" },
        "isp.sensorConfig": { ru: "Конфигурация сенсора", en: "Sensor configuration" },
        "isp.antiflicker": { ru: "Подавление мерцания", en: "Anti-flicker" },
        "osd.enabled": { ru: "Включить OSD", en: "Enable OSD" },
        "osd.template": { ru: "Текст OSD", en: "OSD text" },
        "osd.posX": { ru: "OSD позиция X", en: "OSD X position" },
        "osd.posY": { ru: "OSD позиция Y", en: "OSD Y position" },
        "outgoing.enabled": { ru: "Включить исходящую трансляцию", en: "Enable outgoing stream" },
        "outgoing.server": { ru: "Сервер трансляции", en: "Streaming server" },
        "outgoing.endpoint": { ru: "Путь/ключ трансляции", en: "Streaming endpoint/key" },
        "outgoing.stream": { ru: "Транслируемый поток", en: "Stream to publish" },
        "ipeye.enabled": { ru: "Включить IPEYE", en: "Enable IPEYE" },
        "onvif.enabled": { ru: "Включить ONVIF", en: "Enable ONVIF" },
        "onvif.port": { ru: "ONVIF-порт", en: "ONVIF port" },
        "mqtt.enabled": { ru: "Включить MQTT", en: "Enable MQTT" },
        "mqtt.server": { ru: "MQTT-сервер", en: "MQTT server" },
        "mqtt.username": { ru: "Пользователь MQTT", en: "MQTT username" },
        "mqtt.password": { ru: "Пароль MQTT", en: "MQTT password" },
        "srt.enabled": { ru: "Включить SRT", en: "Enable SRT" },
        "srt.port": { ru: "SRT-порт", en: "SRT port" },
        "system.webAdmin": { ru: "Включить WebUI камеры", en: "Enable camera WebUI" },
        "system.httpPort": { ru: "HTTP-порт WebUI", en: "WebUI HTTP port" },
        "system.logLevel": { ru: "Детальность логов Majestic", en: "Majestic log verbosity" }
    })
    readonly property var majesticHintText: ({
        enabled: { ru: "Включает или отключает этот модуль Majestic.", en: "Enables or disables this Majestic module." },
        codec: { ru: "Выбирает формат кодирования. H.265 экономит битрейт, H.264 обычно совместимее.", en: "Selects the encoding format. H.265 saves bitrate; H.264 is usually more compatible." },
        size: { ru: "Задаёт разрешение кадра. Для структурных изменений после сохранения нужен reload pipeline.", en: "Sets frame resolution. Structural changes require pipeline reload after saving." },
        fps: { ru: "Количество кадров в секунду. Больше FPS — плавнее видео, но выше нагрузка и битрейт.", en: "Frames per second. Higher FPS is smoother, but increases load and bitrate." },
        bitrate: { ru: "Целевой битрейт видеопотока в kbit/s.", en: "Target video stream bitrate in kbit/s." },
        rcMode: { ru: "Определяет, как кодек держит битрейт: постоянный, переменный или по качеству.", en: "Controls how the encoder maintains bitrate: constant, variable, or quality-based." },
        qfactor: { ru: "Качество JPEG: выше значение — лучше качество и больше размер файла.", en: "JPEG quality: higher value means better quality and larger files." },
        path: { ru: "Путь на камере, куда Majestic будет писать файлы.", en: "Camera-side path where Majestic writes files." },
        split: { ru: "Через сколько минут закрывать текущий файл записи и начинать новый.", en: "How many minutes before closing the current recording file and starting a new one." },
        maxUsage: { ru: "Максимальный процент заполнения носителя, после которого старые записи будут удаляться.", en: "Maximum storage usage percent before old recordings are removed." },
        port: { ru: "Сетевой порт сервиса. После изменения может потребоваться переподключение.", en: "Service network port. Changing it may require reconnecting." },
        password: { ru: "Секретное значение. В diff и подтверждениях приложение скрывает его содержимое.", en: "Secret value. The app masks it in diffs and confirmations." },
        server: { ru: "Адрес внешнего сервера или сервиса для интеграции.", en: "External server or service address for integration." },
        volume: { ru: "Уровень громкости входного аудио.", en: "Input audio volume level." },
        outputVolume: { ru: "Уровень громкости динамика камеры.", en: "Camera speaker output volume." },
        outputEnabled: { ru: "Включает аудиовыход на динамик камеры.", en: "Enables audio output to the camera speaker." },
        dual: { ru: "Разрешает двустороннее аудио, если аппаратная платформа это поддерживает.", en: "Enables two-way audio when supported by the hardware platform." },
        jitterBufferMs: { ru: "Буфер сглаживает сетевые рывки аудио, но добавляет задержку.", en: "Buffer smooths audio network jitter, but adds latency." },
        luminance: { ru: "Live ISP-настройка яркости. Применяется сразу и сохраняется кнопкой Save.", en: "Live ISP brightness control. Applies immediately and is persisted with Save." },
        contrast: { ru: "Live ISP-настройка контраста. Применяется сразу и сохраняется кнопкой Save.", en: "Live ISP contrast control. Applies immediately and is persisted with Save." },
        saturation: { ru: "Live ISP-настройка насыщенности цвета.", en: "Live ISP color saturation control." },
        hue: { ru: "Live ISP-настройка оттенка цвета.", en: "Live ISP color hue control." },
        mirror: { ru: "Зеркально отражает кадр по горизонтали.", en: "Mirrors the frame horizontally." },
        flip: { ru: "Переворачивает кадр по вертикали.", en: "Flips the frame vertically." },
        rotate: { ru: "Поворачивает кадр. Может требовать reload pipeline.", en: "Rotates the frame. May require pipeline reload." },
        crop: { ru: "Обрезает поток до указанной области кадра. Пустое значение означает полный кадр.", en: "Crops the stream to a selected frame region. Empty value keeps the full frame." },
        gopSize: { ru: "Интервал ключевых кадров. Меньше GOP ускоряет перемотку/восстановление, но повышает битрейт.", en: "Keyframe interval. Smaller GOP improves seeking/recovery but increases bitrate." },
        profile: { ru: "Профиль кодека влияет на совместимость декодеров и эффективность сжатия.", en: "Codec profile affects decoder compatibility and compression efficiency." },
        roi: { ru: "Список прямоугольников в формате XxYxWxH.", en: "List of rectangles in XxYxWxH format." },
        threshold: { ru: "Порог срабатывания: меньше значение — выше чувствительность.", en: "Trigger threshold: lower value means higher sensitivity." },
        sensitivity: { ru: "Чувствительность алгоритма к изменениям в кадре.", en: "Algorithm sensitivity to frame changes." },
        template: { ru: "Шаблон текста, который будет наложен на видео.", en: "Text template overlaid on the video." },
        sensorConfig: { ru: "Файл профиля сенсора. Неверный профиль может ухудшить изображение.", en: "Sensor profile file. Wrong profile can degrade image quality." },
        antiflicker: { ru: "Снижает мерцание от источников света 50/60 Гц.", en: "Reduces flicker from 50/60 Hz lighting." },
        lightMonitor: { ru: "Автоматически переключает день/ночь по освещённости, если сенсор поддерживает мониторинг.", en: "Automatically switches day/night by light level when the sensor supports monitoring." },
        irCutPin1: { ru: "GPIO первой линии механического IR-cut фильтра.", en: "GPIO for the first mechanical IR-cut control line." },
        irCutPin2: { ru: "GPIO второй линии механического IR-cut фильтра.", en: "GPIO for the second mechanical IR-cut control line." },
        backlightPin: { ru: "GPIO управления ИК-подсветкой.", en: "GPIO used to control the IR light." },
        dhcp: { ru: "Получать сетевые параметры автоматически от роутера.", en: "Obtain network settings automatically from the router." },
        gateway: { ru: "Адрес шлюза для выхода камеры за пределы локальной сети.", en: "Gateway address used to reach networks outside the local subnet." },
        dns: { ru: "DNS-серверы для разрешения доменных имён.", en: "DNS servers used to resolve domain names." },
        mac: { ru: "MAC-адрес сетевого интерфейса камеры.", en: "MAC address of the camera network interface." },
        endpoint: { ru: "Путь, ключ или URL назначения для внешней трансляции.", en: "Destination path, key, or URL for an outgoing stream." },
        token: { ru: "Секретный токен интеграции. Храните его как пароль.", en: "Secret integration token. Treat it like a password." },
        stream: { ru: "Выбирает поток Majestic, который будет использовать этот модуль.", en: "Selects which Majestic stream this module uses." },
        mode: { ru: "Режим работы параметра или модуля.", en: "Operating mode for the setting or module." },
        debug: { ru: "Включает расширенную диагностику. Используйте временно, чтобы не засорять логи.", en: "Enables extended diagnostics. Use temporarily to avoid noisy logs." },
        interval: { ru: "Интервал повторной проверки или отправки события.", en: "Interval for repeated checks or event publishing." },
        timeout: { ru: "Сколько ждать ответа перед ошибкой операции.", en: "How long to wait for a response before failing the operation." },
        color: { ru: "Цвет элемента, обычно в формате RGB/HEX.", en: "Element color, usually in RGB/HEX format." },
        alpha: { ru: "Прозрачность элемента: меньше значение — прозрачнее.", en: "Element opacity: lower value means more transparent." },
        fontSize: { ru: "Размер шрифта экранной надписи.", en: "On-screen text font size." },
        posX: { ru: "Горизонтальная позиция элемента на кадре.", en: "Horizontal position of the element on the frame." },
        posY: { ru: "Вертикальная позиция элемента на кадре.", en: "Vertical position of the element on the frame." },
        webAdmin: { ru: "Управляет встроенным WebUI OpenIPC/Majestic на камере.", en: "Controls the built-in OpenIPC/Majestic WebUI on the camera." },
        logLevel: { ru: "Чем детальнее логи, тем проще диагностика, но выше шум в журнале.", en: "More verbose logs help diagnostics, but increase log noise." }
    })

    modal: true
    focus: true
    width: Math.min(parent ? parent.width - 32 : 1180, 1180)
    height: Math.min(parent ? parent.height - 32 : 820, 820)
    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape
    padding: 0

    function track(id) { if (id && id.length) requestIds[id] = true; return id }
    function untrack(id) { if (id && id.length) requestIds[id] = false }
    function owns(id) { return requestIds[id] === true }
    function clone(value) {
        if (value === undefined) return undefined
        return JSON.parse(JSON.stringify(value))
    }
    function equal(a, b) { return JSON.stringify(a) === JSON.stringify(b) }
    function boolValue(value) { return value === true || value === 1 || String(value).toLowerCase() === "true" }
    function l10n(entry, fallback) {
        I18n.language
        if (entry && entry.ru !== undefined && entry.en !== undefined) {
            return I18n.language === "en" ? entry.en : entry.ru
        }
        return I18n.t(fallback || "")
    }
    function lastPathPart(path) {
        var parts = String(path || "").split(".")
        return parts.length ? parts[parts.length - 1] : String(path || "")
    }
    function titleCaseToken(token) {
        var text = String(token || "")
            .replace(/([a-z0-9])([A-Z])/g, "$1 $2")
            .replace(/[_-]+/g, " ")
            .trim()
        return text.length ? text.charAt(0).toUpperCase() + text.slice(1) : text
    }
    function videoSectionLabel(sectionId) {
        var match = String(sectionId || "").match(/^video([0-9]+)$/)
        if (!match) return ""
        var index = Number(match[1])
        if (index === 0) return l10n({ ru: "Основной поток", en: "Main stream" })
        if (index === 1) return l10n({ ru: "Дополнительный поток", en: "Sub stream" })
        return I18n.language === "en" ? "Stream " + (index + 1) : "Поток " + (index + 1)
    }
    function localizedSectionLabel(sectionId, schemaLabel) {
        I18n.language
        if (majesticSectionText[sectionId]) return l10n(majesticSectionText[sectionId])
        var streamLabel = videoSectionLabel(sectionId)
        if (streamLabel.length) return streamLabel
        if (schemaLabel && String(schemaLabel).length) return I18n.t(schemaLabel)
        return titleCaseToken(sectionId)
    }
    function localizedGroupLabel(group) {
        I18n.language
        var id = typeof group === "string" ? group : (group ? group.id : "")
        var schemaLabel = typeof group === "string" ? "" : (group ? group.label : "")
        if (majesticGroupText[id]) return l10n(majesticGroupText[id])
        if (schemaLabel && String(schemaLabel).length) return I18n.t(schemaLabel)
        return titleCaseToken(id)
    }
    function localizedKeyLabel(key) {
        if (majesticKeyText[key]) return l10n(majesticKeyText[key])
        return titleCaseToken(key)
    }
    function genericFieldHint(field, key) {
        if (!field) return ""
        if (field.enumValues && field.enumValues.length > 0) {
            return l10n({
                ru: "Выберите одно из значений, объявленных schema этой камеры.",
                en: "Select one of the values advertised by this camera schema."
            })
        }
        if (field.type === "boolean") {
            return l10n({
                ru: "Включает или отключает этот параметр Majestic.",
                en: "Enables or disables this Majestic setting."
            })
        }
        if (field.type === "array") {
            return l10n({
                ru: "Список значений. Формат каждого элемента задаётся schema камеры.",
                en: "List of values. Item format is defined by the camera schema."
            })
        }
        if (field.minimum !== undefined && field.maximum !== undefined) {
            var range = field.minimum + " … " + field.maximum
            return I18n.language === "en"
                    ? "Allowed range: " + range + "."
                    : "Допустимый диапазон: " + range + "."
        }
        if (String(field.path || "").indexOf("video") === 0) {
            return l10n({
                ru: "Параметр видеопотока Majestic. Для codec/resolution/fps/bitrate после сохранения обычно нужен reload pipeline.",
                en: "Majestic video stream setting. Codec/resolution/FPS/bitrate usually require pipeline reload after saving."
            })
        }
        if (key === "url" || key === "uri" || key === "endpoint" || key === "server" || key === "host") {
            return l10n({
                ru: "Сетевой адрес или endpoint интеграции. Проверьте протокол, порт и доступность сервера.",
                en: "Network address or integration endpoint. Check protocol, port, and server reachability."
            })
        }
        return l10n({
            ru: "Параметр Majestic из schema этой камеры; неподдерживаемые поля скрыты автоматически.",
            en: "Majestic setting from this camera schema; unsupported fields are hidden automatically."
        })
    }
    function localizedFieldTitle(field) {
        I18n.language
        if (!field) return ""
        if (majesticFieldText[field.path]) return l10n(majesticFieldText[field.path])
        var key = lastPathPart(field.path || field.key)
        var generic = localizedKeyLabel(key)
        var section = localizedSectionLabel(field.section, field.sectionLabel)
        if (section.length && generic.length && section !== generic) return section + " — " + generic
        if (field.title && String(field.title).length) return I18n.t(field.title)
        return generic || field.path
    }
    function localizedFieldHint(field) {
        I18n.language
        if (!field) return I18n.t("Параметр Majestic из schema этой камеры")
        var hint = ""
        if (majesticHintText[field.path]) hint = l10n(majesticHintText[field.path])
        var key = lastPathPart(field.path || field.key)
        if (!hint.length && majesticHintText[key]) hint = l10n(majesticHintText[key])
        if (!hint.length && field.hint && String(field.hint).length) hint = I18n.t(field.hint)
        if (!hint.length && field.description && String(field.description).length) hint = I18n.t(field.description)
        if (!hint.length) hint = genericFieldHint(field, key)
        if (field.live !== true && String(hint).toLowerCase().indexOf("reload") < 0) {
            hint += " " + I18n.t("После сохранения потребуется reload pipeline.")
        }
        return hint
    }

    function refresh() {
        if (!cameraHost.length) return
        loading = true
        statusError = false
        statusText = I18n.t("Чтение конфигурации Majestic…")
        activeLoadId = track(SystemController.majesticClient.loadConfiguration(
                                 cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function refreshMetrics() {
        if (!cameraHost.length) return
        statusError = false
        statusText = I18n.t("Обновление метрик Majestic…")
        activeMetricsId = track(SystemController.majesticClient.loadMetrics(
                                    cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function refreshOverviewMetrics() {
        activeMetricsId = track(SystemController.majesticClient.loadMetrics(
                                    cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function parseMajesticJson(text) {
        return SystemController.majesticClient.parseJsonObject(text)
    }

    function loadFirmwareStatus() {
        if (!cameraHost.length) return
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Чтение статуса прошивки…")
        activeFirmwareStatusId = track(SystemController.firmwareClient.loadStatus(
                                           cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function loadFirmwareNetwork() {
        if (!cameraHost.length) return
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Чтение сетевых настроек OpenIPC…")
        activeFirmwareNetworkId = track(SystemController.firmwareClient.loadNetwork(
                                            cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function saveFirmwareNetwork() {
        var settings = {
            hostname: networkPage.hostname,
            interface: networkPage.interfaceName,
            dhcp: networkPage.dhcpEnabled,
            address: networkPage.address,
            netmask: networkPage.netmask,
            gateway: networkPage.gateway,
            nameserver: networkPage.nameserver,
            wlanSsid: networkPage.wlanSsid,
            wlanPassword: networkPage.wlanPassword
        }
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Сохранение сетевых настроек OpenIPC…")
        activeFirmwareNetworkSaveId = track(SystemController.firmwareClient.saveNetwork(
                                                cameraHost, cameraPort, cameraUser, cameraPassword, settings))
    }

    function resetFirmwareNetwork() {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Сброс сетевой конфигурации OpenIPC…")
        activeFirmwareNetworkSaveId = track(SystemController.firmwareClient.resetNetwork(
                                                cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function scanFirmwareWifi() {
        if (!cameraHost.length) return
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Сканирование Wi‑Fi сетей камерой…")
        activeFirmwareNetworkId = track(SystemController.firmwareClient.scanWifi(
                                            cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function loadFirmwareTime() {
        if (!cameraHost.length) return
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Чтение настроек времени OpenIPC…")
        activeFirmwareTimeId = track(SystemController.firmwareClient.loadTime(
                                         cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function saveFirmwareTime() {
        var settings = {
            zoneName: timePage.zoneName,
            zoneData: timePage.zoneData,
            servers: [timePage.server0, timePage.server1, timePage.server2, timePage.server3]
        }
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Сохранение настроек времени OpenIPC…")
        activeFirmwareTimeSaveId = track(SystemController.firmwareClient.saveTime(
                                             cameraHost, cameraPort, cameraUser, cameraPassword, settings))
    }

    function syncFirmwareTime(setFromComputer) {
        firmwareBusy = true
        statusError = false
        statusText = setFromComputer ? I18n.t("Установка времени камеры с компьютера…")
                                     : I18n.t("Синхронизация времени камеры через NTP…")
        activeFirmwareTimeSaveId = track(SystemController.firmwareClient.syncTime(
                                             cameraHost, cameraPort, cameraUser, cameraPassword, setFromComputer))
    }

    function loadFirmwareLogs(source, silent) {
        firmwareLogsSource = source || "all"
        if (!silent) {
            firmwareBusy = true
            statusError = false
            statusText = I18n.t("Чтение логов OpenIPC…")
        }
        activeFirmwareLogsId = track(SystemController.firmwareClient.loadLogs(
                                         cameraHost, cameraPort, cameraUser, cameraPassword,
                                         firmwareLogsSource, 300))
    }

    function appendFirmwareLogText(text) {
        if (!text || firmwareLogsPaused) return
        var merged = firmwareLogsText
        if (merged.length && merged.charAt(merged.length - 1) !== "\n") merged += "\n"
        merged += String(text)
        var lines = merged.split("\n")
        if (lines.length > firmwareLogLineLimit) lines = lines.slice(lines.length - firmwareLogLineLimit)
        firmwareLogsText = lines.join("\n")
    }

    function htmlEscape(text) {
        return String(text || "")
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#39;")
    }

    function firmwareLogLineColor(line) {
        var lower = String(line || "").toLowerCase()
        if (lower.indexOf("panic") >= 0 || lower.indexOf("fatal") >= 0
                || lower.indexOf("error") >= 0 || lower.indexOf("failed") >= 0
                || lower.indexOf("segfault") >= 0) return Theme.danger
        if (lower.indexOf("warn") >= 0 || lower.indexOf("timeout") >= 0
                || lower.indexOf("retry") >= 0) return Theme.warning
        if (lower.indexOf("majestic") >= 0) return Theme.accentHover
        if (lower.indexOf("kernel") >= 0 || lower.indexOf("dmesg") >= 0) return Theme.infoText
        return Theme.textSecondary
    }

    function filteredFirmwareLogsText() {
        var text = firmwareLogsText.length ? firmwareLogsText : I18n.t("Нажмите All, majestic или kernel, чтобы прочитать логи камеры.")
        var query = firmwareLogFilter.trim().toLowerCase()
        if (!query.length || !firmwareLogsText.length) return text
        var lines = firmwareLogsText.split("\n")
        var result = []
        for (var i = 0; i < lines.length; ++i) {
            if (lines[i].toLowerCase().indexOf(query) >= 0) result.push(lines[i])
        }
        return result.length ? result.join("\n") : I18n.t("Нет совпадений по фильтру")
    }

    function filteredFirmwareLogsHtml() {
        var lines = String(filteredFirmwareLogsText() || "").split("\n")
        var html = ""
        for (var i = 0; i < lines.length; ++i) {
            if (i > 0) html += "\n"
            html += "<span style=\"color:" + firmwareLogLineColor(lines[i]) + "\">"
                    + htmlEscape(lines[i]) + "</span>"
        }
        return "<pre style=\"margin:0; white-space:pre; font-family:Consolas,monospace; font-size:11px;\">"
                + html + "</pre>"
    }

    function exportFirmwareLogs(path) {
        if (!path || !String(path).length) return
        var normalized = SystemController.normalizeLocalPath(String(path))
        var ok = SystemController.saveTextFile(normalized, filteredFirmwareLogsText())
        statusError = !ok
        statusText = ok ? I18n.t("Логи экспортированы: %1", [normalized])
                        : I18n.t("Не удалось экспортировать логи.")
    }

    function setFirmwareLogBufferSize(sizeKiB) {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Настраиваю ring-buffer логов OpenIPC…")
        activeFirmwareLogsId = track(SystemController.firmwareClient.setLogBufferSize(
                                         cameraHost, cameraPort, cameraUser, cameraPassword,
                                         Math.round(Number(sizeKiB))))
    }

    function appendFirmwareUpgradeText(text) {
        if (!text) return
        firmwareUpgradeText += String(text)
        var lines = firmwareUpgradeText.split("\n")
        if (lines.length > firmwareLogLineLimit) lines = lines.slice(lines.length - firmwareLogLineLimit)
        firmwareUpgradeText = lines.join("\n")
    }

    function firmwareUpgradeTimestamp() {
        return Qt.formatTime(new Date(), "HH:mm:ss")
    }

    function appendFirmwareUpgradeLogLine(text) {
        appendFirmwareUpgradeText("[" + firmwareUpgradeTimestamp() + "] " + text + "\n")
    }

    function startFirmwareLiveLogs() {
        if (!cameraHost.length || firmwareLiveLogs) return
        firmwareLiveLogs = true
        firmwareLogsPaused = false
        statusError = false
        if (SystemController.firmwareClient.webSocketsAvailable) {
            statusText = I18n.t("Подключение live logs через /ws/logs…")
            activeFirmwareLiveLogsId = track(SystemController.firmwareClient.startLiveLogs(
                                                 cameraHost, cameraPort, cameraUser, cameraPassword))
        } else {
            statusText = I18n.t("Qt WebSockets недоступен: включён polling логов.")
            firmwareLiveLogsTimer.restart()
            loadFirmwareLogs(firmwareLogsSource, true)
        }
    }

    function stopFirmwareLiveLogs() {
        if (!firmwareLiveLogs) return
        firmwareLiveLogs = false
        firmwareLiveLogsTimer.stop()
        if (SystemController.firmwareClient.webSocketsAvailable) SystemController.firmwareClient.stopLiveLogs()
        statusError = false
        statusText = I18n.t("Live logs остановлены")
    }

    function refreshFirmwareUpdateInfo() {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Чтение информации об обновлении прошивки…")
        activeFirmwareUpdateId = track(SystemController.firmwareClient.loadUpdateInfo(
                                           cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function resetFirmwareArchiveState() {
        firmwareArchiveUploaded = false
        firmwareArchivePath = ""
        firmwareArchiveName = ""
        firmwareArchiveSizeBytes = 0
        firmwareArchiveInspection = ({})
    }

    function firmwareArchiveSizeText() {
        var size = Number(firmwareArchiveSizeBytes)
        if (!size || size <= 0) return I18n.t("архив не выбран")
        if (size >= 1048576) return (size / 1048576).toFixed(size >= 10485760 ? 0 : 1) + " MB"
        return Math.max(1, Math.round(size / 1024)) + " KB"
    }

    function firmwareArchiveSafetyProblem(path) {
        var info = SystemController.getFileInfo(String(path || ""))
        var name = String(info.fileName || "").toLowerCase()
        var size = Number(info.size || 0)
        if (info.exists !== true) return I18n.t("файл архива не найден")
        if (!(name.endsWith(".tgz") || name.endsWith(".tar.gz") || name.endsWith(".gz")))
            return I18n.t("firmware archive должен быть .tgz/.tar.gz/.gz")
        if (size <= 0) return I18n.t("firmware archive пустой")
        if (size > 128 * 1024 * 1024)
            return I18n.t("firmware archive больше safety-limit 128 MB")
        return ""
    }

    function firmwareArchiveCompatibility() {
        if (!firmwareArchiveName.length) {
            return { state: "warn", text: I18n.t("Локальный archive ещё не выбран.") }
        }
        var name = firmwareArchiveName.toLowerCase()
        var soc = firmwareSocText().toLowerCase()
        var family = String(firmwareUpdateInfo.socFamily || "").toLowerCase()
        var flash = firmwareFlashText().toLowerCase()
        var variant = firmwareVariantText().toLowerCase()
        var socMentioned = (soc.length > 3 && name.indexOf(soc) >= 0)
                || (family.length > 3 && name.indexOf(family) >= 0)
        if (name.indexOf("nand") >= 0 && flash.length && flash.indexOf("nand") < 0)
            return { state: "block", text: I18n.t("Archive похож на NAND, а камера сообщает Flash: %1", [firmwareFlashText()]) }
        if (name.indexOf("nor") >= 0 && flash.length && flash.indexOf("nor") < 0)
            return { state: "block", text: I18n.t("Archive похож на NOR, а камера сообщает Flash: %1", [firmwareFlashText()]) }
        if (name.indexOf("ultimate") >= 0 && variant.length && variant.indexOf("ultimate") < 0)
            return { state: "block", text: I18n.t("Archive variant ultimate не совпадает с камерой: %1", [firmwareVariantText() || "—"]) }
        if (name.indexOf("lite") >= 0 && variant.length && variant.indexOf("lite") < 0)
            return { state: "block", text: I18n.t("Archive variant lite не совпадает с камерой: %1", [firmwareVariantText() || "—"]) }
        if (!socMentioned && (soc.length || family.length))
            return { state: "warn", text: I18n.t("В имени archive не найден SoC камеры (%1). Проверьте совместимость вручную.", [firmwareSocText() || family]) }
        return { state: "ok", text: I18n.t("Archive выглядит совместимым с SoC/Flash/variant: %1 · %2 · %3", [firmwareSocText() || "—", firmwareFlashText() || "—", firmwareVariantText() || "—"]) }
    }

    function uploadFirmwareArchive(path) {
        var localPath = SystemController.normalizeLocalPath(String(path || ""))
        var problem = firmwareArchiveSafetyProblem(localPath)
        if (problem.length) {
            firmwareBusy = false
            statusError = true
            statusText = I18n.t("Firmware archive отклонён: %1", [problem])
            resetFirmwareArchiveState()
            return
        }
        var info = SystemController.getFileInfo(localPath)
        firmwareArchivePath = localPath
        firmwareArchiveName = String(info.fileName || "")
        firmwareArchiveSizeBytes = Number(info.size || 0)
        firmwareArchiveInspection = SystemController.inspectFirmwareArchive(localPath, firmwarePublishedChecksum())
        if (firmwareArchiveManifestState() === "block") {
            firmwareBusy = false
            statusError = true
            statusText = firmwareArchiveManifestSummary()
            firmwareArchiveUploaded = false
            return
        }
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Загрузка firmware-архива на камеру… %1 (%2)", [firmwareArchiveName, firmwareArchiveSizeText()])
        firmwareArchiveUploaded = false
        activeFirmwareUpdateId = track(SystemController.firmwareClient.uploadFirmwareArchive(
                                           cameraHost, cameraPort, cameraUser, cameraPassword, localPath))
    }

    function firmwareSocText() {
        var device = firmwareStatus.device || {}
        return String(firmwareUpdateInfo.socName || firmwareUpdateInfo.socFamily || firmwareUpdateInfo.soc || device.soc || "").trim()
    }

    function firmwareFlashText() {
        var storage = firmwareStatus.storage || {}
        return String(firmwareUpdateInfo.flashType || firmwareUpdateInfo.flash || storage.flash || "").trim()
    }

    function firmwareVariantText() {
        var device = firmwareStatus.device || {}
        return String(firmwareUpdateInfo.variant || device.firmware || "").trim()
    }

    function firmwareReturnSummary(status) {
        var device = status.device || {}
        var network = status.network || {}
        var pulse = status.pulse || {}
        var parts = []
        if (network.address) parts.push("IP " + network.address)
        if (device.firmware) parts.push(I18n.t("firmware %1", [device.firmware]))
        if (device.majestic) parts.push("Majestic " + device.majestic)
        if (pulse.uptime) parts.push("uptime " + pulse.uptime)
        return parts.length ? parts.join(" · ") : I18n.t("status endpoint ответил")
    }

    function firmwareUpdateOptionsSummary() {
        var text = ""
        if (firmwareUpdateKernel) text = "kernel"
        if (firmwareUpdateRootfs) text = text.length ? text + " + rootfs" : "rootfs"
        if (firmwareUpdateReset) text = text.length ? text + " + reset" : "reset"
        if (firmwareUpdateForce) text = text.length ? text + " + force" : "force"
        return text.length ? text : I18n.t("не выбрано")
    }

    function firmwareUpdateChecksumText() {
        var checksum = String(firmwareUpdateInfo.sha256 || firmwareUpdateInfo.checksum || firmwareUpdateInfo.digest || "")
        var signature = String(firmwareUpdateInfo.signature || "")
        if (checksum.length && signature.length) return I18n.t("checksum/signature: %1 / %2", [checksum, signature])
        if (checksum.length) return I18n.t("checksum: %1", [checksum])
        if (signature.length) return I18n.t("signature: %1", [signature])
        return I18n.t("checksum/signature не опубликованы update page — проверяем источник, размер и совместимость")
    }

    function firmwarePublishedChecksum() {
        return String(firmwareUpdateInfo.sha256 || firmwareUpdateInfo.checksum || firmwareUpdateInfo.digest || "").trim()
    }

    function firmwareArchiveManifestState() {
        if (!firmwareArchiveName.length) return "warn"
        return String(firmwareArchiveInspection.checksumStatus || "warn")
    }

    function firmwareArchiveManifestSummary() {
        if (!firmwareArchiveName.length)
            return I18n.t("Локальный firmware archive ещё не выбран. При upload приложение посчитает SHA-256 и проверит sidecar-файлы рядом с архивом.")
        var issues = firmwareArchiveInspection.issues || []
        if (issues.length > 0)
            return I18n.t("Archive заблокирован: %1", [issues.join("; ")])
        if (firmwareArchiveManifestState() === "ok")
            return I18n.t("Archive проверен: checksum совпадает с опубликованным значением или sidecar-файлом.")
        return I18n.t("Archive принят с предупреждением: опубликованный checksum или sidecar не найден, остаётся ручная проверка источника.")
    }

    function firmwareArchiveManifestRows() {
        var rows = [
            { label: I18n.t("Файл"), value: firmwareArchiveName.length ? firmwareArchiveName : I18n.t("не выбран") },
            { label: I18n.t("Размер"), value: firmwareArchiveSizeText() }
        ]
        if (!firmwareArchiveName.length) {
            rows.push({ label: "SHA-256", value: I18n.t("будет рассчитан при upload"), state: "warn" })
            return rows
        }
        rows.push({
            label: "SHA-256",
            value: firmwareArchiveInspection.sha256 || I18n.t("не рассчитан"),
            mono: true,
            state: firmwareArchiveInspection.sha256 ? "ok" : "warn"
        })
        rows.push({
            label: I18n.t("Update page"),
            value: firmwarePublishedChecksum() || I18n.t("checksum не опубликован"),
            mono: firmwarePublishedChecksum().length > 0,
            state: firmwarePublishedChecksum().length > 0
                   ? (firmwareArchiveInspection.expectedMatches === false ? "block" : "ok")
                   : "warn"
        })
        rows.push({
            label: I18n.t("Sidecar"),
            value: firmwareArchiveInspection.sidecarPath
                   ? firmwareArchiveInspection.sidecarPath
                   : I18n.t(".sha256/.sha256sum рядом с архивом не найден"),
            mono: true,
            state: firmwareArchiveInspection.sidecarPath
                   ? (firmwareArchiveInspection.sidecarMatches === false ? "block" : "ok")
                   : "warn"
        })
        rows.push({
            label: I18n.t("Signature"),
            value: firmwareArchiveInspection.signaturePath
                   ? firmwareArchiveInspection.signaturePath
                   : I18n.t(".sig/.asc/.minisig не найден"),
            mono: true,
            state: firmwareArchiveInspection.signaturePath ? "ok" : "warn"
        })
        return rows
    }

    function firmwareDangerousOptionsActive() {
        return firmwareUpdateReset || firmwareUpdateForce
    }

    function firmwareUpdateBlockReason(source) {
        if (firmwareReturnPolling) return I18n.t("дождитесь возврата камеры после предыдущего update")
        if (!SystemController.firmwareClient.webSocketsAvailable) return I18n.t("Native /ws/upgrade недоступен в этой сборке")
        if (!firmwareUpdateKernel && !firmwareUpdateRootfs) return I18n.t("выберите хотя бы kernel или rootfs")
        if (!firmwareSocText().length || !firmwareFlashText().length) return I18n.t("сначала загрузите update-info: SoC и Flash не определены")
        if (source === "github" && firmwareUpdateInfo.githubAvailable !== true) return I18n.t("GitHub update недоступен по данным камеры")
        if (source === "uploaded" && !firmwareArchiveUploaded) return I18n.t("сначала загрузите firmware archive")
        if (source === "uploaded" && firmwareArchiveSizeBytes <= 0) return I18n.t("неизвестен размер загруженного firmware archive")
        if (source === "uploaded" && firmwareArchiveManifestState() === "block") return firmwareArchiveManifestSummary()
        if (source === "uploaded" && firmwareArchiveCompatibility().state === "block") return firmwareArchiveCompatibility().text
        if (!firmwarePowerSafetyConfirmed) return I18n.t("подтвердите стабильное питание и сеть")
        if (firmwareDangerousOptionsActive() && !firmwareDangerOptionsConfirmed)
            return I18n.t("подтвердите опасные опции reset/force")
        return ""
    }

    function canStartFirmwareUpdate(source) {
        return firmwareUpdateBlockReason(source).length === 0
    }

    function startFirmwareReturnPolling() {
        firmwareReturnPolling = true
        firmwareReturnPollTries = 0
        firmwareReturnPhase = "rebooting"
        firmwareReturnHealthText = I18n.t("Камера перезагружается после прошивки. Ждём ответ status endpoint.")
        activeFirmwareReturnProbeId = ""
        firmwarePostReturnProbeActive = false
        appendFirmwareUpgradeLogLine(I18n.t("Начинаю проверку возврата камеры после update."))
        firmwareReturnPollTimer.restart()
        statusError = false
        statusText = I18n.t("Ожидание возврата камеры после update…")
    }

    function stopFirmwareReturnPolling(message, error) {
        firmwareReturnPolling = false
        firmwareReturnPollTimer.stop()
        if (activeFirmwareReturnProbeId.length) {
            untrack(activeFirmwareReturnProbeId)
            activeFirmwareReturnProbeId = ""
        }
        firmwareBusy = false
        firmwareUpgradeRebooting = false
        if (error === true) firmwarePostReturnProbeActive = false
        firmwareReturnPhase = error === true ? "failed" : "online"
        firmwareReturnHealthText = message || ""
        statusError = error === true
        if (message && message.length) statusText = message
    }

    function startPostFirmwareReturnProbes() {
        firmwarePostReturnProbeActive = true
        firmwareReturnPhase = "validating"
        firmwareReturnHealthText = I18n.t("Камера вернулась. Проверяю Majestic API и RTSP потоки…")
        appendFirmwareUpgradeLogLine(I18n.t("Post-upgrade health probe: Majestic API + RTSP main/sub."))
        startEndpointProbe("api")
        startEndpointProbe("main")
        startEndpointProbe("sub")
    }

    function postFirmwareReturnProbeSummary() {
        return "API: " + probeStateText(majesticApiProbeState, majesticApiProbeMessage, majesticApiProbeElapsedMs)
                + " · RTSP main: " + probeStateText(rtspMainProbeState, rtspMainProbeMessage, rtspMainProbeElapsedMs)
                + " · RTSP sub: " + probeStateText(rtspSubProbeState, rtspSubProbeMessage, rtspSubProbeElapsedMs)
    }

    function maybeFinishPostFirmwareReturnProbes() {
        if (!firmwarePostReturnProbeActive) return
        if (majesticApiProbeState === "running" || rtspMainProbeState === "running" || rtspSubProbeState === "running") return
        if (majesticApiProbeState === "idle" || rtspMainProbeState === "idle" || rtspSubProbeState === "idle") return
        firmwarePostReturnProbeActive = false
        var healthy = majesticApiProbeState === "ok" && rtspMainProbeState === "ok"
        firmwareReturnPhase = healthy ? "online" : "degraded"
        firmwareReturnHealthText = postFirmwareReturnProbeSummary()
        appendFirmwareUpgradeLogLine((healthy
                                      ? I18n.t("Post-upgrade health OK: %1", [firmwareReturnHealthText])
                                      : I18n.t("Post-upgrade health degraded: %1", [firmwareReturnHealthText])))
    }

    function probeFirmwareReturn() {
        if (!firmwareReturnPolling || activeFirmwareReturnProbeId.length) return
        if (firmwareReturnPollTries >= firmwareReturnPollMaxTries) {
            stopFirmwareReturnPolling(I18n.t("Камера не вернулась после update. Проверьте питание и сеть."), true)
            return
        }
        firmwareReturnPollTries += 1
        firmwareReturnPhase = "probing"
        firmwareReturnHealthText = I18n.t("Проверяю status endpoint камеры…")
        appendFirmwareUpgradeLogLine(I18n.t("Проверка возврата камеры: попытка %1/%2", [firmwareReturnPollTries, firmwareReturnPollMaxTries]))
        statusError = false
        statusText = I18n.t("Ожидание возврата камеры… попытка %1/%2", [firmwareReturnPollTries, firmwareReturnPollMaxTries])
        activeFirmwareReturnProbeId = track(SystemController.firmwareClient.loadStatus(
                                                cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function startGithubFirmwareUpdate() {
        var blockReason = firmwareUpdateBlockReason("github")
        if (blockReason.length) {
            firmwareBusy = false
            statusError = true
            statusText = I18n.t("Firmware update заблокирован: %1", [blockReason])
            return
        }
        firmwareBusy = true
        statusError = false
        firmwareUpgradeText = ""
        firmwareUpgradeRebooting = false
        resetFirmwareArchiveState()
        statusText = I18n.t("Запуск updater OpenIPC…")
        appendFirmwareUpgradeLogLine(I18n.t("Старт GitHub firmware update через /ws/upgrade. Опции: %1", [firmwareUpdateOptionsSummary()]))
        activeFirmwareUpdateId = track(SystemController.firmwareClient.startGithubUpdate(
                                           cameraHost, cameraPort, cameraUser, cameraPassword,
                                           firmwareUpdateKernel, firmwareUpdateRootfs, firmwareUpdateReset, firmwareUpdateForce))
    }

    function startUploadedFirmwareUpdate() {
        var blockReason = firmwareUpdateBlockReason("uploaded")
        if (blockReason.length) {
            firmwareBusy = false
            statusError = true
            statusText = I18n.t("Firmware update заблокирован: %1", [blockReason])
            return
        }
        firmwareBusy = true
        statusError = false
        firmwareUpgradeText = ""
        firmwareUpgradeRebooting = false
        statusText = I18n.t("Запуск updater для /tmp/firmware.tgz…")
        appendFirmwareUpgradeLogLine(I18n.t("Старт uploaded firmware update через /ws/upgrade. Опции: %1", [firmwareUpdateOptionsSummary()]))
        activeFirmwareUpdateId = track(SystemController.firmwareClient.startFirmwareUpgrade(
                                           cameraHost, cameraPort, cameraUser, cameraPassword,
                                           "/tmp/firmware.tgz",
                                           firmwareUpdateKernel, firmwareUpdateRootfs, firmwareUpdateReset, firmwareUpdateForce))
    }

    function requestFirmwareReboot() {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Отправляю reboot на камеру…")
        activeFirmwareRebootId = track(SystemController.firmwareClient.reboot(
                                           cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function saveFullFirmwareBackup(path) {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Создание firmware backup…")
        activeFirmwareBackupId = track(SystemController.firmwareClient.saveFirmwareBackup(
                                            cameraHost, cameraPort, cameraUser, cameraPassword, path))
    }

    function openSnapshotDialog() { snapshotDialog.open() }
    function openSaveBackupDialog() { saveBackupDialog.open() }
    function openBackupRestoreDialog() { openBackupDialog.open() }
    function openPcmDialog() { pcmDialog.open() }
    function openFirmwareBackupDialog() { firmwareBackupDialog.open() }
    function openFirmwareUploadDialog() { firmwareUploadDialog.open() }
    function openFirmwareRestoreWebUiConfirm() { firmwareRestoreWebUiConfirm.open() }
    function openFirmwareNetworkConfirm() { firmwareNetworkConfirm.open() }
    function openFirmwareNetworkResetConfirm() { firmwareNetworkResetConfirm.open() }
    function openFirmwareTimeConfirm() { firmwareTimeConfirm.open() }
    function openFirmwareRebootConfirm() { firmwareRebootConfirm.open() }
    function openGithubFirmwareUpdateConfirm() { firmwareUpdateConfirm.open() }
    function openUploadedFirmwareUpdateConfirm() { firmwareUploadedUpdateConfirm.open() }
    function openRollbackConfirm() { rollbackConfirm.open() }

    function setNightMode(mode) {
        track(SystemController.majesticClient.setNightMode(cameraHost, cameraPort, cameraUser, cameraPassword, mode))
    }

    function applyFirmwareNetworkToEditors(network) {
        var current = network.current || {}
        networkPage.hostname = network.hostname || current.hostname || ""
        var iface = network.interface || current.interface || "eth0"
        networkPage.interfaceIndex = iface === "wlan0" ? 1 : 0
        networkPage.dhcpEnabled = network.dhcp === true || String((current.mode || "")).toLowerCase() === "dhcp"
        networkPage.address = network.address || current.address || ""
        networkPage.netmask = network.netmask || current.netmask || ""
        networkPage.gateway = network.gateway || current.gateway || ""
        networkPage.nameserver = network.nameserver || current.nameserver || ""
        networkPage.wlanSsid = network.wlanSsid || ""
        networkPage.wlanPassword = network.wlanPassword || ""
    }

    function applyFirmwareTimeToEditors(time) {
        var servers = time.servers || []
        timePage.zoneName = time.zoneName || time.currentZoneName || ""
        timePage.zoneData = time.zoneData || time.currentZoneData || ""
        timePage.server0 = servers.length > 0 ? servers[0] : ""
        timePage.server1 = servers.length > 1 ? servers[1] : ""
        timePage.server2 = servers.length > 2 ? servers[2] : ""
        timePage.server3 = servers.length > 3 ? servers[3] : ""
    }

    function setDotted(object, path, value) {
        var parts = path.split(".")
        var cursor = object
        for (var i = 0; i < parts.length - 1; ++i) {
            if (!cursor[parts[i]] || typeof cursor[parts[i]] !== "object") cursor[parts[i]] = {}
            cursor = cursor[parts[i]]
        }
        cursor[parts[parts.length - 1]] = value
    }

    function resetDraft() {
        var values = {}
        var groupList = []
        var seen = {}
        var keepSelectedGroup = selectedGroupId
        var selectedStillExists = false
        for (var i = 0; i < fields.length; ++i) {
            var field = fields[i]
            values[field.path] = clone(field.value)
            if (!seen[field.groupId]) {
                seen[field.groupId] = true
                groupList.push({ id: field.groupId, label: field.groupLabel || field.groupId })
            }
            if (field.groupId === keepSelectedGroup) selectedStillExists = true
        }
        draftValues = values
        groups = groupList
        selectedGroupId = selectedStillExists ? keepSelectedGroup : (groupList.length ? groupList[0].id : "")
        dirtyCount = 0
        revision++
    }

    function fieldDirty(field) {
        return field && field.path && !equal(field.value, draftValues[field.path])
    }

    function editedConfig() {
        var result = clone(originalConfig)
        for (var i = 0; i < fields.length; ++i) {
            if (fieldDirty(fields[i])) {
                setDotted(result, fields[i].path, draftValues[fields[i].path])
            }
        }
        return result
    }

    function recomputeDirty() {
        var count = 0
        for (var i = 0; i < fields.length; ++i)
            if (fieldDirty(fields[i])) count++
        dirtyCount = count
    }

    function updateDraft(field, value) {
        draftValues[field.path] = value
        revision++
        recomputeDirty()
        if (field.live && settingsPage.livePreviewChecked) liveImageTimer.restart()
    }

    function valueText(field, value) {
        if (value === undefined || value === null) return ""
        if (field.type === "array" && value.join) return value.join(", ")
        return typeof value === "object" ? JSON.stringify(value) : String(value)
    }

    function parsedValue(field, text) {
        if (field.type === "integer") { var i = parseInt(text); return isNaN(i) ? field.value : i }
        if (field.type === "number") { var n = Number(text); return isNaN(n) ? field.value : n }
        if (field.type === "array")
            return text.split(",").map(function(v) { return v.trim() }).filter(function(v) { return v.length })
        return text
    }

    function enumIndex(field) {
        revision
        var model = field.enumValues || []
        for (var i = 0; i < model.length; ++i)
            if (equal(model[i], draftValues[field.path]) || String(model[i]) === String(draftValues[field.path])) return i
        return -1
    }

    function fieldMatches(field) {
        revision
        var groupOk = selectedGroupId.length > 0 && field.groupId === selectedGroupId
        var condition = field.visibleWhen || {}
        if (condition.field !== undefined) {
            var parent = field.path.substring(0, field.path.lastIndexOf("."))
            var controller = parent + "." + condition.field
            if (String(draftValues[controller]) !== String(condition.equals)) return false
        }
        var needle = settingsPage.searchText.trim().toLowerCase()
        return groupOk && (!needle.length || localizedFieldTitle(field).toLowerCase().indexOf(needle) >= 0
                           || String(field.path).toLowerCase().indexOf(needle) >= 0
                           || localizedFieldHint(field).toLowerCase().indexOf(needle) >= 0
                           || String(field.description).toLowerCase().indexOf(needle) >= 0)
    }

    function prepareApply(edited) {
        var changes = SystemController.majesticClient.describeChanges(originalConfig, edited)
        if (!changes.length) { statusError = false; statusText = I18n.t("Изменений нет"); return }
        pendingPatch = SystemController.majesticClient.buildPatch(originalConfig, edited)
        pendingChanges = changes
        pendingPipelineReloadNeeded = false
        for (var i = 0; i < changes.length; ++i) {
            if (fieldRequiresPipelineReload(changes[i].path)) {
                pendingPipelineReloadNeeded = true
                break
            }
        }
        if (pendingPipelineReloadNeeded) autoReloadAfterApply = true
        applyConfirm.open()
    }

    function applyPending() {
        rememberRollbackSnapshot(pendingChanges, I18n.t("Rollback snapshot создан перед применением %1 критичных изменений.", [criticalChanges(pendingChanges).length]))
        loading = true
        statusError = false
        pendingAutoReloadAfterApply = pendingPipelineReloadNeeded && autoReloadAfterApply && capabilities.pipelineReload === true
        statusText = I18n.t("Сохранение конфигурации…")
        activeApplyId = track(SystemController.majesticClient.applyConfiguration(
                                  cameraHost, cameraPort, cameraUser, cameraPassword, pendingPatch))
        applyWatchdogTimer.restart()
    }

    function triggerPipelineReload(message) {
        loading = true
        statusError = false
        statusText = I18n.t(message && String(message).length ? message : "Отправляю reload pipeline…")
        activeReloadId = track(SystemController.majesticClient.reloadPipeline(
                                   cameraHost, cameraPort, cameraUser, cameraPassword))
        reloadWatchdogTimer.restart()
    }

    function requestReset(path) {
        activeResetNeedsPipelineReload = fieldRequiresPipelineReload(path)
        activeResetId = track(SystemController.majesticClient.resetConfigurationFields(
                                   cameraHost, cameraPort, cameraUser, cameraPassword, [path]))
    }

    function requestResetMany(paths) {
        if (!paths || !paths.length) return
        activeResetNeedsPipelineReload = false
        for (var i = 0; i < paths.length; ++i) {
            if (fieldRequiresPipelineReload(paths[i])) {
                activeResetNeedsPipelineReload = true
                break
            }
        }
        activeResetId = track(SystemController.majesticClient.resetConfigurationFields(
                                  cameraHost, cameraPort, cameraUser, cameraPassword, paths))
    }

    function capabilityRows() {
        return [
            { label: I18n.t("JSON Schema"), value: capabilities.schema === true },
            { label: I18n.t("Безопасная запись"), value: capabilities.configWrite === true },
            { label: I18n.t("Сброс значений"), value: capabilities.resetDefaults === true },
            { label: I18n.t("Live ISP"), value: capabilities.liveImage === true },
            { label: I18n.t("Метрики"), value: capabilities.metrics === true },
            { label: I18n.t("Двустороннее аудио"), value: capabilities.playAudio === true },
            { label: "Qt WebSockets", value: SystemController.firmwareClient.webSocketsAvailable },
            { label: I18n.t("OpenIPC firmware API"), value: true }
        ]
    }

    function safeActionRows() {
        var uploadedBlockReason = firmwareUpdateBlockReason("uploaded")
        var githubBlockReason = firmwareUpdateBlockReason("github")
        return [
            {
                title: I18n.t("Majestic diff apply"),
                text: I18n.t("Отправляет только изменённые ключи, неизвестные поля текущей камеры не удаляются."),
                guard: capabilities.configWrite === true
                       ? I18n.t("Защита: schema, redaction секретов, rollback snapshot для критичных изменений.")
                       : I18n.t("Закрыто: камера не отдала schema/config write capability."),
                enabled: capabilities.configWrite === true,
                level: "safe"
            },
            {
                title: I18n.t("Pipeline reload"),
                text: I18n.t("Применяет codec/resolution/FPS без reboot камеры, но поток кратко прерывается."),
                guard: capabilities.pipelineReload === true
                       ? I18n.t("Защита: выполняется только после diff, который требует reload.")
                       : I18n.t("Закрыто: endpoint reload не подтверждён этой камерой."),
                enabled: capabilities.pipelineReload === true,
                level: "warn"
            },
            {
                title: I18n.t("Live ISP"),
                text: I18n.t("Меняет яркость/контраст/зеркало live без записи majestic.yaml."),
                guard: capabilities.liveImage === true
                       ? I18n.t("Защита: короткий debounce и только live-поля из schema.")
                       : I18n.t("Закрыто: live image endpoint не подтверждён."),
                enabled: capabilities.liveImage === true,
                level: "safe"
            },
            {
                title: I18n.t("OpenIPC network write"),
                text: I18n.t("Записывает сетевую конфигурацию OpenIPC, потенциально меняет доступность камеры."),
                guard: Object.keys(firmwareNetwork || {}).length > 0
                       ? I18n.t("Защита: отдельный confirm перед сохранением и reset через firmware API.")
                       : I18n.t("Сначала загрузите вкладку Сеть, чтобы видеть текущие значения."),
                enabled: Object.keys(firmwareNetwork || {}).length > 0,
                level: "warn"
            },
            {
                title: I18n.t("Firmware update from upload"),
                text: I18n.t("Устанавливает локальный archive через /ws/upgrade после upload в /tmp/firmware.tgz."),
                guard: uploadedBlockReason.length ? uploadedBlockReason : I18n.t("Защита: SoC/Flash, checksum, backup, питание и опасные опции."),
                enabled: uploadedBlockReason.length === 0,
                level: "danger"
            },
            {
                title: I18n.t("Firmware update from GitHub"),
                text: I18n.t("Запускает штатный updater камеры по данным update page."),
                guard: githubBlockReason.length ? githubBlockReason : I18n.t("Защита: update-info, WebSocket updater, backup и power-gate."),
                enabled: githubBlockReason.length === 0,
                level: "danger"
            },
            {
                title: I18n.t("Majestic restore diff"),
                text: I18n.t("Применяет backup как diff к текущей конфигурации, без удаления неизвестных полей."),
                guard: backupRestorePath.length
                       ? backupRestoreRiskSummary()
                       : I18n.t("Сначала откройте Majestic backup для preview."),
                enabled: backupRestorePath.length > 0 && backupRestoreChanges.length > 0 && capabilities.configWrite === true,
                level: backupRestoreCriticalCount() > 0 ? "warn" : "safe"
            }
        ]
    }

    function groupFieldCount(groupId) {
        var count = 0
        for (var i = 0; i < fields.length; ++i) {
            if (!groupId.length || fields[i].groupId === groupId) count++
        }
        return count
    }

    function selectedGroupLabel() {
        for (var i = 0; i < groups.length; ++i) {
            if (groups[i].id === selectedGroupId) return localizedGroupLabel(groups[i])
        }
        return selectedGroupId.length ? localizedGroupLabel(selectedGroupId) : I18n.t("Настройки")
    }

    function tabItems() {
        var rows = [
            { kind: "overview", id: "", label: I18n.t("Статус") },
            { kind: "settings", id: "", label: I18n.t("Majestic") },
            { kind: "firmware", id: "", label: I18n.t("Firmware") },
            { kind: "network", id: "", label: I18n.t("Сеть") },
            { kind: "time", id: "", label: I18n.t("Время") },
            { kind: "update", id: "", label: I18n.t("Update") },
            { kind: "tools", id: "", label: I18n.t("Tools") }
        ]
        rows.push({ kind: "endpoints", id: "", label: I18n.t("Эндпоинты") })
        rows.push({ kind: "raw", id: "", label: I18n.t("Raw JSON") })
        rows.push({ kind: "metrics", id: "", label: I18n.t("Метрики") })
        return rows
    }

    function currentTabItem() {
        var rows = tabItems()
        var idx = Math.max(0, Math.min(tabs.currentIndex, rows.length - 1))
        return rows[idx] || rows[0]
    }

    function currentContentIndex() {
        var kind = currentTabItem().kind
        if (kind === "settings") return 1
        if (kind === "firmware") return 2
        if (kind === "network") return 3
        if (kind === "time") return 4
        if (kind === "update") return 5
        if (kind === "tools") return 6
        if (kind === "endpoints") return 7
        if (kind === "raw") return 8
        if (kind === "metrics") return 9
        return 0
    }

    function tabIndexForKind(kind) {
        var rows = tabItems()
        for (var i = 0; i < rows.length; ++i) {
            if (rows[i].kind === kind) return i
        }
        return 0
    }

    function syncSelectedGroupFromTab() {
        revision++
    }

    function fieldRequiresPipelineReload(path) {
        for (var i = 0; i < fields.length; ++i) {
            if (fields[i].path === path) return fields[i].live !== true
        }
        return true
    }

    function isCriticalSettingPath(path) {
        var p = String(path || "")
        if (fieldRequiresPipelineReload(p)) return true
        return p.indexOf("video") === 0
                || p.indexOf("audio") === 0
                || p.indexOf("rtsp") === 0
                || p.indexOf("hls") === 0
                || p.indexOf("mjpeg") === 0
                || p.indexOf("webrtc") === 0
                || p.indexOf("outgoing") === 0
                || p.indexOf("records") === 0
                || p.indexOf("recording") === 0
                || p.indexOf("system") === 0
                || p.indexOf("netip") === 0
                || p === "image.rotate"
                || p === "image.tuning"
    }

    function criticalChanges(changes) {
        var out = []
        for (var i = 0; i < changes.length; ++i) {
            if (isCriticalSettingPath(changes[i].path)) out.push(changes[i])
        }
        return out
    }

    function rememberRollbackSnapshot(changes, reason) {
        rollbackConfig = clone(originalConfig)
        rollbackSchema = clone(currentSchema)
        rollbackChanges = criticalChanges(changes || [])
        rollbackCritical = rollbackChanges.length > 0
        rollbackAvailable = rollbackCritical
        rollbackReason = reason || (rollbackCritical
                                    ? I18n.t("Перед критичным изменением сохранён снимок конфигурации для быстрого отката.")
                                    : "")
        rollbackHealthState = rollbackCritical ? "ready" : "idle"
        rollbackHealthText = rollbackCritical
                             ? I18n.t("После apply будет проверен Majestic API и основной RTSP поток.")
                             : ""
        rollbackWatchActive = false
        rollbackWatchTries = 0
        rollbackAutoAllowed = false
    }

    function clearRollbackSnapshot() {
        rollbackConfig = ({})
        rollbackSchema = ({})
        rollbackChanges = []
        rollbackAvailable = false
        rollbackCritical = false
        rollbackReason = ""
        rollbackHealthState = "idle"
        rollbackHealthText = ""
        rollbackWatchActive = false
        rollbackWatchTries = 0
        rollbackAutoAllowed = false
        rollbackHealthWatchTimer.stop()
        activeRollbackId = ""
    }

    function startRollbackHealthWatch(reason) {
        if (!rollbackAvailable || !rollbackCritical) return
        rollbackWatchActive = true
        rollbackWatchTries = 0
        rollbackHealthState = "watching"
        rollbackHealthText = reason && String(reason).length
                             ? I18n.t(reason)
                             : I18n.t("Наблюдаю за восстановлением Majestic API и RTSP после apply…")
        rollbackHealthWatchTimer.restart()
    }

    function rollbackHealthSummary() {
        return "API: " + probeStateText(majesticApiProbeState, majesticApiProbeMessage, majesticApiProbeElapsedMs)
                + " · RTSP main: " + probeStateText(rtspMainProbeState, rtspMainProbeMessage, rtspMainProbeElapsedMs)
    }

    function maybeFinishRollbackHealthProbe() {
        if (!rollbackWatchActive) return
        if (majesticApiProbeState === "running" || rtspMainProbeState === "running") return
        if (majesticApiProbeState === "idle" || rtspMainProbeState === "idle") return
        var ok = majesticApiProbeState === "ok" && rtspMainProbeState === "ok"
        rollbackHealthText = rollbackHealthSummary()
        if (ok) {
            rollbackWatchActive = false
            rollbackHealthWatchTimer.stop()
            rollbackHealthState = "ok"
            rollbackReason = I18n.t("Health probe успешен. Rollback snapshot сохранён на случай ручного отката.")
            return
        }
        if (rollbackWatchTries >= rollbackWatchMaxTries) {
            rollbackWatchActive = false
            rollbackHealthWatchTimer.stop()
            rollbackHealthState = "fail"
            rollbackReason = I18n.t("После apply поток/API не восстановились. Можно откатить критичные изменения.")
            if (rollbackAutoAllowed && majesticApiProbeState === "ok") {
                rollbackReason = I18n.t("Авто-rollback разрешён: API доступен, выполняю откат критичных изменений.")
                rollbackPendingChanges()
            }
        }
    }

    function rollbackPendingChanges() {
        if (!rollbackAvailable) return
        var changes = SystemController.majesticClient.describeChanges(originalConfig, rollbackConfig)
        if (!changes.length) {
            statusError = false
            statusText = I18n.t("Откат не требуется: конфигурация уже совпадает со снимком.")
            clearRollbackSnapshot()
            return
        }
        pendingPatch = SystemController.majesticClient.buildPatch(originalConfig, rollbackConfig)
        pendingChanges = changes
        pendingPipelineReloadNeeded = false
        for (var i = 0; i < changes.length; ++i) {
            if (fieldRequiresPipelineReload(changes[i].path)) {
                pendingPipelineReloadNeeded = true
                break
            }
        }
        pendingAutoReloadAfterApply = pendingPipelineReloadNeeded && autoReloadAfterApply && capabilities.pipelineReload === true
        loading = true
        statusError = false
        statusText = I18n.t("Откат критичных настроек Majestic…")
        activeRollbackId = track(SystemController.majesticClient.applyConfiguration(
                                     cameraHost, cameraPort, cameraUser, cameraPassword, pendingPatch))
        activeApplyId = activeRollbackId
        applyWatchdogTimer.restart()
    }

    function clearBackupRestore() {
        backupRestoreConfig = ({})
        backupRestoreSchema = ({})
        backupRestoreChanges = []
        backupRestorePath = ""
    }

    function backupRestoreSummary() {
        if (!backupRestorePath.length) return I18n.t("Backup не выбран.")
        return I18n.t("Backup: %1 · отличий: %2", [backupRestorePath, backupRestoreChanges.length])
    }

    function backupRestoreReloadCount() {
        var count = 0
        for (var i = 0; i < backupRestoreChanges.length; ++i)
            if (fieldRequiresPipelineReload(backupRestoreChanges[i].path)) count++
        return count
    }

    function backupRestoreLiveCount() {
        return Math.max(0, backupRestoreChanges.length - backupRestoreReloadCount())
    }

    function backupRestoreCriticalCount() {
        return criticalChanges(backupRestoreChanges).length
    }

    function backupRestoreSecretCount() {
        var count = 0
        for (var i = 0; i < backupRestoreChanges.length; ++i) {
            var path = String(backupRestoreChanges[i].path || "")
            var tail = path.substring(path.lastIndexOf(".") + 1)
            if (isSensitiveKey(path) || isSensitiveKey(tail)) count++
        }
        return count
    }

    function backupRestoreRiskSummary() {
        if (!backupRestorePath.length) return I18n.t("Backup не выбран.")
        if (!backupRestoreChanges.length) return I18n.t("Backup совпадает с текущей конфигурацией.")
        var parts = [
            I18n.t("diff %1", [backupRestoreChanges.length]),
            I18n.t("reload %1", [backupRestoreReloadCount()]),
            I18n.t("critical %1", [backupRestoreCriticalCount()])
        ]
        if (backupRestoreSecretCount() > 0) parts.push(I18n.t("secret %1", [backupRestoreSecretCount()]))
        return parts.join(" · ")
    }

    function backupRestoreRiskRows() {
        return [
            { label: I18n.t("Всего"), value: String(backupRestoreChanges.length), state: backupRestoreChanges.length ? "warn" : "ok" },
            { label: I18n.t("Live"), value: String(backupRestoreLiveCount()), state: "ok" },
            { label: I18n.t("Reload"), value: String(backupRestoreReloadCount()), state: backupRestoreReloadCount() ? "warn" : "ok" },
            { label: I18n.t("Critical"), value: String(backupRestoreCriticalCount()), state: backupRestoreCriticalCount() ? "block" : "ok" },
            { label: I18n.t("Secrets"), value: String(backupRestoreSecretCount()), state: backupRestoreSecretCount() ? "warn" : "ok" }
        ]
    }

    function previewBackupRestore() {
        if (!backupRestorePath.length) return
        rawJsonPage.text = JSON.stringify(backupRestoreConfig, null, 2)
        tabs.currentIndex = tabIndexForKind("raw")
    }

    function applyBackupRestore() {
        if (!backupRestorePath.length) return
        if (!backupRestoreChanges.length) {
            statusError = false
            statusText = I18n.t("Backup совпадает с текущей конфигурацией.")
            return
        }
        prepareApply(backupRestoreConfig)
    }

    function selectGroup(groupId) {
        if (selectedGroupId === groupId) return
        selectedGroupId = groupId
        revision++
    }

    function visibleField(field) {
        revision
        if (!field || field.groupId !== selectedGroupId) return false
        var condition = field.visibleWhen || {}
        if (condition.field !== undefined) {
            var parent = field.path.substring(0, field.path.lastIndexOf("."))
            var controller = parent + "." + condition.field
            if (String(draftValues[controller]) !== String(condition.equals)) return false
        }
        var needle = settingsPage.searchText.trim().toLowerCase()
        return !needle.length || localizedFieldTitle(field).toLowerCase().indexOf(needle) >= 0
                || String(field.path).toLowerCase().indexOf(needle) >= 0
                || localizedFieldHint(field).toLowerCase().indexOf(needle) >= 0
                || String(field.description).toLowerCase().indexOf(needle) >= 0
                || String(field.hint).toLowerCase().indexOf(needle) >= 0
    }

    function liveFieldsForGroup(groupId) {
        revision
        var out = []
        for (var i = 0; i < fields.length; ++i) {
            var field = fields[i]
            if (field.groupId === groupId && field.live === true && visibleField(field)) out.push(field)
        }
        out.sort(function(a, b) { return liveFieldOrder(a.key) - liveFieldOrder(b.key) })
        return out
    }

    function liveFieldOrder(key) {
        var order = ["luminance", "contrast", "saturation", "hue", "mirror", "flip"]
        var idx = order.indexOf(key)
        return idx < 0 ? 99 : idx
    }

    function liveFieldLabel(field) {
        var map = {
            luminance: "☀  " + l10n({ ru: "Яркость", en: "Brightness" }),
            contrast: "◐  " + l10n({ ru: "Контраст", en: "Contrast" }),
            saturation: "💧  " + l10n({ ru: "Насыщенность", en: "Saturation" }),
            hue: "🌈  " + l10n({ ru: "Оттенок", en: "Hue" }),
            mirror: "↔  " + l10n({ ru: "Зеркало", en: "Mirror" }),
            flip: "↕  " + l10n({ ru: "Переворот", en: "Flip" })
        }
        return map[field.key] || localizedFieldTitle(field)
    }

    function sectionCardsForGroup(groupId) {
        revision
        var cards = []
        var bySection = {}
        for (var i = 0; i < fields.length; ++i) {
            var field = fields[i]
            if (field.groupId !== groupId || field.live === true || !visibleField(field)) continue
            var id = field.section || field.groupId
            if (!bySection[id]) {
                bySection[id] = {
                    id: id,
                    label: localizedSectionLabel(id, field.sectionLabel),
                    fields: []
                }
                cards.push(bySection[id])
            }
            bySection[id].fields.push(field)
        }
        return cards
    }

    function cardHeight(card) {
        var total = 74
        for (var i = 0; i < card.fields.length; ++i) {
            total += card.fields[i].type === "array" ? 126 : 96
        }
        return Math.max(150, total)
    }

    function arrayValue(field) {
        revision
        var value = draftValues[field.path]
        if (value === undefined || value === null) return []
        if (value instanceof Array) return value
        if (value.join) return value
        return String(value).split(",").map(function(v) { return v.trim() }).filter(function(v) { return v.length })
    }

    function updateArrayValue(field, index, value) {
        var values = arrayValue(field).slice()
        values[index] = value
        updateDraft(field, values.filter(function(v) { return String(v).trim().length > 0 }))
    }

    function addArrayValue(field) {
        var values = arrayValue(field).slice()
        values.push("")
        updateDraft(field, values)
    }

    function removeArrayValue(field, index) {
        var values = arrayValue(field).slice()
        values.splice(index, 1)
        updateDraft(field, values)
    }

    function isRangeField(field) {
        return (field.type === "integer" || field.type === "number")
                && field.minimum !== undefined && field.maximum !== undefined
                && Number(field.maximum) <= 100
    }

    function isResolutionField(field) {
        return field.type === "string"
                && (field.resolution === true || field.path === "video0.size"
                    || field.path === "video1.size" || field.path === "jpeg.size")
    }

    function resolutionValues(field) {
        var current = valueText(field, draftValues[field.path])
        var presets = ["3840x2160", "2592x1944", "2560x1440", "2304x1296", "1920x1080",
                       "1600x1200", "1280x960", "1280x720", "1024x768", "704x576",
                       "704x480", "640x480", "640x360", "352x288", "320x240"]
        var out = []
        for (var i = 0; i < presets.length; ++i) out.push(presets[i])
        if (current.length && out.indexOf(current) < 0) out.unshift(current)
        return out
    }

    function resolutionIndex(field) {
        var model = resolutionValues(field)
        var current = valueText(field, draftValues[field.path])
        for (var i = 0; i < model.length; ++i) if (model[i] === current) return i
        return -1
    }

    function configValue(path, fallback) {
        var parts = path.split(".")
        var value = originalConfig
        for (var i = 0; i < parts.length; ++i) {
            if (value === undefined || value === null || value[parts[i]] === undefined) return fallback
            value = value[parts[i]]
        }
        return value
    }

    function majesticRtspPort() {
        var port = Number(configValue("rtsp.port", 554))
        return isNaN(port) || port <= 0 ? 554 : Math.round(port)
    }

    function probeStateColor(state) {
        if (state === "ok") return Theme.success
        if (state === "fail") return Theme.danger
        if (state === "running") return Theme.warning
        return Theme.textFaint
    }

    function probeStateText(state, message, elapsedMs) {
        if (state === "running") return I18n.t("Проверка…")
        if (state === "idle") return I18n.t("Не проверено")
        var text = I18n.t(message || (state === "ok" ? "Готово" : "Ошибка"))
        if (elapsedMs > 0) text += " · " + elapsedMs + " " + I18n.t("мс")
        return text
    }

    function startEndpointProbe(slot) {
        if (!cameraHost.length) return
        if (slot === "main") {
            rtspMainProbeState = "running"
            rtspMainProbeMessage = I18n.t("Проверка RTSP…")
            rtspMainProbeElapsedMs = 0
            rtspMainProbeId = SystemController.probeCameraEndpoint(
                        "rtsp", cameraHost, majesticRtspPort(), "/stream=0", cameraUser, cameraPassword)
        } else if (slot === "sub") {
            rtspSubProbeState = "running"
            rtspSubProbeMessage = I18n.t("Проверка RTSP…")
            rtspSubProbeElapsedMs = 0
            rtspSubProbeId = SystemController.probeCameraEndpoint(
                        "rtsp", cameraHost, majesticRtspPort(), "/stream=1", cameraUser, cameraPassword)
        } else if (slot === "api") {
            majesticApiProbeState = "running"
            majesticApiProbeMessage = I18n.t("Проверка Majestic API…")
            majesticApiProbeElapsedMs = 0
            majesticApiProbeId = SystemController.probeCameraEndpoint(
                        "majestic", cameraHost, cameraPort, "/api/v1/config.json", cameraUser, cameraPassword)
        }
    }

    function resetEndpointProbes() {
        rtspMainProbeId = ""
        rtspSubProbeId = ""
        majesticApiProbeId = ""
        rtspMainProbeState = "idle"
        rtspSubProbeState = "idle"
        majesticApiProbeState = "idle"
        rtspMainProbeMessage = ""
        rtspSubProbeMessage = ""
        majesticApiProbeMessage = ""
        rtspMainProbeElapsedMs = 0
        rtspSubProbeElapsedMs = 0
        majesticApiProbeElapsedMs = 0
    }

    function streamRows() {
        var rows = []
        for (var i = 0; i < 2; ++i) {
            var prefix = "video" + i
            if (configValue(prefix + ".enabled", false) !== true) continue
            rows.push({
                name: i === 0 ? I18n.t("Main") : I18n.t("Sub"),
                size: configValue(prefix + ".size", "?"),
                codec: String(configValue(prefix + ".codec", "?")).toUpperCase(),
                fps: configValue(prefix + ".fps", ""),
                bitrate: configValue(prefix + ".bitrate", "")
            })
        }
        if (configValue("jpeg.enabled", false) === true)
            rows.push({ name: "JPEG", size: I18n.t("снимки включены"), codec: "", fps: "", bitrate: "" })
        return rows
    }

    function metric(key, fallback) {
        if (metricsData && metricsData[key] !== undefined) return metricsData[key]
        return fallback
    }

    function ramPercent() {
        var total = Number(metric("node_memory_MemTotal_bytes", 0))
        var available = Number(metric("node_memory_MemAvailable_bytes", 0))
        if (total <= 0) return 0
        return Math.round((1 - available / total) * 100)
    }

    function ramText() {
        var total = Number(metric("node_memory_MemTotal_bytes", 0))
        var available = Number(metric("node_memory_MemAvailable_bytes", 0))
        if (total <= 0) return "—"
        return Math.round((total - available) / 1048576) + " / " + Math.round(total / 1048576) + " MB"
    }

    function uptimeText() {
        var now = Number(metric("node_time_seconds", 0))
        var boot = Number(metric("node_boot_time_seconds", 0))
        if (now <= 0 || boot <= 0 || now <= boot) return "—"
        var seconds = Math.max(0, Math.floor(now - boot))
        var days = Math.floor(seconds / 86400)
        var daysRemainder = seconds - Math.floor(seconds / 86400) * 86400
        var hoursRemainder = seconds - Math.floor(seconds / 3600) * 3600
        var hours = Math.floor(daysRemainder / 3600)
        var minutes = Math.floor(hoursRemainder / 60)
        return (days ? days + "d " : "") + (hours || days ? hours + "h " : "") + minutes + "m"
    }

    function tempText() {
        var temp = metric("node_hwmon_temp_celsius", "")
        return temp === "" ? "—" : Math.round(Number(temp)) + " °C"
    }

    function webUiHost() {
        var host = cameraHost.length ? cameraHost : "camera"
        return host + (cameraPort && cameraPort !== 80 ? ":" + cameraPort : "")
    }

    function webUiUrl(path) {
        var suffix = path || ""
        if (suffix.length && suffix.charAt(0) !== "/") suffix = "/" + suffix
        return "http://" + webUiHost() + suffix
    }

    function openWebUiPath(path) {
        Qt.openUrlExternally(webUiUrl(path || ""))
    }

    function copyControlCenterValue(label, value) {
        var text = String(value || "")
        if (!text.length) return
        SystemController.copyTextToClipboard(text)
        statusError = false
        statusText = I18n.t("Скопировано: %1", [label || I18n.t("значение")])
    }

    function cameraTimeText() {
        var seconds = Number(metric("node_time_seconds", 0))
        if (seconds <= 0 || isNaN(seconds)) return "—"
        return Qt.formatDateTime(new Date(seconds * 1000), "yyyy-MM-dd HH:mm:ss")
    }

    function firmwareIdentityRows() {
        return [
            { label: I18n.t("Web UI камеры"), value: webUiUrl(""), hint: I18n.t("Основной веб-интерфейс OpenIPC/FancyWeb на камере.") },
            { label: I18n.t("Пользователь"), value: cameraUser.length ? cameraUser : "root", hint: I18n.t("OpenIPC обычно использует одну пару логин/пароль для Web UI и SSH.") },
            { label: I18n.t("Majestic API"), value: capabilities.schema === true ? I18n.t("schema загружена") : I18n.t("legacy / read-only"), hint: I18n.t("Медиа-сервер Majestic отдаёт конфигурацию, schema, endpoints и metrics.") },
            { label: I18n.t("Доступные настройки"), value: String(fields.length), hint: I18n.t("Поля получены напрямую с камеры, поэтому список зависит от прошивки, SoC и сенсора.") }
        ]
    }

    function firmwareSystemRows() {
        var pulse = firmwareStatus.pulse || {}
        var statusPage = firmwareStatus.statusPage || {}
        var statusDevice = statusPage.device || {}
        return [
            { title: "LOAD", value: String(metric("node_load1", "—")), subtitle: I18n.t("Средняя нагрузка 1 мин"), percent: Math.min(100, Number(metric("node_load1", 0)) * 35), accent: Theme.accent },
            { title: I18n.t("Память"), value: pulse.mem_used !== undefined ? pulse.mem_used + "%" : ramPercent() + "%", subtitle: ramText(), percent: Number(pulse.mem_used !== undefined ? pulse.mem_used : ramPercent()), accent: Theme.accent },
            { title: I18n.t("Температура"), value: pulse.soc_temp || tempText(), subtitle: statusDevice.soc || I18n.t("Температура SoC"), percent: Math.min(100, Number(metric("node_hwmon_temp_celsius", 0)) / 90 * 100), accent: Theme.metroOrange },
            { title: "UPTIME", value: pulse.uptime || uptimeText(), subtitle: pulse.mj_uptime ? ("Majestic " + pulse.mj_uptime) : I18n.t("Время работы Linux"), percent: 100, accent: Theme.success }
        ]
    }

    function networkServiceRows() {
        var hostOnly = webUiHost().split(":")[0]
        var current = firmwareNetwork.current || {}
        return [
            { label: I18n.t("Хост"), value: firmwareNetwork.hostname || current.hostname || hostOnly, hint: I18n.t("Имя устройства из OpenIPC WebUI.") },
            { label: I18n.t("IP"), value: firmwareNetwork.address || current.address || hostOnly, hint: I18n.t("Текущий IP-адрес выбранного интерфейса.") },
            { label: I18n.t("Интерфейс"), value: firmwareNetwork.interface || current.interface || "—", hint: I18n.t("Активный сетевой интерфейс камеры.") },
            { label: I18n.t("Шлюз"), value: firmwareNetwork.gateway || current.gateway || "—", hint: I18n.t("Default gateway камеры.") },
            { label: "DNS", value: firmwareNetwork.nameserver || current.nameserver || "—", hint: I18n.t("DNS сервер камеры.") },
            { label: "MAC", value: firmwareNetwork.macAddress || current.macAddress || "—", hint: I18n.t("MAC-адрес интерфейса.") },
            { label: "HTTP", value: webUiUrl(""), hint: I18n.t("Web UI и Majestic API. Порт берётся из карточки камеры.") },
            { label: "RTSP", value: "rtsp://" + hostOnly + ":" + majesticRtspPort(), hint: I18n.t("Видеопотоки /stream=0 и /stream=1.") },
            { label: "HLS", value: configValue("hls.enabled", false) === true ? I18n.t("включён") : I18n.t("выключен"), hint: webUiUrl("/hls") },
            { label: "MJPEG", value: configValue("mjpeg.enabled", false) === true ? I18n.t("включён") : I18n.t("выключен"), hint: webUiUrl("/mjpeg") },
            { label: "WebRTC", value: configValue("webrtc.enabled", false) === true ? I18n.t("включён") : I18n.t("выключен"), hint: webUiUrl("/webrtc") },
            { label: "ONVIF", value: configValue("onvif.enabled", false) === true ? I18n.t("включён") : I18n.t("по данным Majestic не включён"), hint: I18n.t("ONVIF управляется прошивкой/сервисом и может отличаться от RTSP.") }
        ]
    }

    function timeRows() {
        var pulse = firmwareStatus.pulse || {}
        return [
            { label: I18n.t("Время камеры"), value: pulse.time_now ? Qt.formatDateTime(new Date(Number(pulse.time_now) * 1000), "yyyy-MM-dd HH:mm:ss") : cameraTimeText(), hint: I18n.t("Берётся из /cgi-bin/j/pulse.cgi и metrics.") },
            { label: I18n.t("Время Dashboard"), value: Qt.formatDateTime(new Date(), "yyyy-MM-dd HH:mm:ss"), hint: I18n.t("Локальное время компьютера для сравнения с камерой.") },
            { label: I18n.t("Часовой пояс"), value: firmwareTime.zoneName || firmwareTime.currentZoneName || pulse.timezone || "—", hint: firmwareTime.zoneData || firmwareTime.currentZoneData || I18n.t("POSIX timezone string камеры.") },
            { label: "NTP", value: firmwareTime.ntpSummary || I18n.t("не загружено"), hint: I18n.t("Список NTP серверов из /etc/ntp.conf.") }
        ]
    }

    function updateChecklistRows() {
        var sourceOk = firmwareUpdateInfo.githubAvailable === true || firmwareArchiveUploaded
        var identityOk = firmwareSocText().length > 0 && firmwareFlashText().length > 0
        var optionsOk = firmwareUpdateKernel || firmwareUpdateRootfs
        var dangerousOptions = firmwareUpdateReset || firmwareUpdateForce
        var uploadedArchiveOk = !firmwareArchiveUploaded
                || (firmwareArchiveSizeBytes > 0 && firmwareArchiveSizeBytes <= 128 * 1024 * 1024)
        var archiveCompatibility = firmwareArchiveCompatibility()
        var safetyOk = firmwarePowerSafetyConfirmed
                && (!dangerousOptions || firmwareDangerOptionsConfirmed)
        return [
            {
                title: I18n.t("1. WebSocket updater"),
                text: SystemController.firmwareClient.webSocketsAvailable
                      ? I18n.t("Native /ws/upgrade доступен: приложение может вести update и читать прогресс.")
                      : I18n.t("Qt WebSockets не найден в этой сборке: используйте штатный Update WebUI камеры."),
                state: SystemController.firmwareClient.webSocketsAvailable ? "ok" : "block"
            },
            {
                title: I18n.t("2. Совместимость"),
                text: identityOk
                      ? (firmwareArchiveUploaded
                         ? archiveCompatibility.text
                         : I18n.t("Определено: SoC %1 · Flash %2 · Variant %3", [firmwareSocText(), firmwareFlashText(), firmwareVariantText() || "—"]))
                      : I18n.t("Сначала загрузите update-info, чтобы приложение видело SoC и тип flash."),
                state: identityOk ? (firmwareArchiveUploaded ? archiveCompatibility.state : "ok") : "block"
            },
            {
                title: I18n.t("3. Источник прошивки"),
                text: sourceOk
                      ? (firmwareArchiveUploaded
                         ? I18n.t("Загружен локальный archive: %1 · %2", [firmwareArchiveName || "/tmp/firmware.tgz", firmwareArchiveSizeText()])
                         : I18n.t("Доступен GitHub update по данным update page камеры."))
                      : I18n.t("Нет готового источника: загрузите update-info для GitHub или upload archive."),
                state: sourceOk && uploadedArchiveOk ? "ok" : (sourceOk ? "warn" : "block")
            },
            {
                title: I18n.t("4. Checksum / подпись"),
                text: firmwareArchiveName.length
                      ? firmwareArchiveManifestSummary()
                      : firmwareUpdateChecksumText(),
                state: firmwareArchiveName.length
                       ? firmwareArchiveManifestState()
                       : ((firmwareUpdateInfo.sha256 || firmwareUpdateInfo.checksum
                           || firmwareUpdateInfo.digest || firmwareUpdateInfo.signature) ? "ok" : "warn")
            },
            {
                title: I18n.t("5. Backup"),
                text: firmwareBackupSaved
                      ? I18n.t("Firmware backup уже сохранён в этой сессии.")
                      : I18n.t("Перед прошивкой рекомендуется сохранить firmware backup и Majestic backup."),
                state: firmwareBackupSaved ? "ok" : "warn"
            },
            {
                title: I18n.t("6. Опции прошивки"),
                text: I18n.t("Выбрано: %1", [firmwareUpdateOptionsSummary()]),
                state: !optionsOk ? "block" : (dangerousOptions ? "warn" : "ok")
            },
            {
                title: I18n.t("7. Питание и сеть"),
                text: firmwarePowerSafetyConfirmed
                      ? I18n.t("Пользователь подтвердил стабильное питание/сеть. После flashing/reboot приложение будет ждать возврата камеры.")
                      : I18n.t("Подтвердите стабильное питание и сеть перед стартом update."),
                state: safetyOk ? "ok" : "block"
            }
        ]
    }

    function toolRows() {
        return [
            { title: I18n.t("Web console"), text: I18n.t("Открывает штатную консоль WebUI камеры, если она доступна в этой сборке."), path: "/cgi-bin/tool-console.cgi", external: false },
            { title: I18n.t("File browser"), text: I18n.t("Штатный файловый менеджер WebUI камеры для диагностики и ручного обслуживания."), path: "/cgi-bin/tool-files.cgi", external: false },
            { title: I18n.t("Firmware settings"), text: I18n.t("Раздел системных настроек OpenIPC WebUI."), path: "/cgi-bin/fw-settings.cgi", external: false },
            { title: I18n.t("FancyWeb-NG"), text: I18n.t("Перспективный интерфейс OpenIPC: используем как UX-ориентир и справочник."), path: "https://github.com/OpenIPC/fancyweb-ng", external: true }
        ]
    }

    function metricsCount() {
        return metricsData ? Object.keys(metricsData).length : 0
    }

    function metricKeyList() {
        var keys = []
        if (!metricsData) return keys
        for (var key in metricsData) keys.push(key)
        keys.sort()
        return keys
    }

    function metricKeyMatchesWords(key, words) {
        var text = String(key || "").toLowerCase()
        for (var i = 0; i < words.length; ++i) {
            if (text.indexOf(String(words[i]).toLowerCase()) < 0) return false
        }
        return true
    }

    function firstMetricByWords(words, fallback) {
        var keys = metricKeyList()
        for (var i = 0; i < keys.length; ++i) {
            if (metricKeyMatchesWords(keys[i], words)) return metricsData[keys[i]]
        }
        return fallback
    }

    function sumMetricsByWords(words) {
        var keys = metricKeyList()
        var sum = 0
        var found = false
        for (var i = 0; i < keys.length; ++i) {
            if (!metricKeyMatchesWords(keys[i], words)) continue
            var value = Number(metricsData[keys[i]])
            if (isNaN(value)) continue
            sum += value
            found = true
        }
        return found ? sum : null
    }

    function metricNumberText(value, decimals, suffix) {
        var n = Number(value)
        if (value === undefined || value === null || value === "" || isNaN(n)) return "—"
        return n.toFixed(decimals) + (suffix || "")
    }

    function metricSumText(words) {
        var value = sumMetricsByWords(words)
        return value === null ? "—" : String(Math.round(value))
    }

    function metricClientPercent(words) {
        var value = sumMetricsByWords(words)
        return value === null ? 0 : Math.min(100, value * 20)
    }

    function metricsOverviewRows() {
        var count = metricsCount()
        return [
            { title: I18n.t("Метрик"), value: String(count), subtitle: metricsUpdatedAt.length ? I18n.t("Обновлено: %1", [metricsUpdatedAt]) : I18n.t("Нет данных"), percent: count > 0 ? 100 : 0, accent: Theme.accent },
            { title: "LOAD", value: String(metric("node_load1", "—")), subtitle: I18n.t("Средняя нагрузка 1 мин"), percent: Math.min(100, Number(metric("node_load1", 0)) * 35), accent: Theme.accent },
            { title: I18n.t("Память"), value: ramPercent() + "%", subtitle: ramText(), percent: ramPercent(), accent: Theme.accent },
            { title: I18n.t("Температура"), value: tempText(), subtitle: I18n.t("Температура SoC"), percent: Math.min(100, Number(metric("node_hwmon_temp_celsius", 0)) / 90 * 100), accent: Theme.metroOrange },
            { title: "RTSP", value: metricSumText(["rtsp", "clients"]), subtitle: I18n.t("Клиенты RTSP"), percent: metricClientPercent(["rtsp", "clients"]), accent: Theme.success },
            { title: "HLS", value: metricSumText(["hls", "clients"]), subtitle: I18n.t("Клиенты HLS"), percent: metricClientPercent(["hls", "clients"]), accent: Theme.success },
            { title: "WebRTC", value: metricSumText(["webrtc", "clients"]), subtitle: I18n.t("Клиенты WebRTC"), percent: metricClientPercent(["webrtc", "clients"]), accent: Theme.success },
            { title: "FPS", value: metricNumberText(firstMetricByWords(["fps"], null), 1, ""), subtitle: I18n.t("Первый найденный FPS encoder/stream"), percent: Math.min(100, Number(firstMetricByWords(["fps"], 0)) * 4), accent: Theme.warning }
        ]
    }

    function metricsHealthRows() {
        var rows = []
        if (metricsCount() === 0) {
            rows.push({
                level: "idle",
                title: I18n.t("Метрики ещё не загружены"),
                text: I18n.t("Нажмите «Обновить метрики», чтобы получить состояние encoder, runtime и сетевых клиентов."),
                color: Theme.textFaint
            })
            return rows
        }

        var load = Number(metric("node_load1", 0))
        var memory = ramPercent()
        var temp = Number(metric("node_hwmon_temp_celsius", 0))
        if (load >= 2.5) {
            rows.push({
                level: "warn",
                title: I18n.t("Высокая нагрузка CPU"),
                text: I18n.t("Средняя нагрузка 1 мин: %1. Проверьте FPS, bitrate, codec и число клиентов.", [load.toFixed(2)]),
                color: Theme.warning
            })
        }
        if (memory >= 85) {
            rows.push({
                level: "warn",
                title: I18n.t("Память почти заполнена"),
                text: I18n.t("Использовано %1% RAM. Уменьшите число потоков/клиентов или проверьте утечки.", [memory]),
                color: Theme.warning
            })
        }
        if (!isNaN(temp) && temp >= 75) {
            rows.push({
                level: "warn",
                title: I18n.t("Температура высокая"),
                text: I18n.t("SoC: %1. Проверьте охлаждение и нагрузку encoder.", [tempText()]),
                color: Theme.metroOrange
            })
        }
        if (rows.length === 0) {
            rows.push({
                level: "ok",
                title: I18n.t("Критичных предупреждений нет"),
                text: I18n.t("Majestic metrics выглядят штатно по доступным показателям."),
                color: Theme.success
            })
        }
        return rows
    }

    function filteredMetricsText() {
        if (!metricsText.length) return I18n.t("Нажмите «Обновить метрики»")
        var query = metricsFilterText.trim().toLowerCase()
        if (!query.length) return metricsText
        var lines = metricsText.split("\n")
        var result = []
        for (var i = 0; i < lines.length; ++i) {
            if (lines[i].toLowerCase().indexOf(query) >= 0) result.push(lines[i])
        }
        return result.length ? result.join("\n") : I18n.t("Нет совпадений по фильтру")
    }

    function isSensitiveKey(key) {
        var k = String(key || "").toLowerCase()
        return k.indexOf("password") >= 0
                || k.indexOf("passwd") >= 0
                || k === "pass"
                || k.indexOf("token") >= 0
                || k.indexOf("secret") >= 0
                || k.indexOf("apikey") >= 0
                || k.indexOf("api_key") >= 0
                || k.indexOf("key") >= 0 && (k.indexOf("stream") >= 0 || k.indexOf("rtmp") >= 0 || k.indexOf("auth") >= 0)
    }

    function redactedClone(value, key) {
        if (isSensitiveKey(key)) return "********"
        if (value === null || value === undefined) return value
        if (Array.isArray(value)) {
            var arrayCopy = []
            for (var i = 0; i < value.length; ++i) arrayCopy.push(redactedClone(value[i], key))
            return arrayCopy
        }
        if (typeof value === "object") {
            var objectCopy = {}
            var keys = Object.keys(value)
            for (var j = 0; j < keys.length; ++j) objectCopy[keys[j]] = redactedClone(value[keys[j]], keys[j])
            return objectCopy
        }
        return value
    }

    function copyRedactedJson(value, label) {
        SystemController.copyTextToClipboard(JSON.stringify(redactedClone(value, ""), null, 2))
        statusError = false
        statusText = I18n.t("Скопировано без секретов: %1", [label || "Majestic"])
    }

    function copyRedactedRawJson() {
        var parsed = SystemController.majesticClient.parseJsonObject(rawJsonPage.text)
        if (!parsed.ok) {
            statusError = true
            statusText = I18n.t("Ошибка JSON: ") + parsed.error
            return
        }
        copyRedactedJson(parsed.value, I18n.t("Raw JSON"))
    }

    function copyEndpoint(row) {
        var value = row.copyValue || row.openValue || row.value || ""
        if (!value.length) return
        SystemController.copyTextToClipboard(value)
        statusError = false
        statusText = I18n.t("Скопировано: %1", [row.name || I18n.t("Эндпоинт")])
    }

    function openEndpoint(row) {
        var value = row.openValue || ""
        if (!value.length) return
        Qt.openUrlExternally(value)
    }

    function endpointProbeState(row) {
        var name = String(row.name || "").toLowerCase()
        if (name.indexOf("основной rtsp") >= 0) return rtspMainProbeState
        if (name.indexOf("дополнительный rtsp") >= 0) return rtspSubProbeState
        if (name.indexOf("config.json") >= 0 || name.indexOf("schema") >= 0) return majesticApiProbeState
        return ""
    }

    function endpointCapabilityState(row) {
        var name = String(row.name || "").toLowerCase()
        var value = String(row.value || "").toLowerCase()
        if (name.indexOf("write config") >= 0) return capabilities.configWrite === true ? "ok" : "block"
        if (name.indexOf("reset default") >= 0) return capabilities.resetDefaults === true ? "ok" : "block"
        if (name.indexOf("live image") >= 0) return capabilities.liveImage === true ? "ok" : "block"
        if (name.indexOf("play_audio") >= 0) return capabilities.playAudio === true ? "ok" : "block"
        if (name.indexOf("metrics") >= 0) return capabilities.metrics === true ? "ok" : "warn"
        if (name.indexOf("reload pipeline") >= 0) return capabilities.pipelineReload === true ? "ok" : "warn"
        if (value.indexOf("/ws/") >= 0) return SystemController.firmwareClient.webSocketsAvailable ? "ok" : "warn"
        if (name === "hls") return configValue("hls.enabled", false) === true ? "ok" : "warn"
        if (name === "mjpeg") return configValue("mjpeg.enabled", false) === true ? "ok" : "warn"
        if (name.indexOf("webrtc") >= 0) return configValue("webrtc.enabled", false) === true ? "ok" : "warn"
        return "info"
    }

    function endpointState(row) {
        var probe = endpointProbeState(row)
        if (probe === "ok") return "ok"
        if (probe === "fail") return "warn"
        if (probe === "running") return "warn"
        return endpointCapabilityState(row)
    }

    function endpointRisk(row) {
        var value = String(row.value || "").toLowerCase()
        var name = String(row.name || "").toLowerCase()
        if (value.indexOf("/ws/upgrade") >= 0 || value.indexOf("upload firmware") >= 0) return "danger"
        if (name.indexOf("write config") >= 0 || name.indexOf("reset default") >= 0
                || name.indexOf("reload pipeline") >= 0 || name.indexOf("night") >= 0
                || name.indexOf("ir-cut") >= 0 || name.indexOf("подсвет") >= 0
                || name.indexOf("play_audio") >= 0) return "warn"
        return "safe"
    }

    function endpointStatusText(row) {
        var probe = endpointProbeState(row)
        if (probe.length) return probeStateText(probe, "", 0)
        var state = endpointState(row)
        if (state === "ok") return I18n.t("подтверждено")
        if (state === "block") return I18n.t("capability закрыта")
        if (state === "warn") return I18n.t("проверьте доступность")
        return I18n.t("справочно")
    }

    function decorateEndpointRows(rows) {
        var out = []
        for (var i = 0; i < rows.length; ++i) {
            var row = rows[i]
            row.state = endpointState(row)
            row.risk = endpointRisk(row)
            row.statusText = endpointStatusText(row)
            out.push(row)
        }
        return out
    }

    function endpointRows() {
        var host = cameraHost.length ? cameraHost : "camera"
        var rtspPort = majesticRtspPort()
        var rtspHost = host + (rtspPort && rtspPort !== 554 ? ":" + rtspPort : "")
        var httpHost = host + (cameraPort && cameraPort !== 80 ? ":" + cameraPort : "")
        var displayAuth = cameraUser.length ? encodeURIComponent(cameraUser) + (cameraPassword.length ? ":••••" : "") + "@" : ""
        var realAuth = cameraUser.length ? encodeURIComponent(cameraUser) + (cameraPassword.length ? ":" + encodeURIComponent(cameraPassword) : "") + "@" : ""
        var rows = [
            { group: I18n.t("Видео"), name: I18n.t("Основной RTSP"), value: "rtsp://" + displayAuth + rtspHost + "/stream=0", copyValue: "rtsp://" + realAuth + rtspHost + "/stream=0", openValue: "rtsp://" + realAuth + rtspHost + "/stream=0", openable: true, hint: I18n.t("Главный поток Majestic") },
            { group: I18n.t("Видео"), name: I18n.t("Дополнительный RTSP"), value: "rtsp://" + displayAuth + rtspHost + "/stream=1", copyValue: "rtsp://" + realAuth + rtspHost + "/stream=1", openValue: "rtsp://" + realAuth + rtspHost + "/stream=1", openable: true, hint: I18n.t("Sub-stream, если включён в конфигурации") },
            { group: I18n.t("Видео"), name: "RTSP JPEG", value: "rtsp://" + displayAuth + rtspHost + "/stream=2", copyValue: "rtsp://" + realAuth + rtspHost + "/stream=2", openValue: "rtsp://" + realAuth + rtspHost + "/stream=2", openable: true, hint: I18n.t("JPEG-поток Majestic, если включён в прошивке") },
            { group: I18n.t("Видео"), name: I18n.t("WebSocket preview main"), value: "ws://" + httpHost + "/ws/video?stream=0", copyValue: "ws://" + httpHost + "/ws/video?stream=0", openable: false, hint: I18n.t("Низкая задержка, как в majestic-webui") },
            { group: I18n.t("Видео"), name: I18n.t("WebSocket preview sub"), value: "ws://" + httpHost + "/ws/video?stream=1", copyValue: "ws://" + httpHost + "/ws/video?stream=1", openable: false, hint: I18n.t("Второй поток предпросмотра") },
            { group: I18n.t("Видео"), name: "MJPEG", value: "http://" + httpHost + "/mjpeg", copyValue: "http://" + httpHost + "/mjpeg", openValue: "http://" + httpHost + "/mjpeg", openable: true, hint: I18n.t("MJPEG live stream в браузере или совместимом клиенте") },
            { group: I18n.t("Видео"), name: "MP4", value: "http://" + httpHost + "/video.mp4", copyValue: "http://" + httpHost + "/video.mp4", openValue: "http://" + httpHost + "/video.mp4", openable: true, hint: I18n.t("MP4 video stream Majestic") },
            { group: I18n.t("Видео"), name: "HLS", value: "http://" + httpHost + "/hls", copyValue: "http://" + httpHost + "/hls", openValue: "http://" + httpHost + "/hls", openable: true, hint: I18n.t("HLS live-streaming для браузера") },
            { group: I18n.t("Аудио"), name: "Opus", value: "http://" + httpHost + "/audio.opus", copyValue: "http://" + httpHost + "/audio.opus", openValue: "http://" + httpHost + "/audio.opus", openable: true, hint: I18n.t("Opus audio stream") },
            { group: I18n.t("Аудио"), name: "AAC", value: "http://" + httpHost + "/audio.m4a", copyValue: "http://" + httpHost + "/audio.m4a", openValue: "http://" + httpHost + "/audio.m4a", openable: true, hint: I18n.t("AAC audio stream") },
            { group: I18n.t("Аудио"), name: "PCM", value: "http://" + httpHost + "/audio.pcm", copyValue: "http://" + httpHost + "/audio.pcm", openValue: "http://" + httpHost + "/audio.pcm", openable: true, hint: I18n.t("Raw PCM audio stream") },
            { group: I18n.t("Снимки"), name: "JPEG", value: "http://" + httpHost + "/image.jpg", copyValue: "http://" + httpHost + "/image.jpg", openValue: "http://" + httpHost + "/image.jpg", openable: true, hint: I18n.t("Быстрый снимок текущего кадра") },
            { group: I18n.t("Снимки"), name: "HEIF", value: "http://" + httpHost + "/image.heif", copyValue: "http://" + httpHost + "/image.heif", openValue: "http://" + httpHost + "/image.heif", openable: true, hint: I18n.t("Снимок HEIF, если поддерживается прошивкой") },
            { group: I18n.t("Снимки"), name: "YUV420", value: "http://" + httpHost + "/image.yuv420", copyValue: "http://" + httpHost + "/image.yuv420", openValue: "http://" + httpHost + "/image.yuv420", openable: true, hint: I18n.t("Сырой кадр YUV420, если поддерживается прошивкой") },
            { group: I18n.t("День/ночь"), name: "night on", value: "http://" + httpHost + "/night/on", copyValue: "http://" + httpHost + "/night/on", openValue: "http://" + httpHost + "/night/on", openable: true, hint: I18n.t("Включить ночной режим") },
            { group: I18n.t("День/ночь"), name: "night off", value: "http://" + httpHost + "/night/off", copyValue: "http://" + httpHost + "/night/off", openValue: "http://" + httpHost + "/night/off", openable: true, hint: I18n.t("Выключить ночной режим") },
            { group: I18n.t("День/ночь"), name: "night toggle", value: "http://" + httpHost + "/night/toggle", copyValue: "http://" + httpHost + "/night/toggle", openValue: "http://" + httpHost + "/night/toggle", openable: true, hint: I18n.t("Переключить день/ночь") },
            { group: I18n.t("День/ночь"), name: "IR-cut", value: "http://" + httpHost + "/night/ircut", copyValue: "http://" + httpHost + "/night/ircut", openValue: "http://" + httpHost + "/night/ircut", openable: true, hint: I18n.t("Переключить механический IR-cut") },
            { group: I18n.t("День/ночь"), name: I18n.t("ИК-подсветка"), value: "http://" + httpHost + "/night/light", copyValue: "http://" + httpHost + "/night/light", openValue: "http://" + httpHost + "/night/light", openable: true, hint: I18n.t("Переключить ИК-подсветку") },
            { group: I18n.t("API"), name: "config.json", value: "http://" + httpHost + "/api/v1/config.json", copyValue: "http://" + httpHost + "/api/v1/config.json", openValue: "http://" + httpHost + "/api/v1/config.json", openable: true, hint: I18n.t("Текущая конфигурация Majestic") },
            { group: I18n.t("API"), name: "schema", value: "http://" + httpHost + "/api/v1/config.schema.json", copyValue: "http://" + httpHost + "/api/v1/config.schema.json", openValue: "http://" + httpHost + "/api/v1/config.schema.json", openable: true, hint: I18n.t("Описание доступных настроек") },
            { group: I18n.t("API"), name: "write config", value: "POST http://" + httpHost + "/api/v1/config", copyValue: "http://" + httpHost + "/api/v1/config", openable: false, hint: I18n.t("Schema-safe запись diff настроек") },
            { group: I18n.t("API"), name: "reset default", value: "http://" + httpHost + "/api/v1/reset?key=image.contrast", copyValue: "http://" + httpHost + "/api/v1/reset?key=image.contrast", openValue: "http://" + httpHost + "/api/v1/reset?key=image.contrast", openable: true, hint: I18n.t("Сброс одного или нескольких ключей к default") },
            { group: I18n.t("API"), name: "live image", value: "POST http://" + httpHost + "/api/v1/image?contrast=50", copyValue: "http://" + httpHost + "/api/v1/image?contrast=50", openable: false, hint: I18n.t("Мгновенные live ISP параметры") },
            { group: I18n.t("API"), name: "reload pipeline", value: "http://" + httpHost + "/cgi-bin/j/mj-apply.cgi", copyValue: "http://" + httpHost + "/cgi-bin/j/mj-apply.cgi", openValue: "http://" + httpHost + "/cgi-bin/j/mj-apply.cgi", openable: true, hint: I18n.t("Применить codec/resolution/fps без reboot камеры") },
            { group: I18n.t("OpenIPC"), name: "live logs", value: "ws://" + httpHost + "/ws/logs", copyValue: "ws://" + httpHost + "/ws/logs", openable: false, hint: I18n.t("Live logread stream из firmware WebUI") },
            { group: I18n.t("OpenIPC"), name: "firmware upgrade", value: "ws://" + httpHost + "/ws/upgrade", copyValue: "ws://" + httpHost + "/ws/upgrade", openable: false, hint: I18n.t("WebSocket updater: JSON source/kernel/rootfs/reset/force") },
            { group: I18n.t("OpenIPC"), name: "upload firmware", value: "POST http://" + httpHost + "/upload", copyValue: "http://" + httpHost + "/upload", openable: false, hint: I18n.t("Загрузка архива в /tmp/firmware.tgz перед /ws/upgrade") },
            { group: I18n.t("OpenIPC"), name: "status", value: "http://" + httpHost + "/cgi-bin/status.cgi", copyValue: "http://" + httpHost + "/cgi-bin/status.cgi", openValue: "http://" + httpHost + "/cgi-bin/status.cgi", openable: true, hint: I18n.t("Статус устройства OpenIPC WebUI") },
            { group: I18n.t("OpenIPC"), name: "network", value: "http://" + httpHost + "/cgi-bin/fw-network.cgi", copyValue: "http://" + httpHost + "/cgi-bin/fw-network.cgi", openValue: "http://" + httpHost + "/cgi-bin/fw-network.cgi", openable: true, hint: I18n.t("Сетевые настройки OpenIPC WebUI") },
            { group: I18n.t("OpenIPC"), name: "time", value: "http://" + httpHost + "/cgi-bin/fw-time.cgi", copyValue: "http://" + httpHost + "/cgi-bin/fw-time.cgi", openValue: "http://" + httpHost + "/cgi-bin/fw-time.cgi", openable: true, hint: I18n.t("Время и NTP OpenIPC WebUI") },
            { group: I18n.t("OpenIPC"), name: "update", value: "http://" + httpHost + "/cgi-bin/fw-update.cgi", copyValue: "http://" + httpHost + "/cgi-bin/fw-update.cgi", openValue: "http://" + httpHost + "/cgi-bin/fw-update.cgi", openable: true, hint: I18n.t("Штатная страница firmware update") },
            { group: I18n.t("Аудио"), name: "play_audio", value: "POST http://" + httpHost + "/play_audio", copyValue: "http://" + httpHost + "/play_audio", openable: false, hint: I18n.t("Передать PCM S16LE на динамик камеры") },
            { group: I18n.t("Мониторинг"), name: "metrics", value: "http://" + httpHost + "/metrics", copyValue: "http://" + httpHost + "/metrics", openValue: "http://" + httpHost + "/metrics", openable: true, hint: I18n.t("Prometheus-метрики") }
        ]
        return decorateEndpointRows(rows)
    }

    function endpointSummaryRows() {
        var rows = endpointRows()
        var ws = 0
        var openable = 0
        var risky = 0
        for (var i = 0; i < rows.length; ++i) {
            if (String(rows[i].value || "").indexOf("ws://") >= 0 || String(rows[i].value || "").indexOf("wss://") >= 0) ws++
            if (rows[i].openable === true) openable++
            if (rows[i].risk === "danger" || rows[i].risk === "warn") risky++
        }
        var caps = capabilityRows()
        var enabledCaps = 0
        for (var j = 0; j < caps.length; ++j) if (caps[j].value === true) enabledCaps++
        return [
            { title: I18n.t("Endpoints"), value: String(rows.length), subtitle: I18n.t("Majestic/OpenIPC адреса") },
            { title: "WebSocket", value: String(ws), subtitle: SystemController.firmwareClient.webSocketsAvailable ? I18n.t("native модуль включён") : I18n.t("только справочно") },
            { title: I18n.t("Открываемые"), value: String(openable), subtitle: I18n.t("можно открыть из приложения") },
            { title: I18n.t("Risk-gated"), value: String(risky), subtitle: I18n.t("требуют внимания") },
            { title: I18n.t("Capabilities"), value: enabledCaps + "/" + caps.length, subtitle: I18n.t("подтверждено этой камерой") }
        ]
    }

    onOpened: {
        requestIds = {}
        metricsData = ({})
        metricsText = ""
        metricsFilterText = ""
        metricsUpdatedAt = ""
        firmwareStatus = ({})
        firmwareNetwork = ({})
        firmwareTime = ({})
        firmwareUpdateInfo = ({})
        firmwareLogsText = ""
        firmwareLogsSource = "all"
        firmwareLiveLogs = false
        firmwareLogsPaused = false
        firmwareLogFilter = ""
        firmwareUpgradeText = ""
        firmwareUpgradeRebooting = false
        resetFirmwareArchiveState()
        firmwarePowerSafetyConfirmed = false
        firmwareDangerOptionsConfirmed = false
        firmwarePostReturnProbeActive = false
        firmwareUpdateKernel = true
        firmwareUpdateRootfs = true
        firmwareUpdateReset = false
        firmwareUpdateForce = false
        firmwareBackupSaved = false
        firmwareReturnPolling = false
        firmwareReturnPollTries = 0
        firmwareReturnPhase = "idle"
        firmwareReturnHealthText = ""
        activeFirmwareReturnProbeId = ""
        clearBackupRestore()
        clearRollbackSnapshot()
        refresh()
        loadFirmwareStatus()
        loadFirmwareNetwork()
        loadFirmwareTime()
        refreshFirmwareUpdateInfo()
    }

    onClosed: {
        if (firmwareLiveLogs) stopFirmwareLiveLogs()
        stopFirmwareReturnPolling("", false)
    }

    background: Rectangle { color: Theme.panelBackground; border.color: Theme.panelBorderStrong; radius: Theme.radiusLg }










    header: Rectangle {
        implicitHeight: 66
        color: Theme.topBarBackground
        radius: Theme.radiusLg
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 12
            spacing: 12
            Rectangle {
                Layout.preferredWidth: 38; Layout.preferredHeight: 38; radius: 19; color: Theme.metroDeepBlue
                Text { anchors.centerIn: parent; text: "O"; color: Theme.infoText; font.bold: true; font.pixelSize: 18 }
            }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 1
                Text { text: I18n.t("OpenIPC Control Center") + (cameraName.length ? " — " + cameraName : ""); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                Text { Layout.fillWidth: true; text: cameraHost + ":" + cameraPort + " · " + statusText; color: statusError ? Theme.danger : (loading ? Theme.warning : Theme.textMuted); font.pixelSize: 12; elide: Text.ElideRight }
            }
            BusyIndicator { running: loading; visible: running; Layout.preferredWidth: 30; Layout.preferredHeight: 30 }
            MajesticButton { text: "↻"; subtle: true; enabled: !loading; onClicked: refresh() }
            MetroWindowButton { kind: "close"; Layout.preferredWidth: 38; Layout.preferredHeight: 34; onClicked: dialog.close() }
        }
    }

    contentItem: ColumnLayout {
        spacing: 0
        TabBar {
            id: tabs
            Layout.fillWidth: true; Layout.preferredHeight: 44
            background: Rectangle { color: Theme.panelAltBackground }
            onCurrentIndexChanged: dialog.syncSelectedGroupFromTab()
            Repeater {
                model: { revision; return dialog.tabItems() }
                delegate: MajesticTabButton {
                    required property var modelData
                    text: modelData.label
                }
            }
        }
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: dialog.currentContentIndex()

            MajesticOverviewPage {
                id: overviewPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            MajesticSettingsPage {
                id: settingsPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            OpenIpcStatusPage {
                id: firmwarePage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            OpenIpcNetworkPage {
                id: networkPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            OpenIpcTimePage {
                id: timePage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            OpenIpcUpdatePage {
                id: updatePage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            OpenIpcToolsPage {
                id: toolsPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            MajesticEndpointsPage {
                id: endpointsPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            MajesticRawJsonPage {
                id: rawJsonPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            MajesticMetricsPage {
                id: metricsPage
                controller: dialog
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Timer {
        id: liveImageTimer; interval: 160
        onTriggered: {
            var values = {}
            for (var i = 0; i < fields.length; ++i) if (fields[i].live) {
                var tail = fields[i].path.substring(fields[i].path.lastIndexOf(".") + 1)
                values[tail] = draftValues[fields[i].path]
            }
            track(SystemController.majesticClient.applyLiveImage(cameraHost, cameraPort, cameraUser, cameraPassword, values))
        }
    }

    Timer {
        id: firmwareLiveLogsTimer
        interval: 2500
        repeat: true
        onTriggered: {
            if (!firmwareLiveLogs || SystemController.firmwareClient.webSocketsAvailable) return
            if (firmwareLogsPaused) return
            if (!activeFirmwareLogsId.length) loadFirmwareLogs(firmwareLogsSource, true)
        }
    }

    Timer {
        id: firmwareReturnPollTimer
        interval: 3000
        repeat: true
        onTriggered: probeFirmwareReturn()
    }

    Timer {
        id: rollbackHealthWatchTimer
        interval: 3500
        repeat: true
        onTriggered: {
            if (!rollbackWatchActive || !rollbackAvailable) {
                rollbackWatchActive = false
                stop()
                return
            }
            if (majesticApiProbeState === "running" || rtspMainProbeState === "running") return
            if (rollbackWatchTries >= rollbackWatchMaxTries) {
                maybeFinishRollbackHealthProbe()
                if (rollbackWatchActive) {
                    rollbackWatchActive = false
                    rollbackHealthState = "fail"
                    rollbackReason = I18n.t("После apply поток/API не восстановились. Можно откатить критичные изменения.")
                    stop()
                }
                return
            }
            rollbackWatchTries += 1
            rollbackHealthState = "watching"
            rollbackHealthText = I18n.t("Health probe Majestic после apply: попытка %1/%2", [rollbackWatchTries, rollbackWatchMaxTries])
            startEndpointProbe("api")
            startEndpointProbe("main")
        }
    }

    Timer {
        id: applyWatchdogTimer
        interval: 25000
        repeat: false
        onTriggered: {
            if (!activeApplyId.length) return
            loading = false
            statusError = true
            statusText = I18n.t("Majestic не ответил на сохранение за %1 секунд. Проверьте доступность камеры и повторите попытку.", [Math.round(interval / 1000)])
            untrack(activeApplyId)
            if (activeApplyId === activeRollbackId) activeRollbackId = ""
            activeApplyId = ""
        }
    }

    Timer {
        id: reloadRefreshTimer
        interval: 2500
        repeat: false
        onTriggered: refresh()
    }

    Timer {
        id: reloadWatchdogTimer
        interval: 8000
        repeat: false
        onTriggered: {
            if (!dialog.activeReloadId.length) return
            dialog.untrack(dialog.activeReloadId)
            dialog.activeReloadId = ""
            dialog.loading = false
            dialog.statusError = false
            dialog.pipelineReloadNeeded = false
            dialog.statusText = I18n.t("Reload pipeline отправлен; перечитываю состояние…")
            dialog.startRollbackHealthWatch("Reload pipeline отправлен; проверяю восстановление API/RTSP…")
            reloadRefreshTimer.restart()
        }
    }

    Connections {
        target: SystemController.majesticClient
        function onConfigurationLoaded(requestId, config, schema, loadedFields, loadedCapabilities) {
            if (requestId !== activeLoadId) return
            loading = false; originalConfig = clone(config); currentSchema = clone(schema); fields = loadedFields; capabilities = loadedCapabilities
            rawJsonPage.text = JSON.stringify(config, null, 2); resetDraft(); statusError = false
            statusText = pipelineReloadNeeded
                         ? I18n.t("Конфигурация сохранена; примените reload pipeline")
                         : (loadedCapabilities.schema ? I18n.t("Majestic подключён, schema загружена") : I18n.t("Majestic подключён в legacy-режиме"))
            if (loadedCapabilities.metrics === true) refreshMetrics()
        }
        function onConfigurationApplied(requestId) {
            if (requestId !== activeApplyId) return
            var rollbackFlow = requestId === activeRollbackId
            applyWatchdogTimer.stop()
            untrack(activeApplyId)
            activeApplyId = ""
            if (rollbackFlow) clearRollbackSnapshot()
            pipelineReloadNeeded = pendingPipelineReloadNeeded
            if (pipelineReloadNeeded && pendingAutoReloadAfterApply && capabilities.pipelineReload === true) {
                triggerPipelineReload(rollbackFlow ? "Откат сохранён; отправляю reload pipeline…" : "Конфигурация сохранена; отправляю reload pipeline…")
                return
            }
            statusText = rollbackFlow
                         ? I18n.t("Откат сохранён; перечитываю конфигурацию…")
                         : pipelineReloadNeeded
                         ? I18n.t("Конфигурация сохранена; примените reload pipeline")
                         : I18n.t("Конфигурация сохранена; live-параметры применены")
            if (!rollbackFlow && !pipelineReloadNeeded)
                startRollbackHealthWatch("Конфигурация сохранена; проверяю Majestic API и RTSP…")
            refresh()
        }
        function onConfigurationFieldsReset(requestId, fieldPaths) {
            if (requestId !== activeResetId) return
            pipelineReloadNeeded = activeResetNeedsPipelineReload
            statusText = pipelineReloadNeeded
                         ? I18n.t("Параметр сброшен; примените reload pipeline")
                         : I18n.t("Параметр сброшен; перечитываю состояние…")
            refresh()
        }
        function onMetricsLoaded(requestId, metrics, rawText) {
            if (!owns(requestId)) return
            untrack(requestId)
            if (requestId === activeMetricsId) activeMetricsId = ""
            metricsData = metrics; metricsText = rawText; metricsUpdatedAt = Qt.formatTime(new Date(), "HH:mm:ss"); statusError = false; statusText = I18n.t("Метрики обновлены: %1 значений", [Object.keys(metrics).length])
        }
        function onBackupLoaded(requestId, config, schema, path) {
            if (!owns(requestId)) return
            untrack(requestId)
            backupRestoreConfig = clone(config)
            backupRestoreSchema = clone(schema)
            backupRestorePath = path
            backupRestoreChanges = SystemController.majesticClient.describeChanges(originalConfig, backupRestoreConfig)
            rawJsonPage.text = JSON.stringify(config, null, 2)
            statusError = false
            statusText = I18n.t("Backup загружен: %1 · отличий: %2", [path, backupRestoreChanges.length])
        }
        function onOperationSucceeded(requestId, operation, result) {
            if (!owns(requestId) || operation === "load-config" || operation === "apply-config" || operation === "metrics" || operation === "backup-load") return
            untrack(requestId)
            loading = false; statusError = false
            if (operation === "reload-pipeline") {
                reloadWatchdogTimer.stop()
                if (requestId === activeReloadId) activeReloadId = ""
                pipelineReloadNeeded = false
                statusText = I18n.t("Reload pipeline отправлен; перечитываю состояние…")
                startRollbackHealthWatch("Reload pipeline отправлен; проверяю восстановление API/RTSP…")
                reloadRefreshTimer.restart()
            }
            else statusText = I18n.t("Операция выполнена: %1", [operation])
        }
        function onOperationFailed(requestId, operation, message, httpStatus) {
            if (!owns(requestId) && requestId !== activeLoadId && requestId !== activeApplyId) return
            if (operation === "apply-config" || requestId === activeApplyId) {
                applyWatchdogTimer.stop()
                untrack(activeApplyId)
                if (requestId === activeRollbackId) activeRollbackId = ""
                activeApplyId = ""
            }
            if (operation === "metrics" || requestId === activeMetricsId) {
                untrack(activeMetricsId)
                activeMetricsId = ""
            }
            if (operation === "reload-pipeline" || requestId === activeReloadId) {
                reloadWatchdogTimer.stop()
                untrack(activeReloadId)
                activeReloadId = ""
            }
            loading = false; statusError = true
            if (operation === "reload-pipeline" && httpStatus === 0) {
                statusError = false
                pipelineReloadNeeded = false
                statusText = I18n.t("Reload pipeline отправлен; камера может кратко не отвечать…")
                startRollbackHealthWatch("Reload pipeline отправлен; проверяю восстановление API/RTSP…")
                reloadRefreshTimer.restart()
                return
            }
            statusText = httpStatus === 401 ? I18n.t("Ошибка авторизации Majestic (401). Проверьте логин и пароль.") : operation + ": " + message
        }
    }

    Connections {
        target: SystemController.firmwareClient

        function onStatusLoaded(requestId, status) {
            if (!owns(requestId)) return
            if (requestId === activeFirmwareReturnProbeId) {
                untrack(requestId)
                activeFirmwareReturnProbeId = ""
                firmwareStatus = status
                var summary = firmwareReturnSummary(status)
                appendFirmwareUpgradeLogLine(I18n.t("Камера ответила после update: %1", [summary]))
                appendFirmwareUpgradeText("\n--- camera is back online ---\n")
                stopFirmwareReturnPolling(I18n.t("Камера вернулась после update"), false)
                firmwareReturnHealthText = summary
                loadFirmwareNetwork()
                loadFirmwareTime()
                refreshFirmwareUpdateInfo()
                if (capabilities.metrics === true) refreshOverviewMetrics()
                startPostFirmwareReturnProbes()
                return
            }
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareStatusId = ""
            firmwareStatus = status
            statusError = false
            statusText = I18n.t("Статус прошивки загружен")
        }

        function onNetworkLoaded(requestId, network) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareNetworkId = ""
            firmwareNetwork = network
            applyFirmwareNetworkToEditors(network)
            statusError = false
            statusText = I18n.t("Сетевые настройки загружены")
        }

        function onNetworkSaved(requestId, network) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareNetworkSaveId = ""
            firmwareNetwork = network
            statusError = false
            statusText = I18n.t("Сетевые настройки отправлены на камеру")
            loadFirmwareNetwork()
        }

        function onNetworkReset(requestId) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareNetworkSaveId = ""
            statusError = false
            statusText = I18n.t("Сетевая конфигурация сброшена")
            loadFirmwareNetwork()
        }

        function onWifiScanned(requestId, networks, error) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareNetworkId = ""
            firmwareWifiNetworks = networks || []
            statusError = !!(error && String(error).length)
            statusText = statusError ? error : I18n.t("Wi‑Fi scan: найдено %1 сетей", [firmwareWifiNetworks.length])
            if (firmwareWifiNetworks.length && !networkPage.wlanSsid.length) networkPage.wlanSsid = firmwareWifiNetworks[0].ssid || ""
        }

        function onTimeLoaded(requestId, time) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareTimeId = ""
            firmwareTime = time
            applyFirmwareTimeToEditors(time)
            statusError = false
            statusText = I18n.t("Настройки времени загружены")
        }

        function onTimeSaved(requestId, time) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareTimeSaveId = ""
            firmwareTime = time
            statusError = false
            statusText = I18n.t("Настройки времени отправлены на камеру")
            loadFirmwareTime()
        }

        function onTimeSynced(requestId, result) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareTimeSaveId = ""
            statusError = String(result.result || "") === "danger"
            statusText = result.message || I18n.t("Операция времени выполнена")
            loadFirmwareStatus()
            loadFirmwareTime()
        }

        function onLogsLoaded(requestId, source, text) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareLogsId = ""
            firmwareLogsSource = source
            if (!firmwareLogsPaused) firmwareLogsText = text
            statusError = false
            statusText = firmwareLiveLogs
                         ? I18n.t("Live logs обновлены: %1", [source])
                         : I18n.t("Логи загружены: %1", [source])
        }

        function onLogBufferSizeChanged(requestId, sizeKiB) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareLogsId = ""
            statusError = false
            statusText = I18n.t("Ring-buffer логов установлен: %1 KiB", [sizeKiB])
            loadFirmwareLogs(firmwareLogsSource, true)
        }

        function onBackupSaved(requestId, path) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareBackupId = ""
            firmwareBackupSaved = true
            statusError = false
            statusText = I18n.t("Firmware backup сохранён: %1", [path])
        }

        function onRebootStarted(requestId) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareRebootId = ""
            statusError = false
            statusText = I18n.t("Reboot отправлен. Камера временно пропадёт из сети.")
        }

        function onUpdateInfoLoaded(requestId, info) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareUpdateId = ""
            firmwareUpdateInfo = info
            statusError = false
            statusText = I18n.t("Информация update загружена")
        }

        function onFirmwareUploaded(requestId, remotePath) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareUpdateId = ""
            firmwareArchiveUploaded = true
            statusError = false
            statusText = I18n.t("Firmware archive загружен: %1 · %2", [remotePath, firmwareArchiveSizeText()])
        }

        function onUpdateStarted(requestId, mode) {
            if (!owns(requestId)) return
            firmwareBusy = false
            statusError = false
            firmwareUpgradeRebooting = false
            firmwareUpgradeText = I18n.t("Updater started: %1", [mode]) + "\n"
            statusText = I18n.t("Firmware updater запущен через /ws/upgrade")
        }

        function onFirmwareUpgradeOutput(requestId, text) {
            if (requestId !== activeFirmwareUpdateId) return
            appendFirmwareUpgradeText(text)
        }

        function onFirmwareUpgradeRebooting(requestId) {
            if (requestId !== activeFirmwareUpdateId) return
            firmwareBusy = false
            firmwareUpgradeRebooting = true
            appendFirmwareUpgradeText("\n--- flashing/rebooting ---\n")
            startFirmwareReturnPolling()
            statusError = false
            statusText = I18n.t("Прошивка перешла в flashing/rebooting. Не выключайте питание камеры.")
        }

        function onLiveLogsStarted(requestId) {
            if (requestId !== activeFirmwareLiveLogsId) return
            statusError = false
            statusText = I18n.t("Live logs подключены через /ws/logs")
        }

        function onLiveLogChunk(requestId, text) {
            if (requestId !== activeFirmwareLiveLogsId) return
            appendFirmwareLogText(text)
        }

        function onLiveLogsStopped(requestId, reason) {
            if (requestId !== activeFirmwareLiveLogsId) return
            untrack(requestId)
            activeFirmwareLiveLogsId = ""
            firmwareLiveLogs = false
            statusError = false
            statusText = reason === "stopped" ? I18n.t("Live logs остановлены") : I18n.t("Live logs закрыты: %1", [reason])
        }

        function onOperationSucceeded(requestId, operation, result) {
            if (!owns(requestId)) return
            if (operation === "firmware-update-start") return
            untrack(requestId)
            firmwareBusy = false
            if (requestId === activeFirmwareUpdateId) activeFirmwareUpdateId = ""
            if (operation === "firmware-update") {
                statusError = false
                statusText = result || I18n.t("Firmware update запущен; ожидайте возвращения камеры")
                if (!firmwareReturnPolling) startFirmwareReturnPolling()
            }
        }

        function onOperationFailed(requestId, operation, message, httpStatus) {
            if (!owns(requestId)) return
            if (requestId === activeFirmwareReturnProbeId) {
                untrack(requestId)
                activeFirmwareReturnProbeId = ""
                firmwareBusy = false
                if (firmwareReturnPollTries >= firmwareReturnPollMaxTries) {
                    appendFirmwareUpgradeLogLine(I18n.t("Камера не ответила после %1 попыток.", [firmwareReturnPollMaxTries]))
                    stopFirmwareReturnPolling(I18n.t("Камера не вернулась после update. Проверьте питание и сеть."), true)
                } else {
                    firmwareReturnPhase = "waiting"
                    firmwareReturnHealthText = I18n.t("Status endpoint ещё недоступен: %1", [message])
                    appendFirmwareUpgradeLogLine(I18n.t("Status endpoint ещё недоступен: %1", [message]))
                    statusError = false
                    statusText = I18n.t("Ожидание возврата камеры… попытка %1/%2", [firmwareReturnPollTries, firmwareReturnPollMaxTries])
                }
                return
            }
            untrack(requestId)
            firmwareBusy = false
            if (requestId === activeFirmwareStatusId) activeFirmwareStatusId = ""
            if (requestId === activeFirmwareNetworkId) activeFirmwareNetworkId = ""
            if (requestId === activeFirmwareNetworkSaveId) activeFirmwareNetworkSaveId = ""
            if (requestId === activeFirmwareTimeId) activeFirmwareTimeId = ""
            if (requestId === activeFirmwareTimeSaveId) activeFirmwareTimeSaveId = ""
            if (requestId === activeFirmwareLogsId) activeFirmwareLogsId = ""
            if (requestId === activeFirmwareLiveLogsId) {
                activeFirmwareLiveLogsId = ""
                firmwareLiveLogs = false
                firmwareLiveLogsTimer.stop()
            }
            if (requestId === activeFirmwareBackupId) activeFirmwareBackupId = ""
            if (requestId === activeFirmwareRebootId) activeFirmwareRebootId = ""
            if (requestId === activeFirmwareUpdateId) {
                activeFirmwareUpdateId = ""
                if (operation === "firmware-upload") resetFirmwareArchiveState()
            }
            statusError = true
            statusText = httpStatus === 401
                         ? I18n.t("Ошибка авторизации OpenIPC WebUI (401). Проверьте root-пароль.")
                         : operation + ": " + message
        }
    }

    Connections {
        target: SystemController
        function onCameraEndpointProbeFinished(requestId, kind, host, port, success, message, httpStatus, elapsedMs) {
            var state = success ? "ok" : "fail"
            if (requestId === dialog.rtspMainProbeId) {
                dialog.rtspMainProbeId = ""
                dialog.rtspMainProbeState = state
                dialog.rtspMainProbeMessage = message
                dialog.rtspMainProbeElapsedMs = elapsedMs
            } else if (requestId === dialog.rtspSubProbeId) {
                dialog.rtspSubProbeId = ""
                dialog.rtspSubProbeState = state
                dialog.rtspSubProbeMessage = message
                dialog.rtspSubProbeElapsedMs = elapsedMs
            } else if (requestId === dialog.majesticApiProbeId) {
                dialog.majesticApiProbeId = ""
                dialog.majesticApiProbeState = state
                dialog.majesticApiProbeMessage = message
                dialog.majesticApiProbeElapsedMs = elapsedMs
            }
            dialog.maybeFinishPostFirmwareReturnProbes()
            dialog.maybeFinishRollbackHealthProbe()
        }
    }

    Dialog {
        id: applyConfirm
        modal: true; anchors.centerIn: parent; width: Math.min(dialog.width - 80, 720); height: Math.min(dialog.height - 100, 560)
        title: I18n.t("Проверка изменений"); standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: dialog.applyPending()
        contentItem: ColumnLayout {
            spacing: 8
            Text { Layout.fillWidth: true; text: I18n.t("На камеру будет отправлен только этот patch (%1 изменений):", [dialog.pendingChanges.length]); color: Theme.textSecondary; wrapMode: Text.WordWrap }
            ListView {
                Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: dialog.pendingChanges
                delegate: Rectangle {
                    required property var modelData
                    required property int index
                    property bool alternate: (index & 1) === 1
                    width: ListView.view.width; height: 62; color: alternate ? Theme.panelSoftBackground : Theme.cardBackground
                    ColumnLayout {
                        property var row: parent.modelData
                        anchors.fill: parent
                        anchors.margins: 7
                        Text { text: parent.row.path; color: Theme.accentHover; font.family: "Consolas"; font.pixelSize: 11 }
                        Text { Layout.fillWidth: true; text: String(parent.row.before) + "  →  " + String(parent.row.after); color: Theme.textSecondary; elide: Text.ElideRight; font.pixelSize: 11 }
                    }
                }
            }
            Rectangle {
                visible: dialog.pendingPipelineReloadNeeded
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? 72 : 0
                color: Theme.panelSoftBackground
                border.color: Theme.warning
                border.width: 1
                radius: Theme.radiusSm
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4
                    MajesticCheckBox {
                        text: I18n.t("После сохранения сразу выполнить reload pipeline")
                        checked: dialog.autoReloadAfterApply
                        enabled: dialog.capabilities.pipelineReload === true
                        onToggled: dialog.autoReloadAfterApply = checked
                    }
                    Text {
                        Layout.fillWidth: true
                        text: dialog.capabilities.pipelineReload === true
                              ? I18n.t("Reload применит codec/resolution/fps без reboot камеры; видеопоток кратко мигнёт.")
                              : I18n.t("Эта сборка Majestic не сообщает endpoint reload pipeline — после сохранения примените изменения вручную на камере.")
                        color: Theme.textMuted
                        wrapMode: Text.WordWrap
                        font.pixelSize: 11
                    }
                }
            }
            Text { Layout.fillWidth: true; text: I18n.t("Видеопоток может кратковременно прерваться после применения pipeline."); color: Theme.warning; font.pixelSize: 11 }
        }
    }

    MajesticConfirmDialog {
        id: rollbackConfirm
        dialogWidth: 620
        title: I18n.t("Откатить критичные настройки Majestic")
        message: I18n.t("Будет отправлен diff, который вернёт конфигурацию Majestic к снимку, сохранённому перед последним критичным apply. После отката может потребоваться reload pipeline.")
        messageColor: Theme.warning
        onAccepted: dialog.rollbackPendingChanges()
    }

    MajesticConfirmDialog {
        id: firmwareNetworkConfirm
        title: I18n.t("Подтвердить изменение сети")
        message: I18n.t("Сетевые настройки будут записаны в OpenIPC. Если IP/DHCP указан неверно, камера может стать недоступной до ручного восстановления. Продолжить?")
        onAccepted: dialog.saveFirmwareNetwork()
    }

    MajesticConfirmDialog {
        id: firmwareNetworkResetConfirm
        title: I18n.t("Сбросить сетевую конфигурацию")
        message: I18n.t("Будет восстановлена network-конфигурация из прошивки. Все текущие изменения сети будут потеряны.")
        messageColor: Theme.warning
        onAccepted: dialog.resetFirmwareNetwork()
    }

    MajesticConfirmDialog {
        id: firmwareTimeConfirm
        title: I18n.t("Сохранить время и NTP")
        message: I18n.t("Настройки timezone и NTP будут записаны в /etc/TZ, /etc/timezone и /etc/ntp.conf на камере.")
        onAccepted: dialog.saveFirmwareTime()
    }

    MajesticConfirmDialog {
        id: firmwareRebootConfirm
        title: I18n.t("Перезагрузить камеру")
        message: I18n.t("Камера будет перезагружена через штатный fw-restart.cgi. Видео и WebUI временно пропадут. Продолжить?")
        messageColor: Theme.warning
        onAccepted: dialog.requestFirmwareReboot()
    }

    MajesticConfirmDialog {
        id: firmwareRestoreWebUiConfirm
        dialogWidth: 620
        title: I18n.t("Восстановить OpenIPC backup")
        message: I18n.t("Восстановление полного OpenIPC backup может изменить overlay, сеть, пароли и сервисы камеры. Dashboard откроет штатную страницу WebUI камеры; продолжайте только если backup точно от этой камеры.")
        messageColor: Theme.warning
        onAccepted: dialog.openWebUiPath("/cgi-bin/ext-backuper.cgi")
    }

    MajesticConfirmDialog {
        id: firmwareUpdateConfirm
        dialogWidth: 620
        title: I18n.t("Запустить обновление прошивки")
        message: I18n.t("Будет запущен GitHub update через /ws/upgrade. Опции: %1. Камера остановит видео и перезагрузится. Продолжить?", [dialog.firmwareUpdateOptionsSummary()])
        messageColor: Theme.warning
        onAccepted: dialog.startGithubFirmwareUpdate()
    }

    MajesticConfirmDialog {
        id: firmwareUploadedUpdateConfirm
        dialogWidth: 620
        title: I18n.t("Прошить загруженный архив")
        message: I18n.t("Будет запущен /ws/upgrade с source=/tmp/firmware.tgz. Опции: %1. Убедитесь, что архив подходит этой камере и питание не будет отключено.", [dialog.firmwareUpdateOptionsSummary()])
        messageColor: Theme.warning
        onAccepted: dialog.startUploadedFirmwareUpdate()
    }

    FileDialog {
        id: snapshotDialog; title: I18n.t("Сохранить снимок Majestic"); fileMode: FileDialog.SaveFile; defaultSuffix: "jpg"
        nameFilters: [I18n.t("JPEG (*.jpg *.jpeg)"), I18n.t("Все файлы (*)")]
        onAccepted: track(SystemController.majesticClient.takeSnapshot(cameraHost, cameraPort, cameraUser, cameraPassword, selectedFile.toString(), overviewPage.snapshotWidth, overviewPage.snapshotHeight, overviewPage.snapshotQuality, overviewPage.snapshotGray))
    }
    FileDialog {
        id: saveBackupDialog; title: I18n.t("Сохранить backup Majestic"); fileMode: FileDialog.SaveFile; defaultSuffix: "json"
        nameFilters: [I18n.t("JSON (*.json)"), I18n.t("Все файлы (*)")]
        onAccepted: track(SystemController.majesticClient.saveConfigurationBackup(originalConfig, currentSchema, selectedFile.toString()))
    }
    FileDialog {
        id: openBackupDialog; title: I18n.t("Открыть backup Majestic"); fileMode: FileDialog.OpenFile
        nameFilters: [I18n.t("JSON (*.json)"), I18n.t("Все файлы (*)")]
        onAccepted: track(SystemController.majesticClient.loadConfigurationBackup(selectedFile.toString()))
    }
    FileDialog {
        id: pcmDialog; title: I18n.t("Выбрать PCM (S16 LE, 8 кГц, mono)"); fileMode: FileDialog.OpenFile
        nameFilters: [I18n.t("PCM (*.pcm *.raw)"), I18n.t("Все файлы (*)")]
        onAccepted: track(SystemController.majesticClient.playPcmFile(cameraHost, cameraPort, cameraUser, cameraPassword, selectedFile.toString()))
    }
    FileDialog {
        id: firmwareBackupDialog
        title: I18n.t("Сохранить firmware backup OpenIPC")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "tgz"
        nameFilters: [I18n.t("OpenIPC backup (*.tgz *.tar.gz)"), I18n.t("Все файлы (*)")]
        onAccepted: saveFullFirmwareBackup(String(selectedFile))
    }
    FileDialog {
        id: firmwareUploadDialog
        title: I18n.t("Выбрать firmware archive OpenIPC")
        fileMode: FileDialog.OpenFile
        nameFilters: [I18n.t("OpenIPC firmware (*.tgz *.gz)"), I18n.t("Все файлы (*)")]
        onAccepted: uploadFirmwareArchive(String(selectedFile))
    }
}
