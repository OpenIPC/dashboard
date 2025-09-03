// hls-loader.js
// Dynamically loads hls.js if needed and attaches HLS stream to a video element

function loadHlsJs(callback) {
    if (window.Hls) {
        callback(window.Hls);
        return;
    }
    const script = document.createElement('script');
    script.src = 'https://cdn.jsdelivr.net/npm/hls.js@latest';
    script.onload = () => callback(window.Hls);
    document.head.appendChild(script);
}

function attachHlsStream(videoElement, streamUrl) {
    loadHlsJs(Hls => {
        if (Hls.isSupported()) {
            const hls = new Hls();
            hls.loadSource(streamUrl);
            hls.attachMedia(videoElement);
        } else if (videoElement.canPlayType('application/vnd.apple.mpegurl')) {
            videoElement.src = streamUrl;
        } else {
            videoElement.poster = '';
            videoElement.innerHTML = '<span>HLS not supported on this device.</span>';
        }
    });
}

window.HlsLoader = { attachHlsStream };
