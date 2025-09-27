// webrtc-loader.js
// Dynamically attaches WebRTC stream to a video element using MediaMTX WHEP

async function attachWebRTCStream(videoElement, streamPath) {
    // Clear any existing srcObject
    if (videoElement.srcObject) {
        videoElement.srcObject.getTracks().forEach(track => track.stop());
        videoElement.srcObject = null;
    }

    try {
        const pc = new RTCPeerConnection();

        pc.addEventListener('track', (event) => {
            if (event.streams && event.streams[0]) {
                videoElement.srcObject = event.streams[0];
            }
        });

        pc.addEventListener('connectionstatechange', () => {
            console.log(`[WebRTC] Connection state: ${pc.connectionState}`);
            if (pc.connectionState === 'failed' || pc.connectionState === 'disconnected') {
                console.warn('[WebRTC] Connection failed, stopping tracks');
                if (videoElement.srcObject) {
                    videoElement.srcObject.getTracks().forEach(track => track.stop());
                    videoElement.srcObject = null;
                }
            }
        });

        // Create offer
        const offer = await pc.createOffer();
        await pc.setLocalDescription(offer);

        // Send offer to MediaMTX WHEP endpoint
        const whepUrl = `http://127.0.0.1:8889/${streamPath}/whep`;
        const response = await fetch(whepUrl, {
            method: 'POST',
            headers: { 'Content-Type': 'application/sdp' },
            body: pc.localDescription.sdp
        });

        if (!response.ok) {
            throw new Error(`WHEP request failed: ${response.status} ${response.statusText}`);
        }

        const answerSdp = await response.text();
        await pc.setRemoteDescription({ type: 'answer', sdp: answerSdp });

        console.log(`[WebRTC] Attached stream: ${streamPath}`);
    } catch (error) {
        console.error('[WebRTC] Failed to attach stream:', error);
        videoElement.innerHTML = '<span style="color: #ff6b6b;">WebRTC not available</span>';
    }
}

window.WebRTCLoader = { attachWebRTCStream };