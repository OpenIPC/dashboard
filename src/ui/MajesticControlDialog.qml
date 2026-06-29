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
    property string firmwareLogsSource: "syslog"
    property string activeFirmwareStatusId: ""
    property string activeFirmwareNetworkId: ""
    property string activeFirmwareNetworkSaveId: ""
    property string activeFirmwareTimeId: ""
    property string activeFirmwareTimeSaveId: ""
    property string activeFirmwareLogsId: ""
    property string activeFirmwareBackupId: ""
    property string activeFirmwareRebootId: ""
    property string activeFirmwareUpdateId: ""

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
            hostname: networkHostname.text,
            interface: networkInterface.currentText,
            dhcp: networkDhcp.checked,
            address: networkAddress.text,
            netmask: networkNetmask.text,
            gateway: networkGateway.text,
            nameserver: networkDns.text,
            wlanSsid: networkWlanSsid.text,
            wlanPassword: networkWlanPassword.text
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
            zoneName: timeZoneName.text,
            zoneData: timeZoneData.text,
            servers: [timeServer0.text, timeServer1.text, timeServer2.text, timeServer3.text]
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

    function loadFirmwareLogs(source) {
        firmwareLogsSource = source || "syslog"
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Чтение логов OpenIPC…")
        activeFirmwareLogsId = track(SystemController.firmwareClient.loadLogs(
                                         cameraHost, cameraPort, cameraUser, cameraPassword,
                                         firmwareLogsSource, 300))
    }

    function refreshFirmwareUpdateInfo() {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Чтение информации об обновлении прошивки…")
        activeFirmwareUpdateId = track(SystemController.firmwareClient.loadUpdateInfo(
                                           cameraHost, cameraPort, cameraUser, cameraPassword))
    }

    function uploadFirmwareArchive(path) {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Загрузка firmware-архива на камеру…")
        activeFirmwareUpdateId = track(SystemController.firmwareClient.uploadFirmwareArchive(
                                           cameraHost, cameraPort, cameraUser, cameraPassword, path))
    }

    function startGithubFirmwareUpdate() {
        firmwareBusy = true
        statusError = false
        statusText = I18n.t("Запуск updater OpenIPC…")
        activeFirmwareUpdateId = track(SystemController.firmwareClient.startGithubUpdate(
                                           cameraHost, cameraPort, cameraUser, cameraPassword, true, true, false, false))
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

    function applyFirmwareNetworkToEditors(network) {
        var current = network.current || {}
        networkHostname.text = network.hostname || current.hostname || ""
        var iface = network.interface || current.interface || "eth0"
        networkInterface.currentIndex = iface === "wlan0" ? 1 : 0
        networkDhcp.checked = network.dhcp === true || String((current.mode || "")).toLowerCase() === "dhcp"
        networkAddress.text = network.address || current.address || ""
        networkNetmask.text = network.netmask || current.netmask || ""
        networkGateway.text = network.gateway || current.gateway || ""
        networkDns.text = network.nameserver || current.nameserver || ""
        networkWlanSsid.text = network.wlanSsid || ""
        networkWlanPassword.text = network.wlanPassword || ""
    }

    function applyFirmwareTimeToEditors(time) {
        var servers = time.servers || []
        timeZoneName.text = time.zoneName || time.currentZoneName || ""
        timeZoneData.text = time.zoneData || time.currentZoneData || ""
        timeServer0.text = servers.length > 0 ? servers[0] : ""
        timeServer1.text = servers.length > 1 ? servers[1] : ""
        timeServer2.text = servers.length > 2 ? servers[2] : ""
        timeServer3.text = servers.length > 3 ? servers[3] : ""
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
        if (field.live && livePreview.checked) liveImageTimer.restart()
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
        var needle = settingsSearch.text.trim().toLowerCase()
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
            { label: I18n.t("Двустороннее аудио"), value: capabilities.playAudio === true }
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
        var needle = settingsSearch.text.trim().toLowerCase()
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
            { title: I18n.t("Температура"), value: pulse.soc_temp || tempText(), subtitle: statusDevice.soc || I18n.t("Температура SoC"), percent: Math.min(100, Number(metric("node_hwmon_temp_celsius", 0)) / 90 * 100), accent: "#f97316" },
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
        return [
            { title: I18n.t("1. Backup"), text: I18n.t("Перед прошивкой нужно сохранить конфигурацию Majestic и, желательно, полный backup flash/overlay.") },
            { title: I18n.t("2. Совместимость"), text: I18n.t("Образ должен совпадать с SoC, типом памяти и веткой firmware. Этот контроль будет обязательным в нативном updater.") },
            { title: I18n.t("3. Питание и сеть"), text: I18n.t("Камера не должна потерять питание или сеть во время update/reboot.") },
            { title: I18n.t("4. Подтверждение"), text: I18n.t("Опасные операции будут выполняться только после явного подтверждения пользователя.") }
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
            { title: I18n.t("Температура"), value: tempText(), subtitle: I18n.t("Температура SoC"), percent: Math.min(100, Number(metric("node_hwmon_temp_celsius", 0)) / 90 * 100), accent: "#f97316" },
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
                color: "#f97316"
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
        var parsed = SystemController.majesticClient.parseJsonObject(rawEditor.text)
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

    function endpointRows() {
        var host = cameraHost.length ? cameraHost : "camera"
        var rtspPort = majesticRtspPort()
        var rtspHost = host + (rtspPort && rtspPort !== 554 ? ":" + rtspPort : "")
        var httpHost = host + (cameraPort && cameraPort !== 80 ? ":" + cameraPort : "")
        var displayAuth = cameraUser.length ? encodeURIComponent(cameraUser) + (cameraPassword.length ? ":••••" : "") + "@" : ""
        var realAuth = cameraUser.length ? encodeURIComponent(cameraUser) + (cameraPassword.length ? ":" + encodeURIComponent(cameraPassword) : "") + "@" : ""
        return [
            { group: I18n.t("Видео"), name: I18n.t("Основной RTSP"), value: "rtsp://" + displayAuth + rtspHost + "/stream=0", copyValue: "rtsp://" + realAuth + rtspHost + "/stream=0", openValue: "rtsp://" + realAuth + rtspHost + "/stream=0", openable: true, hint: I18n.t("Главный поток Majestic") },
            { group: I18n.t("Видео"), name: I18n.t("Дополнительный RTSP"), value: "rtsp://" + displayAuth + rtspHost + "/stream=1", copyValue: "rtsp://" + realAuth + rtspHost + "/stream=1", openValue: "rtsp://" + realAuth + rtspHost + "/stream=1", openable: true, hint: I18n.t("Sub-stream, если включён в конфигурации") },
            { group: I18n.t("Видео"), name: I18n.t("WebSocket preview main"), value: "ws://" + httpHost + "/ws/video?stream=0", copyValue: "ws://" + httpHost + "/ws/video?stream=0", openable: false, hint: I18n.t("Низкая задержка, как в majestic-webui") },
            { group: I18n.t("Видео"), name: I18n.t("WebSocket preview sub"), value: "ws://" + httpHost + "/ws/video?stream=1", copyValue: "ws://" + httpHost + "/ws/video?stream=1", openable: false, hint: I18n.t("Второй поток предпросмотра") },
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
            { group: I18n.t("Аудио"), name: "play_audio", value: "POST http://" + httpHost + "/play_audio", copyValue: "http://" + httpHost + "/play_audio", openable: false, hint: I18n.t("Передать PCM S16LE на динамик камеры") },
            { group: I18n.t("Мониторинг"), name: "metrics", value: "http://" + httpHost + "/metrics", copyValue: "http://" + httpHost + "/metrics", openValue: "http://" + httpHost + "/metrics", openable: true, hint: I18n.t("Prometheus-метрики") }
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
        refresh()
        loadFirmwareStatus()
        loadFirmwareNetwork()
        loadFirmwareTime()
        refreshFirmwareUpdateInfo()
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
                Layout.preferredWidth: 38; Layout.preferredHeight: 38; radius: 19; color: "#172554"
                Text { anchors.centerIn: parent; text: "O"; color: "#93c5fd"; font.bold: true; font.pixelSize: 18 }
            }
            ColumnLayout {
                Layout.fillWidth: true; spacing: 1
                Text { text: I18n.t("OpenIPC Control Center") + (cameraName.length ? " — " + cameraName : ""); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                Text { Layout.fillWidth: true; text: cameraHost + ":" + cameraPort + " · " + statusText; color: statusError ? Theme.danger : (loading ? Theme.warning : Theme.textMuted); font.pixelSize: 12; elide: Text.ElideRight }
            }
            BusyIndicator { running: loading; visible: running; Layout.preferredWidth: 30; Layout.preferredHeight: 30 }
            MajesticButton { text: "↻"; subtle: true; enabled: !loading; onClicked: refresh() }
            MajesticButton { text: "✕"; subtle: true; onClicked: dialog.close() }
        }
    }

    footer: Rectangle {
        implicitHeight: 50
        color: Theme.topBarBackground
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
            Text {
                Layout.fillWidth: true
                text: pipelineReloadNeeded ? I18n.t("Структурные параметры сохранены: примените reload pipeline")
                                            : I18n.t("Поля получены от самой камеры — неподдерживаемые настройки скрыты")
                color: pipelineReloadNeeded ? Theme.warning : Theme.textMuted; font.pixelSize: 11; elide: Text.ElideRight
            }
            MajesticButton {
                visible: pipelineReloadNeeded; text: I18n.t("Применить reload")
                primary: true
                onClicked: triggerPipelineReload("")
            }
            MajesticButton { text: I18n.t("Закрыть"); onClicked: dialog.close() }
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
            Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: dialog.currentContentIndex()

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 10
                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Device Status")
                            color: Theme.accentHover
                            font.pixelSize: 28
                            font.bold: true
                        }
                        Rectangle {
                            width: 128
                            height: 26
                            radius: 13
                            color: statusError ? "#7f1d1d" : "#166534"
                            Text {
                                anchors.centerIn: parent
                                text: statusError ? I18n.t("Требует внимания") : I18n.t("All systems OK")
                                color: Theme.textPrimary
                                font.bold: true
                                font.pixelSize: 11
                            }
                        }
                        MajesticButton {
                            text: I18n.t("Обновить")
                            enabled: !loading
                            onClicked: {
                                refresh()
                                activeMetricsId = track(SystemController.majesticClient.loadMetrics(cameraHost, cameraPort, cameraUser, cameraPassword))
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.preferredHeight: 300
                        color: Theme.cardBackground
                        border.color: Theme.cardBorder
                        radius: Theme.radiusLg
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            RowLayout {
                                Layout.fillWidth: true
                                Text { Layout.fillWidth: true; text: I18n.t("Логи и обслуживание"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 17 }
                                MajesticButton { text: "syslog"; enabled: !firmwareBusy; onClicked: loadFirmwareLogs("syslog") }
                                MajesticButton { text: "majestic"; enabled: !firmwareBusy; onClicked: loadFirmwareLogs("majestic") }
                                MajesticButton { text: "dmesg"; enabled: !firmwareBusy; onClicked: loadFirmwareLogs("kernel") }
                                MajesticButton { text: I18n.t("Backup"); enabled: !firmwareBusy; onClicked: firmwareBackupDialog.open() }
                                MajesticButton { text: I18n.t("Reboot"); danger: true; enabled: !firmwareBusy; onClicked: firmwareRebootConfirm.open() }
                            }
                            ScrollView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                clip: true
                                TextArea {
                                    id: firmwareLogsArea
                                    readOnly: true
                                    wrapMode: TextEdit.NoWrap
                                    text: firmwareLogsText.length ? firmwareLogsText : I18n.t("Нажмите syslog, majestic или dmesg, чтобы прочитать логи камеры.")
                                    color: Theme.textSecondary
                                    selectedTextColor: Theme.textPrimary
                                    selectionColor: Theme.accent
                                    font.family: "Consolas"
                                    font.pixelSize: 11
                                    background: Rectangle {
                                        radius: Theme.radiusMd
                                        color: Theme.controlBackground
                                        border.color: Theme.controlBorder
                                    }
                                }
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 4 : 2
                        rowSpacing: 12
                        columnSpacing: 12

                        MajesticStatusCard { title: "LOAD"; value: String(metric("node_load1", "—")); subtitle: I18n.t("CPU load average"); percent: Math.min(100, Number(metric("node_load1", 0)) * 35); accent: Theme.accent }
                        MajesticStatusCard { title: "MEMORY"; value: ramPercent() + "%"; subtitle: ramText(); percent: ramPercent(); accent: Theme.accent }
                        MajesticStatusCard { title: "TEMPERATURE"; value: tempText(); subtitle: "SoC"; percent: Math.min(100, Number(metric("node_hwmon_temp_celsius", 0)) / 90 * 100); accent: "#f97316" }
                        MajesticStatusCard { title: "UPTIME"; value: uptimeText(); subtitle: "Majestic / node metrics"; percent: 100; accent: Theme.success }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 2 : 1
                        rowSpacing: 12
                        columnSpacing: 12

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 232
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10
                                Text { text: I18n.t("Streams"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                                Repeater {
                                    model: streamRows()
                                    delegate: ColumnLayout {
                                        required property var modelData
                                        Layout.fillWidth: true
                                        RowLayout {
                                            Rectangle {
                                                width: 56; height: 24; radius: 12; color: Theme.accent
                                                Text { anchors.centerIn: parent; text: modelData.name; color: Theme.textPrimary; font.bold: true; font.pixelSize: 11 }
                                            }
                                            Text { text: modelData.size; color: Theme.textPrimary; font.bold: true; font.pixelSize: 14 }
                                            Rectangle {
                                                visible: modelData.codec.length > 0
                                                width: 56; height: 22; radius: 11; color: "#f8fafc"
                                                Text { anchors.centerIn: parent; text: modelData.codec; color: "#0f172a"; font.bold: true; font.pixelSize: 10 }
                                            }
                                        }
                                        Text {
                                            text: (modelData.fps ? modelData.fps + " fps" : "") + (modelData.bitrate ? " · " + modelData.bitrate + " kbit/s" : "")
                                            color: Theme.textMuted
                                            font.pixelSize: 11
                                        }
                                    }
                                }
                                Text {
                                    visible: streamRows().length === 0
                                    text: I18n.t("Нет включённых потоков")
                                    color: Theme.textMuted
                                    font.pixelSize: 12
                                }
                                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.panelBorder }
                                Text {
                                    text: (metric("night_enabled", 0) ? "🌙 " + I18n.t("Ночь") : "☀ " + I18n.t("День"))
                                          + " · IR-cut " + (metric("ircut_enabled", 0) ? "on" : "off")
                                          + " · HLS " + metric("hls_clients_total", 0)
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 232
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10
                                Text { text: I18n.t("Возможности этой камеры"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: 2
                                    rowSpacing: 8
                                    columnSpacing: 18
                                    Repeater {
                                        model: capabilityRows()
                                        delegate: RowLayout {
                                            required property var modelData
                                            Layout.fillWidth: true
                                            Rectangle { width: 9; height: 9; radius: 5; color: modelData.value ? Theme.success : Theme.textFaint }
                                            Text { Layout.fillWidth: true; text: modelData.label; color: Theme.textSecondary; font.pixelSize: 12 }
                                        }
                                    }
                                }
                                Text { text: I18n.t("Доступно параметров: %1", [fields.length]); color: Theme.textMuted; font.pixelSize: 11 }
                                Text { text: cameraHost + ":" + cameraPort; color: Theme.accentHover; font.family: "Consolas"; font.pixelSize: 12 }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 190
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 14
                                Text { text: I18n.t("День / ночь и механика"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 15 }
                                Text { text: I18n.t("Команды выполняются сразу и не меняют majestic.yaml"); color: Theme.textMuted; font.pixelSize: 11 }
                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    MajesticButton { text: I18n.t("День"); onClicked: track(SystemController.majesticClient.setNightMode(cameraHost, cameraPort, cameraUser, cameraPassword, "off")) }
                                    MajesticButton { text: I18n.t("Ночь"); onClicked: track(SystemController.majesticClient.setNightMode(cameraHost, cameraPort, cameraUser, cameraPassword, "on")) }
                                    MajesticButton { text: I18n.t("Переключить"); primary: true; onClicked: track(SystemController.majesticClient.setNightMode(cameraHost, cameraPort, cameraUser, cameraPassword, "toggle")) }
                                    MajesticButton { text: I18n.t("IR-cut"); onClicked: track(SystemController.majesticClient.setNightMode(cameraHost, cameraPort, cameraUser, cameraPassword, "ircut")) }
                                    MajesticButton { text: I18n.t("ИК-подсветка"); onClicked: track(SystemController.majesticClient.setNightMode(cameraHost, cameraPort, cameraUser, cameraPassword, "light")) }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 190
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            ColumnLayout {
                                anchors.fill: parent; anchors.margins: 14
                                Text { text: I18n.t("Снимок, backup и звук"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 15 }
                                RowLayout {
                                    Label { text: I18n.t("Ширина"); color: Theme.textMuted }
                                    MajesticSpinBox { id: snapshotWidth; from: 0; to: 8192; editable: true }
                                    Label { text: I18n.t("Высота"); color: Theme.textMuted }
                                    MajesticSpinBox { id: snapshotHeight; from: 0; to: 8192; editable: true }
                                }
                                RowLayout {
                                    Label { text: I18n.t("Качество"); color: Theme.textMuted }
                                    MajesticSpinBox { id: snapshotQuality; from: 1; to: 100; value: 85; editable: true }
                                    MajesticCheckBox { id: snapshotGray; text: I18n.t("Ч/Б") }
                                }
                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    MajesticButton { text: I18n.t("Сохранить JPEG…"); primary: true; onClicked: snapshotDialog.open() }
                                    MajesticButton { text: I18n.t("Создать backup…"); enabled: fields.length > 0; onClicked: saveBackupDialog.open() }
                                    MajesticButton { text: I18n.t("Открыть backup…"); onClicked: openBackupDialog.open() }
                                    MajesticButton { text: I18n.t("Копировать без секретов"); enabled: fields.length > 0; onClicked: copyRedactedJson(originalConfig, I18n.t("Backup Majestic")) }
                                    MajesticButton { text: I18n.t("Передать PCM…"); onClicked: pcmDialog.open() }
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76
                    Layout.margins: 12
                    color: Theme.panelSoftBackground
                    border.color: Theme.panelBorder
                    radius: Theme.radiusMd

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10
                        ColumnLayout {
                            Layout.preferredWidth: 260
                            spacing: 2
                            Text { text: selectedGroupLabel(); color: Theme.textPrimary; font.bold: true; font.pixelSize: 14; elide: Text.ElideRight; Layout.fillWidth: true }
                            Text { text: I18n.t("%1 параметров · schema этой камеры", [groupFieldCount(selectedGroupId)]); color: Theme.textMuted; font.pixelSize: 11 }
                        }
                        MajesticTextField {
                            id: settingsSearch
                            Layout.fillWidth: true
                            placeholderText: I18n.t("Поиск по имени, пути или описанию…")
                        }
                        MajesticCheckBox { id: livePreview; text: I18n.t("Live ISP"); checked: true; enabled: capabilities.liveImage === true }
                        Text { text: I18n.t("Изменено: %1", [dirtyCount]); color: dirtyCount ? Theme.warning : Theme.textMuted; font.pixelSize: 12 }
                        MajesticButton { text: I18n.t("Отменить"); enabled: dirtyCount > 0; onClicked: resetDraft() }
                        MajesticButton { text: I18n.t("Проверить и применить"); primary: true; enabled: dirtyCount > 0 && capabilities.configWrite === true && !loading; onClicked: prepareApply(editedConfig()) }
                    }
                }

                Rectangle {
                    visible: pipelineReloadNeeded
                    Layout.fillWidth: true; Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.preferredHeight: visible ? 54 : 0
                    color: "#422006"; border.color: Theme.warning; radius: Theme.radiusSm
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 10; spacing: 10
                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Сохранено. Структурные изменения вступят в силу после reload pipeline; видеопотоки кратко мигнут.")
                            color: "#fde68a"; wrapMode: Text.WordWrap; font.pixelSize: 11
                        }
                        MajesticButton {
                            text: I18n.t("Применить reload")
                            primary: true
                            onClicked: triggerPipelineReload("")
                        }
                    }
                }

                Rectangle {
                    visible: capabilities.schema !== true && fields.length > 0
                    Layout.fillWidth: true; Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.preferredHeight: visible ? 48 : 0
                    color: "#422006"; border.color: Theme.warning; radius: Theme.radiusSm
                    Text { anchors.fill: parent; anchors.margins: 9; text: I18n.t("Старая сборка Majestic: чтение доступно, schema-safe запись отключена."); color: "#fde68a"; wrapMode: Text.WordWrap }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 12
                    Layout.topMargin: 0
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 230
                        Layout.fillHeight: true
                        color: Theme.panelAltBackground
                        border.color: Theme.panelBorder
                        radius: Theme.radiusLg
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8
                            Text {
                                text: I18n.t("Разделы Majestic")
                                color: Theme.textPrimary
                                font.bold: true
                                font.pixelSize: 14
                            }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Группы берутся из schema камеры. Неподдерживаемые функции не показываются.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                font.pixelSize: 10
                            }
                            Repeater {
                                model: groups
                                delegate: Button {
                                    id: groupButton
                                    required property var modelData
                                    Layout.fillWidth: true
                                    implicitHeight: 42
                                    onClicked: selectGroup(modelData.id)
                                    contentItem: RowLayout {
                                        Text {
                                            Layout.fillWidth: true
                                            text: localizedGroupLabel(modelData)
                                            color: selectedGroupId === modelData.id ? Theme.textPrimary : Theme.textSecondary
                                            font.bold: selectedGroupId === modelData.id
                                            font.pixelSize: 12
                                            elide: Text.ElideRight
                                        }
                                        Rectangle {
                                            width: 42
                                            height: 22
                                            radius: 11
                                            color: selectedGroupId === modelData.id ? Theme.accent : Theme.controlBackgroundAlt
                                            Text {
                                                anchors.centerIn: parent
                                                text: groupFieldCount(modelData.id)
                                                color: Theme.textPrimary
                                                font.bold: true
                                                font.pixelSize: 10
                                            }
                                        }
                                    }
                                    background: Rectangle {
                                        radius: Theme.radiusMd
                                        color: selectedGroupId === modelData.id ? "#1e3a8a" : (groupButton.hovered ? Theme.cardHover : Theme.controlBackground)
                                        border.color: selectedGroupId === modelData.id ? Theme.accent : Theme.controlBorder
                                    }
                                }
                            }
                            Item { Layout.fillHeight: true }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Все поля сохраняются diff-ом через /api/v1/config.")
                                color: Theme.textFaint
                                wrapMode: Text.WordWrap
                                font.pixelSize: 10
                            }
                        }
                    }

                    ScrollView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 12

                            RowLayout {
                                visible: liveFieldsForGroup(selectedGroupId).length > 0
                                Layout.fillWidth: true
                                spacing: 12

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 292
                                    color: Theme.cardBackground
                                    border.color: Theme.cardBorder
                                    radius: Theme.radiusLg
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 14
                                        spacing: 8
                                        Text { text: I18n.t("Live preview"); color: Theme.textMuted; font.pixelSize: 12 }
                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            color: "#000000"
                                            border.color: Theme.accent
                                            radius: Theme.radiusMd
                                            Text {
                                                anchors.centerIn: parent
                                                text: I18n.t("Предпросмотр берётся из текущего видеопотока Dashboard.\nMajestic endpoint: /ws/video?stream=0")
                                                color: Theme.textFaint
                                                horizontalAlignment: Text.AlignHCenter
                                                font.pixelSize: 12
                                            }
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.preferredWidth: 390
                                    Layout.preferredHeight: 292
                                    color: Theme.cardBackground
                                    border.color: Theme.cardBorder
                                    radius: Theme.radiusLg
                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 14
                                        spacing: 8
                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text { Layout.fillWidth: true; text: I18n.t("Live adjustments"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                                            MajesticButton {
                                                text: I18n.t("↺ Reset all")
                                                subtle: true
                                                enabled: liveFieldsForGroup(selectedGroupId).length > 0 && capabilities.resetDefaults === true
                                                onClicked: {
                                                    var paths = []
                                                    var live = liveFieldsForGroup(selectedGroupId)
                                                    for (var i = 0; i < live.length; ++i) paths.push(live[i].path)
                                                    requestResetMany(paths)
                                                }
                                            }
                                        }
                                        Repeater {
                                            model: liveFieldsForGroup(selectedGroupId)
                                            delegate: MajesticSettingFieldEditor {
                                                controller: dialog
                                                required property var modelData
                                                field: modelData
                                                compact: true
                                            }
                                        }
                                    }
                                }
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: width > 900 ? 2 : 1
                                rowSpacing: 12
                                columnSpacing: 12

                                Repeater {
                                    model: sectionCardsForGroup(selectedGroupId)
                                    delegate: Rectangle {
                                        required property var modelData
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: cardHeight(modelData)
                                        color: Theme.cardBackground
                                        border.color: Theme.cardBorder
                                        radius: Theme.radiusLg
                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.margins: 14
                                            spacing: 8
                                            RowLayout {
                                                Layout.fillWidth: true
                                                Text {
                                                    Layout.fillWidth: true
                                                    text: modelData.label
                                                    color: Theme.textPrimary
                                                    font.bold: true
                                                    font.pixelSize: 18
                                                    elide: Text.ElideRight
                                                }
                                                Rectangle {
                                                    width: 44; height: 22; radius: 11; color: Theme.controlBackgroundAlt
                                                    Text { anchors.centerIn: parent; text: modelData.fields.length; color: Theme.textMuted; font.pixelSize: 10; font.bold: true }
                                                }
                                            }
                                            Repeater {
                                                model: modelData.fields
                                                delegate: MajesticSettingFieldEditor {
                                                    controller: dialog
                                                    required property var modelData
                                                    field: modelData
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                visible: sectionCardsForGroup(selectedGroupId).length === 0 && liveFieldsForGroup(selectedGroupId).length === 0
                                Layout.fillWidth: true
                                Layout.preferredHeight: visible ? 120 : 0
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder
                                radius: Theme.radiusLg
                                Text {
                                    anchors.centerIn: parent
                                    text: I18n.t("В этом разделе нет доступных полей для текущей schema или фильтра поиска.")
                                    color: Theme.textMuted
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
            }

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 12
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text {
                                text: I18n.t("OpenIPC Firmware")
                                color: Theme.accentHover
                                font.pixelSize: 28
                                font.bold: true
                            }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Единый центр управления камерой: системное состояние, WebUI, Majestic и безопасные firmware-действия.")
                                color: Theme.textMuted
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }
                        }
                        MajesticButton {
                            text: I18n.t("Открыть WebUI")
                            primary: true
                            onClicked: openWebUiPath("")
                        }
                        MajesticButton {
                            text: I18n.t("Обновить")
                            enabled: !loading
                            onClicked: {
                                refresh()
                                refreshMetrics()
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 4 : 2
                        rowSpacing: 12
                        columnSpacing: 12

                        Repeater {
                            model: firmwareSystemRows()
                            delegate: MajesticStatusCard {
                                required property var modelData
                                title: modelData.title
                                value: modelData.value
                                subtitle: modelData.subtitle
                                percent: modelData.percent
                                accent: modelData.accent
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 2 : 1
                        rowSpacing: 12
                        columnSpacing: 12

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 252
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10
                                Text { text: I18n.t("Идентификация камеры"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                                Repeater {
                                    model: firmwareIdentityRows()
                                    delegate: RowLayout {
                                        required property var modelData
                                        Layout.fillWidth: true
                                        spacing: 10
                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 1
                                            Text { text: modelData.label; color: Theme.textPrimary; font.bold: true; font.pixelSize: 12 }
                                            Text { Layout.fillWidth: true; text: modelData.hint; color: Theme.textMuted; font.pixelSize: 10; elide: Text.ElideRight }
                                        }
                                        Text {
                                            Layout.preferredWidth: 210
                                            text: modelData.value
                                            color: Theme.accentHover
                                            font.family: "Consolas"
                                            font.pixelSize: 11
                                            horizontalAlignment: Text.AlignRight
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 252
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10
                                Text { text: I18n.t("Быстрые действия"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 18 }
                                Text {
                                    Layout.fillWidth: true
                                    text: I18n.t("Здесь собраны штатные разделы OpenIPC WebUI и нативные операции firmware-client: status, network, time, logs, backup, reboot и update.")
                                    color: Theme.textMuted
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 11
                                }
                                Flow {
                                    Layout.fillWidth: true
                                    spacing: 8
                                    MajesticButton { text: I18n.t("WebUI"); primary: true; onClicked: openWebUiPath("") }
                                    MajesticButton { text: I18n.t("Network"); onClicked: openWebUiPath("/cgi-bin/fw-network.cgi") }
                                    MajesticButton { text: I18n.t("Time"); onClicked: openWebUiPath("/cgi-bin/fw-time.cgi") }
                                    MajesticButton { text: I18n.t("Update"); onClicked: openWebUiPath("/cgi-bin/fw-update.cgi") }
                                    MajesticButton { text: I18n.t("Settings"); onClicked: openWebUiPath("/cgi-bin/fw-settings.cgi") }
                                    MajesticButton { text: I18n.t("Firmware backup"); onClicked: firmwareBackupDialog.open() }
                                    MajesticButton { text: I18n.t("Reboot"); danger: true; onClicked: firmwareRebootConfirm.open() }
                                    MajesticButton { text: I18n.t("Скопировать адрес"); onClicked: copyControlCenterValue(I18n.t("Web UI камеры"), webUiUrl("")) }
                                }
                                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: Theme.panelBorder }
                                Text {
                                    Layout.fillWidth: true
                                    text: I18n.t("Write-операции используют те же CGI/JSON endpoints, что и WebUI камеры. Опасные действия требуют подтверждения.")
                                    color: Theme.warning
                                    wrapMode: Text.WordWrap
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }
            }

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 12
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text { text: I18n.t("Network"); color: Theme.accentHover; font.pixelSize: 28; font.bold: true }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Сводка сетевых сервисов камеры. Изменение IP/DHCP будет добавлено только с dry-run и явным подтверждением, чтобы не потерять камеру.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                            }
                        }
                        MajesticButton { text: I18n.t("Загрузить"); enabled: !firmwareBusy; onClicked: loadFirmwareNetwork() }
                        MajesticButton { text: I18n.t("Сохранить"); primary: true; enabled: !firmwareBusy; onClicked: firmwareNetworkConfirm.open() }
                        MajesticButton { text: I18n.t("Открыть Network WebUI"); onClicked: openWebUiPath("/cgi-bin/fw-network.cgi") }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.preferredHeight: 250
                        color: Theme.cardBackground
                        border.color: Theme.cardBorder
                        radius: Theme.radiusLg
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            RowLayout {
                                Layout.fillWidth: true
                                Text { Layout.fillWidth: true; text: I18n.t("Настройки сети OpenIPC"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 17 }
                                MajesticButton { text: I18n.t("Wi‑Fi scan"); enabled: !firmwareBusy; onClicked: scanFirmwareWifi() }
                                MajesticButton { text: I18n.t("Reset network"); danger: true; enabled: !firmwareBusy; onClicked: firmwareNetworkResetConfirm.open() }
                            }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: width > 900 ? 4 : 2
                                rowSpacing: 8
                                columnSpacing: 10
                                Text { text: I18n.t("Hostname"); color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: networkHostname; Layout.fillWidth: true; placeholderText: "openipc-camera" }
                                Text { text: I18n.t("Interface"); color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticComboBox { id: networkInterface; Layout.fillWidth: true; model: ["eth0", "wlan0"] }
                                MajesticCheckBox { id: networkDhcp; text: "DHCP"; Layout.columnSpan: 2 }
                                Text { text: I18n.t("IP address"); color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                                MajesticTextField { id: networkAddress; Layout.fillWidth: true; placeholderText: "192.168.0.219"; visible: !networkDhcp.checked }
                                Text { text: I18n.t("Netmask"); color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                                MajesticTextField { id: networkNetmask; Layout.fillWidth: true; placeholderText: "255.255.255.0"; visible: !networkDhcp.checked }
                                Text { text: I18n.t("Gateway"); color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                                MajesticTextField { id: networkGateway; Layout.fillWidth: true; placeholderText: "192.168.0.1"; visible: !networkDhcp.checked }
                                Text { text: "DNS"; color: Theme.textSecondary; font.pixelSize: 11; visible: !networkDhcp.checked }
                                MajesticTextField { id: networkDns; Layout.fillWidth: true; placeholderText: "1.1.1.1"; visible: !networkDhcp.checked }
                                Text { text: "Wi‑Fi SSID"; color: Theme.textSecondary; font.pixelSize: 11; visible: networkInterface.currentText === "wlan0" }
                                MajesticTextField { id: networkWlanSsid; Layout.fillWidth: true; visible: networkInterface.currentText === "wlan0" }
                                Text { text: I18n.t("Wi‑Fi password"); color: Theme.textSecondary; font.pixelSize: 11; visible: networkInterface.currentText === "wlan0" }
                                MajesticTextField { id: networkWlanPassword; Layout.fillWidth: true; echoMode: TextInput.Password; visible: networkInterface.currentText === "wlan0" }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: firmwareWifiNetworks.length
                                      ? I18n.t("Найдено Wi‑Fi сетей: %1", [firmwareWifiNetworks.length])
                                      : I18n.t("Сохранение сети может изменить IP камеры. Подтвердите действие перед отправкой.")
                                color: Theme.warning
                                wrapMode: Text.WordWrap
                                font.pixelSize: 11
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 2 : 1
                        rowSpacing: 12
                        columnSpacing: 12

                        Repeater {
                            model: networkServiceRows()
                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: 92
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder
                                radius: Theme.radiusLg
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 12
                                    Rectangle {
                                        Layout.preferredWidth: 82
                                        Layout.preferredHeight: 28
                                        radius: 14
                                        color: "#172554"
                                        border.color: Theme.accent
                                        Text { anchors.centerIn: parent; text: modelData.label; color: Theme.accentHover; font.bold: true; font.pixelSize: 11 }
                                    }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { Layout.fillWidth: true; text: modelData.value; color: Theme.textPrimary; font.family: "Consolas"; font.pixelSize: 13; elide: Text.ElideRight }
                                        Text { Layout.fillWidth: true; text: modelData.hint; color: Theme.textMuted; font.pixelSize: 11; elide: Text.ElideRight }
                                    }
                                    MajesticButton { text: I18n.t("Копировать"); subtle: true; onClicked: copyControlCenterValue(modelData.label, modelData.value) }
                                }
                            }
                        }
                    }
                }
            }

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 12
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text { text: I18n.t("Time"); color: Theme.accentHover; font.pixelSize: 28; font.bold: true }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Сравнение времени камеры и Dashboard. Корректное время важно для OSD, архива, событий и TLS.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                            }
                        }
                        MajesticButton { text: I18n.t("Загрузить"); enabled: !firmwareBusy; onClicked: loadFirmwareTime() }
                        MajesticButton { text: I18n.t("Сохранить"); primary: true; enabled: !firmwareBusy; onClicked: firmwareTimeConfirm.open() }
                        MajesticButton { text: I18n.t("Открыть Time WebUI"); onClicked: openWebUiPath("/cgi-bin/fw-time.cgi") }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.preferredHeight: 230
                        color: Theme.cardBackground
                        border.color: Theme.cardBorder
                        radius: Theme.radiusLg
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            RowLayout {
                                Layout.fillWidth: true
                                Text { Layout.fillWidth: true; text: I18n.t("Время и NTP OpenIPC"); color: Theme.textPrimary; font.bold: true; font.pixelSize: 17 }
                                MajesticButton { text: I18n.t("NTP sync"); enabled: !firmwareBusy; onClicked: syncFirmwareTime(false) }
                                MajesticButton { text: I18n.t("Set from PC"); enabled: !firmwareBusy; onClicked: syncFirmwareTime(true) }
                            }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: width > 900 ? 4 : 2
                                rowSpacing: 8
                                columnSpacing: 10
                                Text { text: I18n.t("Zone name"); color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: timeZoneName; Layout.fillWidth: true; placeholderText: "Asia/Vladivostok" }
                                Text { text: I18n.t("POSIX string"); color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: timeZoneData; Layout.fillWidth: true; placeholderText: "VLAT-10" }
                                Text { text: "NTP 1"; color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: timeServer0; Layout.fillWidth: true; placeholderText: "pool.ntp.org" }
                                Text { text: "NTP 2"; color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: timeServer1; Layout.fillWidth: true; placeholderText: "time.cloudflare.com" }
                                Text { text: "NTP 3"; color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: timeServer2; Layout.fillWidth: true }
                                Text { text: "NTP 4"; color: Theme.textSecondary; font.pixelSize: 11 }
                                MajesticTextField { id: timeServer3; Layout.fillWidth: true }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Изменение timezone пишет /etc/TZ и /etc/timezone; камера может запросить reboot для полного применения.")
                                color: Theme.warning
                                wrapMode: Text.WordWrap
                                font.pixelSize: 11
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        spacing: 10
                        Repeater {
                            model: timeRows()
                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: 82
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder
                                radius: Theme.radiusLg
                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 14
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: modelData.label; color: Theme.textPrimary; font.bold: true; font.pixelSize: 14 }
                                        Text { Layout.fillWidth: true; text: modelData.hint; color: Theme.textMuted; font.pixelSize: 11; elide: Text.ElideRight }
                                    }
                                    Text {
                                        Layout.preferredWidth: 260
                                        text: modelData.value
                                        color: Theme.accentHover
                                        font.family: "Consolas"
                                        font.pixelSize: 13
                                        horizontalAlignment: Text.AlignRight
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 12
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text { text: I18n.t("Firmware Update"); color: Theme.accentHover; font.pixelSize: 28; font.bold: true }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Прошивка — самый опасный раздел. Сейчас доступны безопасные входы и checklist; прямое обновление будет добавлено после проверки SoC/flash/image.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                            }
                        }
                        MajesticButton { text: I18n.t("Загрузить"); enabled: !firmwareBusy; onClicked: refreshFirmwareUpdateInfo() }
                        MajesticButton { text: I18n.t("Upload archive"); enabled: !firmwareBusy; onClicked: firmwareUploadDialog.open() }
                        MajesticButton { text: I18n.t("GitHub update"); danger: true; enabled: !firmwareBusy; onClicked: firmwareUpdateConfirm.open() }
                        MajesticButton { text: I18n.t("Открыть Update WebUI"); primary: true; onClicked: openWebUiPath("/cgi-bin/fw-update.cgi") }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.preferredHeight: 116
                        color: "#422006"
                        border.color: Theme.warning
                        radius: Theme.radiusLg
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 6
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Updater OpenIPC останавливает видео и перезагружает камеру. Не выключайте питание во время прошивки.")
                                color: "#fde68a"
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                                font.bold: true
                            }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Прямой upload архива уже пишет файл в /tmp/firmware.tgz. Финальная прошивка штатно идёт через /ws/upgrade; если модуль WebSockets недоступен, используйте кнопку WebUI.")
                                color: "#fde68a"
                                wrapMode: Text.WordWrap
                                font.pixelSize: 11
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 4 : 2
                        rowSpacing: 12
                        columnSpacing: 12
                        Repeater {
                            model: [
                                { title: I18n.t("Installed"), value: firmwareUpdateInfo.installed || "—" },
                                { title: I18n.t("Latest GitHub"), value: firmwareUpdateInfo.latest || "—" },
                                { title: "SoC", value: firmwareUpdateInfo.soc || "—" },
                                { title: I18n.t("Flash"), value: firmwareUpdateInfo.flash || "—" }
                            ]
                            delegate: MajesticStatusCard {
                                required property var modelData
                                title: modelData.title
                                value: modelData.value
                                subtitle: ""
                                percent: firmwareUpdateInfo.githubAvailable === true ? 100 : 40
                                accent: firmwareUpdateInfo.githubAvailable === true ? Theme.success : Theme.warning
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 2 : 1
                        rowSpacing: 12
                        columnSpacing: 12
                        Repeater {
                            model: updateChecklistRows()
                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: 112
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder
                                radius: Theme.radiusLg
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 6
                                    Text { text: modelData.title; color: Theme.textPrimary; font.bold: true; font.pixelSize: 16 }
                                    Text { Layout.fillWidth: true; text: modelData.text; color: Theme.textMuted; wrapMode: Text.WordWrap; font.pixelSize: 12 }
                                }
                            }
                        }
                    }
                }
            }

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        spacing: 12
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            Text { text: I18n.t("Tools"); color: Theme.accentHover; font.pixelSize: 28; font.bold: true }
                            Text {
                                Layout.fillWidth: true
                                text: I18n.t("Быстрые инструменты OpenIPC: штатные страницы камеры, диагностические входы и полезные справочники.")
                                color: Theme.textMuted
                                wrapMode: Text.WordWrap
                                font.pixelSize: 12
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: width > 900 ? 2 : 1
                        rowSpacing: 12
                        columnSpacing: 12
                        Repeater {
                            model: toolRows()
                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredHeight: 132
                                color: Theme.cardBackground
                                border.color: Theme.cardBorder
                                radius: Theme.radiusLg
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 8
                                    Text { text: modelData.title; color: Theme.textPrimary; font.bold: true; font.pixelSize: 16 }
                                    Text { Layout.fillWidth: true; text: modelData.text; color: Theme.textMuted; wrapMode: Text.WordWrap; font.pixelSize: 12 }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Item { Layout.fillWidth: true }
                                        MajesticButton {
                                            text: I18n.t("Открыть")
                                            primary: true
                                            onClicked: modelData.external ? Qt.openUrlExternally(modelData.path) : openWebUiPath(modelData.path)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ScrollView {
                clip: true; contentWidth: availableWidth
                ColumnLayout {
                    width: parent.width
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Полезные точки доступа Majestic для этой камеры")
                            color: Theme.textPrimary
                            font.pixelSize: 16
                            font.bold: true
                        }
                        MajesticButton { text: I18n.t("Обновить"); enabled: !dialog.loading; onClicked: dialog.refresh() }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        spacing: 10
                        MajesticEndpointProbeCard {
                            controller: dialog
                            title: I18n.t("HD RTSP")
                            state: dialog.rtspMainProbeState
                            message: dialog.rtspMainProbeMessage
                            elapsedMs: dialog.rtspMainProbeElapsedMs
                            buttonText: I18n.t("Проверить HD")
                            onRun: dialog.startEndpointProbe("main")
                        }
                        MajesticEndpointProbeCard {
                            controller: dialog
                            title: I18n.t("SD RTSP")
                            state: dialog.rtspSubProbeState
                            message: dialog.rtspSubProbeMessage
                            elapsedMs: dialog.rtspSubProbeElapsedMs
                            buttonText: I18n.t("Проверить SD")
                            onRun: dialog.startEndpointProbe("sub")
                        }
                        MajesticEndpointProbeCard {
                            controller: dialog
                            title: I18n.t("Majestic API")
                            state: dialog.majesticApiProbeState
                            message: dialog.majesticApiProbeMessage
                            elapsedMs: dialog.majesticApiProbeElapsedMs
                            buttonText: I18n.t("Проверить API")
                            onRun: dialog.startEndpointProbe("api")
                        }
                    }
                    Repeater {
                        model: dialog.endpointRows()
                        delegate: Rectangle {
                            id: endpointDelegate
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.leftMargin: 16
                            Layout.rightMargin: 16
                            Layout.preferredHeight: 82
                            color: Theme.cardBackground
                            border.color: Theme.cardBorder
                            radius: Theme.radiusLg
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 12
                                Rectangle {
                                    Layout.preferredWidth: 92
                                    Layout.preferredHeight: 26
                                    radius: 13
                                    color: "#172554"
                                    border.color: Theme.accent
                                    Text { anchors.centerIn: parent; text: endpointDelegate.modelData.group; color: Theme.accentHover; font.pixelSize: 11; font.bold: true }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text { text: endpointDelegate.modelData.name; color: Theme.textPrimary; font.bold: true; font.pixelSize: 12 }
                                    Text { Layout.fillWidth: true; text: endpointDelegate.modelData.value; color: Theme.textSecondary; font.family: "Consolas"; font.pixelSize: 11; elide: Text.ElideRight }
                                    Text { Layout.fillWidth: true; text: endpointDelegate.modelData.hint; color: Theme.textMuted; font.pixelSize: 10; elide: Text.ElideRight }
                                }
                                MajesticButton {
                                    text: I18n.t("Копировать")
                                    subtle: true
                                    onClicked: dialog.copyEndpoint(endpointDelegate.modelData)
                                }
                                MajesticButton {
                                    text: I18n.t("Открыть")
                                    enabled: endpointDelegate.modelData.openable === true
                                    onClicked: dialog.openEndpoint(endpointDelegate.modelData)
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true; Layout.margins: 12
                    Text { Layout.fillWidth: true; text: I18n.t("На камеру уйдёт только diff; неизвестные поля не удаляются."); color: Theme.textMuted }
                    MajesticButton { text: I18n.t("Форматировать"); onClicked: { var p = SystemController.majesticClient.parseJsonObject(rawEditor.text); if (p.ok) rawEditor.text = JSON.stringify(p.value, null, 2); else { statusError = true; statusText = p.error } } }
                    MajesticButton { text: I18n.t("Вернуть оригинал"); onClicked: rawEditor.text = JSON.stringify(originalConfig, null, 2) }
                    MajesticButton { text: I18n.t("Копировать без секретов"); onClicked: copyRedactedRawJson() }
                    MajesticButton { text: I18n.t("Проверить и применить"); primary: true; enabled: capabilities.configWrite === true && !loading; onClicked: { var p = SystemController.majesticClient.parseJsonObject(rawEditor.text); if (!p.ok) { statusError = true; statusText = I18n.t("Ошибка JSON: ") + p.error; return } prepareApply(p.value) } }
                }
                ScrollView {
                    Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 12; Layout.topMargin: 0
                    TextArea { id: rawEditor; text: "{}"; color: Theme.textSecondary; selectionColor: Theme.accent; selectedTextColor: Theme.textPrimary; font.family: "Consolas"; font.pixelSize: 12; wrapMode: TextEdit.NoWrap; background: Rectangle { color: Theme.controlBackground; border.color: Theme.controlBorder; radius: Theme.radiusMd } }
                }
            }

            ColumnLayout {
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true; Layout.margins: 12
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        Text {
                            text: I18n.t("Prometheus-метрики Majestic")
                            color: Theme.textPrimary
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Text {
                            Layout.fillWidth: true
                            text: I18n.t("Ключевые показатели encoder, sensor, streaming и runtime")
                            color: Theme.textMuted
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                    MajesticButton {
                        text: activeMetricsId.length ? I18n.t("Обновление…") : I18n.t("Обновить метрики")
                        primary: true
                        enabled: !activeMetricsId.length
                        onClicked: refreshMetrics()
                    }
                }

                GridLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    columns: width > 900 ? 4 : 2
                    rowSpacing: 10
                    columnSpacing: 10

                    Repeater {
                        model: metricsOverviewRows()
                        delegate: MajesticStatusCard {
                            required property var modelData
                            title: modelData.title
                            value: modelData.value
                            subtitle: modelData.subtitle
                            percent: modelData.percent
                            accent: modelData.accent
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    spacing: 8

                    Repeater {
                        model: metricsHealthRows()
                        delegate: Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.preferredHeight: 58
                            color: Theme.cardBackground
                            border.color: modelData.color
                            radius: Theme.radiusMd
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10
                                Rectangle {
                                    Layout.preferredWidth: 10
                                    Layout.preferredHeight: 10
                                    radius: 5
                                    color: modelData.color
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.title
                                        color: Theme.textPrimary
                                        font.bold: true
                                        font.pixelSize: 12
                                        elide: Text.ElideRight
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.text
                                        color: Theme.textMuted
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                    spacing: 10

                    TextField {
                        Layout.fillWidth: true
                        implicitHeight: 34
                        text: metricsFilterText
                        placeholderText: I18n.t("Фильтр raw-метрик…")
                        color: Theme.textPrimary
                        placeholderTextColor: Theme.textMuted
                        selectionColor: Theme.accent
                        selectedTextColor: Theme.textPrimary
                        leftPadding: 12
                        rightPadding: 12
                        background: Rectangle {
                            color: Theme.controlBackground
                            radius: Theme.radiusMd
                            border.color: parent.activeFocus ? Theme.accent : Theme.controlBorder
                        }
                        onTextChanged: metricsFilterText = text
                    }
                    Text {
                        text: I18n.t("Raw Prometheus")
                        color: Theme.textMuted
                        font.pixelSize: 12
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 12
                    Layout.topMargin: 0
                    TextArea {
                        text: filteredMetricsText()
                        readOnly: true
                        color: Theme.textSecondary
                        font.family: "Consolas"
                        font.pixelSize: 11
                        wrapMode: TextEdit.NoWrap
                        background: Rectangle {
                            color: Theme.controlBackground
                            border.color: Theme.controlBorder
                            radius: Theme.radiusMd
                        }
                    }
                }
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
        id: applyWatchdogTimer
        interval: 25000
        repeat: false
        onTriggered: {
            if (!activeApplyId.length) return
            loading = false
            statusError = true
            statusText = I18n.t("Majestic не ответил на сохранение за %1 секунд. Проверьте доступность камеры и повторите попытку.", [Math.round(interval / 1000)])
            untrack(activeApplyId)
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
            reloadRefreshTimer.restart()
        }
    }

    Connections {
        target: SystemController.majesticClient
        function onConfigurationLoaded(requestId, config, schema, loadedFields, loadedCapabilities) {
            if (requestId !== activeLoadId) return
            loading = false; originalConfig = clone(config); currentSchema = clone(schema); fields = loadedFields; capabilities = loadedCapabilities
            rawEditor.text = JSON.stringify(config, null, 2); resetDraft(); statusError = false
            statusText = pipelineReloadNeeded
                         ? I18n.t("Конфигурация сохранена; примените reload pipeline")
                         : (loadedCapabilities.schema ? I18n.t("Majestic подключён, schema загружена") : I18n.t("Majestic подключён в legacy-режиме"))
            if (loadedCapabilities.metrics === true) refreshMetrics()
        }
        function onConfigurationApplied(requestId) {
            if (requestId !== activeApplyId) return
            applyWatchdogTimer.stop()
            untrack(activeApplyId)
            activeApplyId = ""
            pipelineReloadNeeded = pendingPipelineReloadNeeded
            if (pipelineReloadNeeded && pendingAutoReloadAfterApply && capabilities.pipelineReload === true) {
                triggerPipelineReload("Конфигурация сохранена; отправляю reload pipeline…")
                return
            }
            statusText = pipelineReloadNeeded
                         ? I18n.t("Конфигурация сохранена; примените reload pipeline")
                         : I18n.t("Конфигурация сохранена; live-параметры применены")
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
            rawEditor.text = JSON.stringify(config, null, 2); tabs.currentIndex = tabIndexForKind("raw"); statusError = false; statusText = I18n.t("Backup открыт для проверки: %1", [path])
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
                reloadRefreshTimer.restart()
            }
            else statusText = I18n.t("Операция выполнена: %1", [operation])
        }
        function onOperationFailed(requestId, operation, message, httpStatus) {
            if (!owns(requestId) && requestId !== activeLoadId && requestId !== activeApplyId) return
            if (operation === "apply-config" || requestId === activeApplyId) {
                applyWatchdogTimer.stop()
                untrack(activeApplyId)
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
            if (firmwareWifiNetworks.length && !networkWlanSsid.text.length) networkWlanSsid.text = firmwareWifiNetworks[0].ssid || ""
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
            firmwareLogsText = text
            statusError = false
            statusText = I18n.t("Логи загружены: %1", [source])
        }

        function onBackupSaved(requestId, path) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            activeFirmwareBackupId = ""
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
            statusError = false
            statusText = I18n.t("Firmware archive загружен: %1", [remotePath])
        }

        function onOperationFailed(requestId, operation, message, httpStatus) {
            if (!owns(requestId)) return
            untrack(requestId)
            firmwareBusy = false
            if (requestId === activeFirmwareStatusId) activeFirmwareStatusId = ""
            if (requestId === activeFirmwareNetworkId) activeFirmwareNetworkId = ""
            if (requestId === activeFirmwareNetworkSaveId) activeFirmwareNetworkSaveId = ""
            if (requestId === activeFirmwareTimeId) activeFirmwareTimeId = ""
            if (requestId === activeFirmwareTimeSaveId) activeFirmwareTimeSaveId = ""
            if (requestId === activeFirmwareLogsId) activeFirmwareLogsId = ""
            if (requestId === activeFirmwareBackupId) activeFirmwareBackupId = ""
            if (requestId === activeFirmwareRebootId) activeFirmwareRebootId = ""
            if (requestId === activeFirmwareUpdateId) activeFirmwareUpdateId = ""
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

    Dialog {
        id: firmwareNetworkConfirm
        modal: true
        anchors.centerIn: parent
        width: Math.min(dialog.width - 100, 560)
        title: I18n.t("Подтвердить изменение сети")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: saveFirmwareNetwork()
        contentItem: Label {
            text: I18n.t("Сетевые настройки будут записаны в OpenIPC. Если IP/DHCP указан неверно, камера может стать недоступной до ручного восстановления. Продолжить?")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            padding: 16
        }
    }

    Dialog {
        id: firmwareNetworkResetConfirm
        modal: true
        anchors.centerIn: parent
        width: Math.min(dialog.width - 100, 560)
        title: I18n.t("Сбросить сетевую конфигурацию")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: resetFirmwareNetwork()
        contentItem: Label {
            text: I18n.t("Будет восстановлена network-конфигурация из прошивки. Все текущие изменения сети будут потеряны.")
            color: Theme.warning
            wrapMode: Text.WordWrap
            padding: 16
        }
    }

    Dialog {
        id: firmwareTimeConfirm
        modal: true
        anchors.centerIn: parent
        width: Math.min(dialog.width - 100, 560)
        title: I18n.t("Сохранить время и NTP")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: saveFirmwareTime()
        contentItem: Label {
            text: I18n.t("Настройки timezone и NTP будут записаны в /etc/TZ, /etc/timezone и /etc/ntp.conf на камере.")
            color: Theme.textSecondary
            wrapMode: Text.WordWrap
            padding: 16
        }
    }

    Dialog {
        id: firmwareRebootConfirm
        modal: true
        anchors.centerIn: parent
        width: Math.min(dialog.width - 100, 560)
        title: I18n.t("Перезагрузить камеру")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: requestFirmwareReboot()
        contentItem: Label {
            text: I18n.t("Камера будет перезагружена через штатный fw-restart.cgi. Видео и WebUI временно пропадут. Продолжить?")
            color: Theme.warning
            wrapMode: Text.WordWrap
            padding: 16
        }
    }

    Dialog {
        id: firmwareUpdateConfirm
        modal: true
        anchors.centerIn: parent
        width: Math.min(dialog.width - 100, 620)
        title: I18n.t("Запустить обновление прошивки")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: startGithubFirmwareUpdate()
        contentItem: Label {
            text: I18n.t("Update — опасная операция. Камера остановит видео и перезагрузится. Сейчас приложение запускает только безопасно проверенный путь; если WebSockets updater недоступен в этой сборке, откройте штатный WebUI Update.")
            color: Theme.warning
            wrapMode: Text.WordWrap
            padding: 16
        }
    }

    FileDialog {
        id: snapshotDialog; title: I18n.t("Сохранить снимок Majestic"); fileMode: FileDialog.SaveFile; defaultSuffix: "jpg"
        nameFilters: [I18n.t("JPEG (*.jpg *.jpeg)"), I18n.t("Все файлы (*)")]
        onAccepted: track(SystemController.majesticClient.takeSnapshot(cameraHost, cameraPort, cameraUser, cameraPassword, selectedFile.toString(), snapshotWidth.value, snapshotHeight.value, snapshotQuality.value, snapshotGray.checked))
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
