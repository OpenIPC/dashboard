// --- START OF FILE js/grid-manager.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    window.AppModules.createGridManager = function(App) {
        const stateManager = App.stateManager;
        const gridContainer = document.getElementById('grid-container');
        const layoutControls = document.getElementById('layout-controls');
        const MAX_GRID_SIZE = 64;

        let gridCells = [];
        let fullscreenCellIndex = null;
        let isRendering = false;
        const analyticsState = {};
        let animationFrameId = null;

        const peerConnections = {};
        const streamStatsHistory = {};
        let statsUpdateInterval = null;

    // Toggleable debug overlay. Set to true for visual debug markers in overlays.
    const ANALYTICS_DEBUG_DRAW = false; // отключено для production, убраны debug-оверлеи

    function startRenderLoop() {
            if (animationFrameId) {
                cancelAnimationFrame(animationFrameId);
            }
            function renderAnalytics() {
                const now = Date.now();
                const gridState = getGridState();
                if (!gridState) {
                    animationFrameId = requestAnimationFrame(renderAnalytics);
                    return;
                }
                for (const cell of gridCells) {
                    const cellId = parseInt(cell.dataset.cellId, 10);
                    const cellState = gridState[cellId];
                    const cameraId = cellState?.camera?.id;
                    if (!cameraId || !analyticsState[cameraId] || (now - analyticsState[cameraId].timestamp > 2000)) {
                         const overlayCanvas = cell.querySelector('.overlay-canvas');
                         if (overlayCanvas) {
                    // Для каждой ячейки храним два PeerConnection и два MediaStream: SD и HD
                    const mediaStreams = {};    // { cellIndex: { 0: streamHD, 1: streamSD } }
                             const ctx = overlayCanvas.getContext('2d');
                             ctx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
                         }
                         if(analyticsState[cameraId]) {
                            delete analyticsState[cameraId];
                         }
                    }
                }
                for (const cameraId in analyticsState) {
                    const state = analyticsState[cameraId];
                    const cellsForCamera = gridState.map((s, i) => ({s, i})).filter(item => item.s && item.s.camera.id == cameraId);
                    for (const cellInfo of cellsForCamera) {
                        const cellElement = gridCells[cellInfo.i];
                        if (!cellElement) continue;
                        let analyticsImg = cellElement.querySelector('.analytics-img');
                        // Ensure overlay canvas exists for drawing boxes (always create)
                        // We'll only show the full-frame analytics image when the cell is fullscreen
                        // or when explicitly forced via DevTools to avoid covering SD preview.
                        let shouldShowImageOverlay = !!window.__analytics_forceImageOverlay;
                        try {
                            const gridStateForCells = getGridState();
                            const gridCellState = gridStateForCells[cellInfo.i] || {};
                            // If this cell is using HD stream (fullscreen behavior is handled elsewhere), allow image overlay
                            if (gridCellState.streamId === 0) shouldShowImageOverlay = true;
                        } catch (e) { /* ignore */ }

                        if (!analyticsImg) {
                            analyticsImg = document.createElement('img');
                            analyticsImg.className = 'analytics-img';
                            analyticsImg.style.position = 'absolute';
                            analyticsImg.style.top = '0';
                            analyticsImg.style.left = '0';
                            analyticsImg.style.width = '100%';
                            analyticsImg.style.height = '100%';
                            analyticsImg.style.objectFit = 'contain';
                            analyticsImg.style.zIndex = '10';
                            analyticsImg.style.pointerEvents = 'none';
                            // start hidden; we'll enable when allowed
                            analyticsImg.style.display = 'none';
                            cellElement.appendChild(analyticsImg);
                        }

                        // Only assign the full-frame image source when we intend to show the image overlay.
                        if (shouldShowImageOverlay && state.frame_base64) {
                            analyticsImg.src = state.frame_base64;
                            analyticsImg.style.display = '';
                        } else {
                            // hide to avoid covering the native video element (prevents jerkiness)
                            analyticsImg.style.display = 'none';
                        }

                        // Draw bounding boxes for analytics objects on overlay canvas
                        // Declare overlayCanvas in outer scope so it can be referenced after the try block
                        let overlayCanvas = cellElement.querySelector('.overlay-canvas');
                        // If canvas is missing, create a minimal overlay canvas so drawing can proceed
                        if (!overlayCanvas) {
                            try {
                                const wrapper = cellElement.querySelector('.video-wrapper') || cellElement;
                                overlayCanvas = document.createElement('canvas');
                                overlayCanvas.className = 'overlay-canvas';
                                overlayCanvas.style.position = 'absolute';
                                overlayCanvas.style.top = '0';
                                overlayCanvas.style.left = '0';
                                overlayCanvas.style.width = '100%';
                                overlayCanvas.style.height = '100%';
                                overlayCanvas.style.zIndex = '15';
                                overlayCanvas.style.pointerEvents = 'none';
                                wrapper.appendChild(overlayCanvas);
                                // Record that we've ensured overlay for this camera (used for diagnostics)
                                window.__analytics_diag = window.__analytics_diag || {};
                                window.__analytics_diag[cameraId] = window.__analytics_diag[cameraId] || {};
                                window.__analytics_diag[cameraId].overlayEnsured = true;
                            } catch (e) { /* best-effort, ignore errors creating canvas */ }
                        }
                        try {
                            if (overlayCanvas) {
                                const dpr = window.devicePixelRatio || 1;

                                // Use the video-wrapper as the reference area for drawing (it is the element with object-fit behavior)
                                const wrapperEl = cellElement.querySelector('.video-wrapper') || cellElement;
                                const wrapperRect = wrapperEl.getBoundingClientRect();

                                // Use the wrapper (the object-fit container) as the authoritative reference
                                // The video element can have different bounding rect (controls, transforms), so prefer wrapper
                                const videoRef = cellElement.querySelector('.video-player');
                                const refRect = wrapperRect;

                                // Determine drawing surface CSS size using the overlay canvas bounding rect when possible
                                // If overlayCanvas has been pinned to fixed position for fullscreen, use viewport sizes
                                const canvasRect = overlayCanvas.getBoundingClientRect ? overlayCanvas.getBoundingClientRect() : null;
                                const pinnedFixed = (overlayCanvas.style && overlayCanvas.style.position === 'fixed');
                                const cssWidth = pinnedFixed ? window.innerWidth : ((canvasRect && canvasRect.width) || overlayCanvas.clientWidth || wrapperRect.width || Math.round(overlayCanvas.width / dpr));
                                const cssHeight = pinnedFixed ? window.innerHeight : ((canvasRect && canvasRect.height) || overlayCanvas.clientHeight || wrapperRect.height || Math.round(overlayCanvas.height / dpr));

                                // Resize canvas backing store to match display size * DPR
                                overlayCanvas.width = Math.round(cssWidth * dpr);
                                overlayCanvas.height = Math.round(cssHeight * dpr);
                                overlayCanvas.style.width = cssWidth + 'px';
                                overlayCanvas.style.height = cssHeight + 'px';

                                const ctx = overlayCanvas.getContext('2d');
                                // reset transform and scale for DPR
                                ctx.setTransform(1, 0, 0, 1, 0, 0);
                                ctx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
                                ctx.scale(dpr, dpr);

                                // Resolve frame width/height with priorities:
                                // 1) analytics provided (state.frame_width/height)
                                // 2) previously inferred and stored in state (state.inferredFrameW/H)
                                // 3) analytics image natural size (analyticsImg.naturalWidth/Height)
                                // 4) extents from current objects (bbox inferredW/inferredH)
                                // 5) video's intrinsic resolution (videoEl.videoWidth/videoEl.videoHeight)
                                // 6) fallback to CSS wrapper size
                                let inferredW = 0;
                                let inferredH = 0;
                                if (state.objects && state.objects.length > 0) {
                                    for (const o of state.objects) {
                                        if (!o.box) continue;
                                        const bx = (typeof o.box.x === 'number') ? o.box.x : 0;
                                        const by = (typeof o.box.y === 'number') ? o.box.y : 0;
                                        const bw = (typeof o.box.w === 'number') ? o.box.w : 0;
                                        const bh = (typeof o.box.h === 'number') ? o.box.h : 0;
                                        inferredW = Math.max(inferredW, bx + bw);
                                        inferredH = Math.max(inferredH, by + bh);
                                    }
                                }

                                const videoEl = videoRef; // reuse previously queried element


                                // Determine current streamId for this cell (0=HD,1=SD default)
                                const gridStateForCells = getGridState();
                                const gridCellState = gridStateForCells[cellInfo.i] || {};
                                const activeStreamId = (typeof gridCellState.streamId === 'number') ? gridCellState.streamId : 1;

                                // Prefer analytics-provided values
                                let frameW = state.frame_width || 0;
                                let frameH = state.frame_height || 0;

                                // Fallbacks: use inferred frame extents from objects, or analytics image natural size, or video intrinsic size
                                if (!frameW || !frameH) {
                                    // Prefer per-stream inferred sizes (HD/SD) when available
                                    if (state.inferredFrameByStream && state.inferredFrameByStream[activeStreamId]) {
                                        frameW = state.inferredFrameByStream[activeStreamId].w;
                                        frameH = state.inferredFrameByStream[activeStreamId].h;
                                        // [analytics-draw] log removed
                                    } else if (state.inferredFrameW && state.inferredFrameH) {
                                        frameW = state.inferredFrameW;
                                        frameH = state.inferredFrameH;
                                        // [analytics-draw] log removed
                                    } else if (analyticsImg && analyticsImg.naturalWidth && analyticsImg.naturalHeight) {
                                        frameW = analyticsImg.naturalWidth;
                                        frameH = analyticsImg.naturalHeight;
                                        // [analytics-draw] log removed
                                    } else if (videoEl && videoEl.videoWidth && videoEl.videoHeight) {
                                        frameW = videoEl.videoWidth;
                                        frameH = videoEl.videoHeight;
                                        // [analytics-draw] log removed
                                    } else {
                                        // If we still don't have sizes, warn but continue (will clamp coords)

                                        // As a last resort, assume css dimensions match frame
                                        frameW = cssWidth;
                                        frameH = cssHeight;
                                    }
                                }

                                    // Adaptive override: if our inferred extents from object bboxes are substantially
                                    // larger than the chosen frame size, prefer the inferred extents. This covers
                                    // cases where analytics reports absolute pixel bboxes from a higher-resolution
                                    // source (HD) while we are displaying a scaled SD player.
                                    try {
                                        if (inferredW > 0 && inferredH > 0 && frameW > 0 && frameH > 0) {
                                            const ratioW = inferredW / frameW;
                                            const ratioH = inferredH / frameH;
                                            // If inferred extents exceed current frame by >1.2x in either axis, adopt them
                                            if (Math.max(ratioW, ratioH) > 1.2) {
                                                // [analytics-diag] log removed
                                                frameW = inferredW;
                                                frameH = inferredH;
                                            }
                                        }
                                    } catch (e) { /* ignore */ }

                                // Runtime test override: allow forcing frame size from DevTools for diagnosis
                                // Example: window.__analytics_forceFrame = { w: 2560, h: 970 }
                                const forcedFrame = (window.__analytics_forceFrame && typeof window.__analytics_forceFrame === 'object') ? window.__analytics_forceFrame : null;

                                // If analytics provides normalized coordinates (<=1) or pixel coords, detect both
                                const isNormalized = (() => {
                                    if (!state.objects || state.objects.length === 0) return false;
                                    for (const o of state.objects) {
                                        if (!o.box) continue;
                                        if (o.box.x <= 1 && o.box.y <= 1 && o.box.w <= 1 && o.box.h <= 1) return true;
                                    }
                                    return false;
                                })();

                                if (state.objects && state.objects.length > 0 && frameW > 0 && frameH > 0) {
                                    // Apply runtime override for frame size if provided (useful for testing mismatched source resolution)
                                    if (forcedFrame && forcedFrame.w && forcedFrame.h) {
                                        frameW = forcedFrame.w;
                                        frameH = forcedFrame.h;
                                    }

                                    // Before deciding whether to use the analytics full-frame image as the
                                    // display source, compute an enhanced guard: only show the image overlay
                                    // if the cell is fullscreen, forced by DevTools, or the analytics
                                    // frame resolution approximately matches the currently displayed video.
                                    // This avoids overlaying a high-resolution HD analytics image on top of
                                    // a lower-resolution SD player and causing visible jerkiness.
                                    try {
                                        const forced = !!window.__analytics_forceImageOverlay;
                                        const isFullscreenCell = cellElement.classList && cellElement.classList.contains && cellElement.classList.contains('fullscreen');
                                        let showBasedOnResolution = false;
                                        if (videoEl && videoEl.videoWidth && videoEl.videoHeight && frameW && frameH) {
                                            const diffW = Math.abs(videoEl.videoWidth - frameW);
                                            const diffH = Math.abs(videoEl.videoHeight - frameH);
                                            const tolW = Math.max(50, Math.round(frameW * 0.2));
                                            const tolH = Math.max(50, Math.round(frameH * 0.2));
                                            showBasedOnResolution = (diffW <= tolW && diffH <= tolH);
                                        }
                                        const shouldShowImageOverlayFinal = forced || isFullscreenCell || showBasedOnResolution;
                                        if (analyticsImg) {
                                            if (shouldShowImageOverlayFinal && state.frame_base64) {
                                                analyticsImg.src = state.frame_base64;
                                                analyticsImg.style.display = '';
                                            } else {
                                                analyticsImg.style.display = 'none';
                                            }
                                        }
                                    } catch (e) { /* ignore */ }

                                    // Determine the exact displayed image rectangle (prefer actual DOM bounding rect)
                                    // This avoids small rounding/positioning differences when using math-only scale.
                                    let displayW = 0, displayH = 0, offsetX = 0, offsetY = 0, scale = 1;
                                    try {
                                        const canvasRectForOffsets = overlayCanvas.getBoundingClientRect();
                                        // If an analytics image element is present and visible, use its actual displayed rect
                                        if (analyticsImg && analyticsImg.style.display !== 'none') {
                                            const imgRect = analyticsImg.getBoundingClientRect();
                                            // If canvas is pinned fixed, compute offsets relative to viewport
                                            displayW = imgRect.width;
                                            displayH = imgRect.height;
                                            if (pinnedFixed) {
                                                offsetX = imgRect.left; // viewport coords
                                                offsetY = imgRect.top;
                                            } else {
                                                offsetX = imgRect.left - canvasRectForOffsets.left;
                                                offsetY = imgRect.top - canvasRectForOffsets.top;
                                            }
                                            // compute effective scale for diagnostics
                                            scale = displayW / frameW;
                                        } else if (videoEl && typeof videoEl.getBoundingClientRect === 'function') {
                                            // If video element displays the stream, use its bounding rect
                                            const vidRect = videoEl.getBoundingClientRect();
                                            displayW = vidRect.width;
                                            displayH = vidRect.height;
                                            if (pinnedFixed) {
                                                offsetX = vidRect.left;
                                                offsetY = vidRect.top;
                                            } else {
                                                offsetX = vidRect.left - canvasRectForOffsets.left;
                                                offsetY = vidRect.top - canvasRectForOffsets.top;
                                            }
                                            scale = displayW / frameW;
                                        } else {
                                            // Fallback: math-based object-fit: contain calculation inside the overlay canvas
                                            scale = Math.min(cssWidth / frameW, cssHeight / frameH);
                                            displayW = frameW * scale;
                                            displayH = frameH * scale;
                                            offsetX = (cssWidth - displayW) / 2;
                                            offsetY = (cssHeight - displayH) / 2;
                                            // If pinned fixed, offsets are already viewport-relative; use as-is
                                        }
                                    } catch (e) {
                                        // On any error, fallback to math-based calculation
                                        scale = Math.min(cssWidth / frameW, cssHeight / frameH);
                                        displayW = frameW * scale;
                                        displayH = frameH * scale;
                                        offsetX = (cssWidth - displayW) / 2;
                                        offsetY = (cssHeight - displayH) / 2;
                                    }

                                    // Diagnostic logging once per camera to help debug sizing mismatches. Also periodically log inferred sizes
                                    window.__analytics_diag = window.__analytics_diag || {};
                                    if (!window.__analytics_diag[cameraId]) {
                                        // [analytics-diag] log removed
                                        window.__analytics_diag[cameraId] = { lastLog: Date.now() };
                                    } else if (Date.now() - (window.__analytics_diag[cameraId].lastLog || 0) > 5000) {
                                        // log every ~5s to avoid spamming
                                        // [analytics-diag] log removed
                                        window.__analytics_diag[cameraId].lastLog = Date.now();
                                    }

                                    // Heuristic: detect whether analytics boxes use (x,y) as top-left or as center
                                    let useCenterCoords = false;
                                    try {
                                        if (!isNormalized && state.objects && state.objects.length > 0 && frameW > 0 && frameH > 0) {
                                            // Compute penalty as total area of boxes that lie outside the frame for each interpretation.
                                            let penaltyTL = 0;
                                            let penaltyC = 0;
                                            for (const o of state.objects) {
                                                if (!o || !o.box) continue;
                                                const bx = (typeof o.box.x === 'number') ? o.box.x : 0;
                                                const by = (typeof o.box.y === 'number') ? o.box.y : 0;
                                                const bw = (typeof o.box.w === 'number') ? Math.max(0, o.box.w) : 0;
                                                const bh = (typeof o.box.h === 'number') ? Math.max(0, o.box.h) : 0;

                                                // top-left interpretation box
                                                const tlx = bx;
                                                const tly = by;
                                                const tlx2 = bx + bw;
                                                const tly2 = by + bh;

                                                // center interpretation box
                                                const cx = bx - bw / 2;
                                                const cy = by - bh / 2;
                                                const cx2 = cx + bw;
                                                const cy2 = cy + bh;

                                                // helper to compute out-of-bounds area for a rect
                                                const areaOutside = (x1, y1, x2, y2) => {
                                                    // intersection with frame
                                                    const ix1 = Math.max(0, x1);
                                                    const iy1 = Math.max(0, y1);
                                                    const ix2 = Math.min(frameW, x2);
                                                    const iy2 = Math.min(frameH, y2);
                                                    const interW = Math.max(0, ix2 - ix1);
                                                    const interH = Math.max(0, iy2 - iy1);
                                                    const interArea = interW * interH;
                                                    const totalArea = Math.max(0, x2 - x1) * Math.max(0, y2 - y1);
                                                    return Math.max(0, totalArea - interArea);
                                                };

                                                penaltyTL += areaOutside(tlx, tly, tlx2, tly2);
                                                penaltyC += areaOutside(cx, cy, cx2, cy2);
                                            }

                                            // Prefer interpretation with smaller penalty. If penalties equal, prefer top-left (more common).
                                            useCenterCoords = (penaltyC + 1e-9) < penaltyTL;
                                            // Log penalties for diagnostics
                                            console.info('[analytics-diag] coord-penalty', { cameraId, penaltyTL: Math.round(penaltyTL), penaltyC: Math.round(penaltyC), choose: useCenterCoords ? 'center' : 'topleft' });
                                        }
                                    } catch (e) {

                                    }
                                    // Log chosen coord format at most once per camera periodically
                                    if (!window.__analytics_diag[cameraId]) window.__analytics_diag[cameraId] = {};
                                    if (!window.__analytics_diag[cameraId].coordFormatLogged) {
                                        console.info('[analytics-diag]', { cameraId, coordFormat: useCenterCoords ? 'center' : 'topleft' });
                                        window.__analytics_diag[cameraId].coordFormatLogged = true;
                                    }

                                    // Color map for labels (deterministic hash)
                                    const colorFor = (label) => {
                                        const colours = ['#e6194b','#3cb44b','#ffe119','#4363d8','#f58231','#911eb4','#46f0f0','#f032e6','#bcf60c','#fabebe'];
                                        let h = 0;
                                        for (let i = 0; i < label.length; i++) h = (h << 5) - h + label.charCodeAt(i);
                                        return colours[Math.abs(h) % colours.length];
                                    };

                                    ctx.lineWidth = 4;
                                    ctx.font = '12px sans-serif';
                                    ctx.textBaseline = 'top';

                                    // Reset lastBoxes diagnostic for this frame
                                    try {
                                        window.__analytics_diag = window.__analytics_diag || {};
                                        window.__analytics_diag[cameraId] = window.__analytics_diag[cameraId] || {};
                                        window.__analytics_diag[cameraId].lastBoxes = [];
                                    } catch (e) { /* ignore */ }

                                    // Simple NMS to remove overlapping/duplicate boxes before drawing
                                    const objectsToDraw = (() => {
                                        try {
                                            const src = (state.objects || []).slice();
                                            // normalize numeric confidence (fallback to 0)
                                            src.forEach(s => { s._conf = (typeof s.confidence === 'number') ? s.confidence : ((typeof s.score === 'number') ? s.score : 0); });
                                            // sort by confidence desc
                                            src.sort((a, b) => (b._conf || 0) - (a._conf || 0));
                                            const kept = [];
                                            const iou = (a, b) => {
                                                if (!a.box || !b.box) return 0;
                                                const ax1 = a.box.x;
                                                const ay1 = a.box.y;
                                                const ax2 = a.box.x + a.box.w;
                                                const ay2 = a.box.y + a.box.h;
                                                const bx1 = b.box.x;
                                                const by1 = b.box.y;
                                                const bx2 = b.box.x + b.box.w;
                                                const by2 = b.box.y + b.box.h;
                                                const ix1 = Math.max(ax1, bx1);
                                                const iy1 = Math.max(ay1, by1);
                                                const ix2 = Math.min(ax2, bx2);
                                                const iy2 = Math.min(ay2, by2);
                                                const iw = Math.max(0, ix2 - ix1);
                                                const ih = Math.max(0, iy2 - iy1);
                                                const inter = iw * ih;
                                                const areaA = Math.max(0, ax2 - ax1) * Math.max(0, ay2 - ay1);
                                                const areaB = Math.max(0, bx2 - bx1) * Math.max(0, by2 - by1);
                                                const union = areaA + areaB - inter;
                                                return union <= 0 ? 0 : inter / union;
                                            };
                                            for (const cand of src) {
                                                let dup = false;
                                                for (const k of kept) {
                                                    if (iou(cand, k) > 0.45) { dup = true; break; }
                                                }
                                                if (!dup) kept.push(cand);
                                            }
                                            return kept;
                                        } catch (e) {
                                            return state.objects || [];
                                        }
                                    })();

                                    // --- Simple tracking / smoothing (Variant C) ---
                                    // Maintain lightweight tracks in analyticsState[cameraId].tracks
                                    const nowMs = Date.now();
                                    analyticsState[cameraId] = analyticsState[cameraId] || {};
                                    const tracker = analyticsState[cameraId].tracker = analyticsState[cameraId].tracker || { nextId: 1, tracks: [] };

                                    // Per-camera and global renderer-side filtering and stability settings.
                                    // Resolve settings in this order: per-camera overrides -> global window overrides -> defaults.
                                    const cameraSettings = (window.__analytics_camera_settings && window.__analytics_camera_settings[cameraId]) ? window.__analytics_camera_settings[cameraId] : {};
                                    const DEFAULT_MIN_CONFIDENCE = 0.6;
                                    const DEFAULT_MIN_AREA_PX = 400; // ~20x20
                                    const DEFAULT_MIN_NORM_AREA = 0.001;
                                    const DEFAULT_STABILITY_FRAMES = 2; // require seen in 2 frames before confirming

                                    const minConf = (typeof cameraSettings.min_confidence === 'number') ? cameraSettings.min_confidence : ((typeof window.__analytics_min_confidence === 'number') ? window.__analytics_min_confidence : DEFAULT_MIN_CONFIDENCE);
                                    const minAreaPx = (typeof cameraSettings.min_area_px === 'number') ? cameraSettings.min_area_px : ((typeof window.__analytics_min_area_px === 'number') ? window.__analytics_min_area_px : DEFAULT_MIN_AREA_PX);
                                    const minNormArea = (typeof cameraSettings.min_norm_area === 'number') ? cameraSettings.min_norm_area : ((typeof window.__analytics_min_norm_area === 'number') ? window.__analytics_min_norm_area : DEFAULT_MIN_NORM_AREA);
                                    const stabilityFrames = (typeof cameraSettings.stability_frames === 'number') ? cameraSettings.stability_frames : ((typeof window.__analytics_stability_frames === 'number') ? window.__analytics_stability_frames : DEFAULT_STABILITY_FRAMES);

                                    // runtime debug toggle (can be disabled from DevTools)
                                    const debugNow = ANALYTICS_DEBUG_DRAW && (window.__analytics_debug !== false);

                                    const filteredObjects = (objectsToDraw || []).filter(o => {
                                        if (!o || !o.box) return false;
                                        // confidence can be in different fields
                                        const conf = (typeof o.confidence === 'number') ? o.confidence : ((typeof o._conf === 'number') ? o._conf : ((typeof o.conf === 'number') ? o.conf : 0));
                                        if (conf < minConf) return false;

                                        let bx = (typeof o.box.x === 'number') ? o.box.x : 0;
                                        let by = (typeof o.box.y === 'number') ? o.box.y : 0;
                                        let bw = (typeof o.box.w === 'number') ? Math.max(0, o.box.w) : 0;
                                        let bh = (typeof o.box.h === 'number') ? Math.max(0, o.box.h) : 0;

                                        // Decide if coords look normalized (<=1) or pixel-space
                                        const looksNorm = (bw <= 1 && bh <= 1);

                                        // Compute area in pixels (best-effort)
                                        const areaPx = looksNorm ? (bw * frameW) * (bh * frameH) : (bw * bh);
                                        const normArea = looksNorm ? (bw * bh) : (areaPx / Math.max(1, frameW * frameH));

                                        if (areaPx < minAreaPx && normArea < minNormArea) return false;

                                        // Bounds check (allow small margin)
                                        if (!looksNorm) {
                                            if (bx + bw < -10 || by + bh < -10 || bx > frameW + 10 || by > frameH + 10) return false;
                                        } else {
                                            if (bx + bw < -0.05 || by + bh < -0.05 || bx > 1.05 || by > 1.05) return false;
                                        }

                                        // Aspect ratio sanity check
                                        const ar = (bh > 0) ? (bw / bh) : 1;
                                        if (ar > 8 || ar < 0.125) return false;

                                        return true;
                                    });

                                    if (debugNow) {
                                        try { console.info('[analytics-filter] camera', cameraId, 'kept', filteredObjects.length, 'of', (objectsToDraw || []).length, 'minConf=', minConf, 'minAreaPx=', minAreaPx, 'stabilityFrames=', stabilityFrames); } catch (e) {}
                                    }

                                    // Convert detections to pixel-space boxes (top-left)
                                    const detectionsPx = [];
                                    for (const o of filteredObjects) {
                                        if (!o || !o.box) continue;
                                        let pxX, pxY, pxW, pxH;
                                        if (isNormalized) {
                                            pxX = (typeof o.box.x === 'number') ? o.box.x * frameW : 0;
                                            pxY = (typeof o.box.y === 'number') ? o.box.y * frameH : 0;
                                            pxW = (typeof o.box.w === 'number') ? o.box.w * frameW : 0;
                                            pxH = (typeof o.box.h === 'number') ? o.box.h * frameH : 0;
                                        } else {
                                            pxX = (typeof o.box.x === 'number') ? o.box.x : 0;
                                            pxY = (typeof o.box.y === 'number') ? o.box.y : 0;
                                            pxW = (typeof o.box.w === 'number') ? o.box.w : 0;
                                            pxH = (typeof o.box.h === 'number') ? o.box.h : 0;
                                        }
                                        // If original coords were center-based, convert to top-left
                                        if (useCenterCoords) {
                                            pxX = pxX - pxW / 2;
                                            pxY = pxY - pxH / 2;
                                        }
                                        detectionsPx.push({ x: pxX, y: pxY, w: Math.max(0, pxW), h: Math.max(0, pxH), label: o.label || '', conf: (typeof o.confidence === 'number') ? o.confidence : ((typeof o._conf === 'number') ? o._conf : 0) });
                                    }

                                    // IoU helper (pixel space)
                                    const iouPx = (A, B) => {
                                        const ax1 = A.x, ay1 = A.y, ax2 = A.x + A.w, ay2 = A.y + A.h;
                                        const bx1 = B.x, by1 = B.y, bx2 = B.x + B.w, by2 = B.y + B.h;
                                        const ix1 = Math.max(ax1, bx1), iy1 = Math.max(ay1, by1);
                                        const ix2 = Math.min(ax2, bx2), iy2 = Math.min(ay2, by2);
                                        const iw = Math.max(0, ix2 - ix1), ih = Math.max(0, iy2 - iy1);
                                        const inter = iw * ih;
                                        const areaA = Math.max(0, A.w) * Math.max(0, A.h);
                                        const areaB = Math.max(0, B.w) * Math.max(0, B.h);
                                        const union = areaA + areaB - inter;
                                        return union <= 0 ? 0 : inter / union;
                                    };

                                    // Associate detections -> tracks (greedy by confidence)
                                    detectionsPx.sort((a, b) => (b.conf || 0) - (a.conf || 0));
                                    const matchedTrackIds = new Set();
                                    for (const det of detectionsPx) {
                                        let best = null;
                                        let bestIou = 0;
                                        for (const t of tracker.tracks) {
                                            // skip stale tracks
                                            if (nowMs - (t.lastSeen || 0) > 2000) continue;
                                            const i = iouPx(det, t.box || t.lastBox || { x: 0, y: 0, w: 0, h: 0 });
                                            if (i > bestIou) { bestIou = i; best = t; }
                                        }
                                        if (best && bestIou > 0.3 && !matchedTrackIds.has(best.id)) {
                                            // update existing track with smoothing
                                            const alpha = 0.6;
                                            // keep a snapshot of previous box+time to compute velocity for prediction
                                            best.prevBox = best.box ? { x: best.box.x, y: best.box.y, w: best.box.w, h: best.box.h } : null;
                                            best.prevTime = best.lastSeen || nowMs;
                                            best.box = best.box || { x: det.x, y: det.y, w: det.w, h: det.h };
                                            best.box.x = alpha * det.x + (1 - alpha) * best.box.x;
                                            best.box.y = alpha * det.y + (1 - alpha) * best.box.y;
                                            best.box.w = alpha * det.w + (1 - alpha) * best.box.w;
                                            best.box.h = alpha * det.h + (1 - alpha) * best.box.h;
                                            best.label = det.label;
                                            best.conf = det.conf;
                                            best.lastSeen = nowMs;
                                            best.age = (best.age || 0) + 1;
                                            // stability counter: increment when detection matched
                                            best.seenCount = (best.seenCount || 0) + 1;
                                            matchedTrackIds.add(best.id);
                                        } else {
                                            // create new track (start with seenCount = 1)
                                            const newTrack = { id: tracker.nextId++, box: { x: det.x, y: det.y, w: det.w, h: det.h }, label: det.label, conf: det.conf, lastSeen: nowMs, age: 1, seenCount: 1 };
                                            // mark as unconfirmed until seenCount >= stabilityFrames
                                            newTrack.confirmed = (newTrack.seenCount >= stabilityFrames);
                                            tracker.tracks.push(newTrack);
                                            matchedTrackIds.add(newTrack.id);
                                        }
                                    }

                                    // expire old tracks but keep a short grace period to allow re-detection
                                    const TRACK_TTL_MS = 2000;
                                    tracker.tracks = tracker.tracks.filter(t => (nowMs - (t.lastSeen || 0)) <= TRACK_TTL_MS);

                                    // For tracks that were not matched this frame, decrement seenCount slightly (so transient matches lose confirmation over time)
                                    for (const t of tracker.tracks) {
                                        if (!matchedTrackIds.has(t.id)) {
                                            // missed this frame
                                            t.seenCount = Math.max(0, (t.seenCount || 0) - 1);
                                            t.confirmed = (t.seenCount >= stabilityFrames);
                                        } else {
                                            // matched this frame: ensure confirmed flag if reached
                                            t.confirmed = (t.seenCount >= stabilityFrames);
                                        }
                                    }

                                    // Prepare final objects for drawing from tracker (only confirmed tracks are drawn)
                                    const drawObjs = tracker.tracks.filter(t => t.confirmed).map(t => {
                                        // Predict short-term motion based on previous box and time to reduce perceived lag.
                                        let predBox = t.box;
                                        try {
                                            if (t.prevBox && t.prevTime && t.lastSeen && t.prevTime < t.lastSeen) {
                                                const dt = (t.lastSeen - t.prevTime) / 1000; // seconds between samples
                                                if (dt > 0) {
                                                    const vx = (t.box.x - t.prevBox.x) / dt;
                                                    const vy = (t.box.y - t.prevBox.y) / dt;
                                                    const vw = (t.box.w - t.prevBox.w) / dt;
                                                    const vh = (t.box.h - t.prevBox.h) / dt;
                                                    // Estimate playback latency (ms). Allow override via window.__analytics_pred_latency_ms
                                                    const predLatencyMs = (typeof window.__analytics_pred_latency_ms === 'number') ? window.__analytics_pred_latency_ms : 150;
                                                    const leadSec = Math.min(0.3, predLatencyMs / 1000); // cap to 300ms
                                                    const ex = t.box.x + vx * leadSec;
                                                    const ey = t.box.y + vy * leadSec;
                                                    const ew = Math.max(0, t.box.w + vw * leadSec);
                                                    const eh = Math.max(0, t.box.h + vh * leadSec);
                                                    // Avoid huge jumps: clamp predicted displacement to reasonable fraction of box size
                                                    const maxShift = Math.max(10, Math.min(200, Math.max(t.box.w, t.box.h) * 0.6));
                                                    const shiftX = Math.max(-maxShift, Math.min(maxShift, ex - t.box.x));
                                                    const shiftY = Math.max(-maxShift, Math.min(maxShift, ey - t.box.y));
                                                    predBox = { x: t.box.x + shiftX, y: t.box.y + shiftY, w: ew, h: eh };
                                                }
                                            }
                                        } catch (e) {
                                            predBox = t.box;
                                        }

                                        const bx = Math.max(0, Math.min(frameW, predBox.x));
                                        const by = Math.max(0, Math.min(frameH, predBox.y));
                                        const bw = Math.max(0, Math.min(frameW - bx, predBox.w));
                                        const bh = Math.max(0, Math.min(frameH - by, predBox.h));
                                        return { label: t.label, score: t.conf, box: { x: bx / frameW, y: by / frameH, w: bw / frameW, h: bh / frameH }, __fromTracker: true, trackId: t.id };
                                    });

                                    // expose tracks in diagnostics (including unconfirmed)
                                    try { window.__analytics_diag[cameraId].tracks = tracker.tracks.map(t => ({ id: t.id, box: t.box, conf: t.conf, seenCount: t.seenCount, confirmed: !!t.confirmed })); } catch (e) {}

                                    // Debug: draw the mapped source display rectangle and log intermediate data
                                    if (ANALYTICS_DEBUG_DRAW) {
                                        try {
                                            // border around the actual mapped source image area
                                            ctx.save();
                                            ctx.lineWidth = 2;
                                            ctx.strokeStyle = 'rgba(0,200,0,0.9)';
                                            ctx.setLineDash([4,2]);
                                            ctx.strokeRect(Math.round(offsetX) + 0.5, Math.round(offsetY) + 0.5, Math.round(displayW), Math.round(displayH));
                                            ctx.restore();

                                            // quick preview of raw detections in pixel-space mapped to display coords (orange)
                                            for (const dp of detectionsPx) {
                                                const px = Math.max(0, Math.min(frameW, dp.x));
                                                const py = Math.max(0, Math.min(frameH, dp.y));
                                                const pw = Math.max(0, Math.min(frameW - px, dp.w));
                                                const ph = Math.max(0, Math.min(frameH - py, dp.h));
                                                const rx = offsetX + (px / frameW) * displayW;
                                                const ry = offsetY + (py / frameH) * displayH;
                                                const rw = (pw / frameW) * displayW;
                                                const rh = (ph / frameH) * displayH;
                                                ctx.save();
                                                ctx.lineWidth = 1;
                                                ctx.strokeStyle = 'rgba(255,140,0,0.9)';
                                                ctx.globalAlpha = 0.9;
                                                ctx.strokeRect(Math.round(rx) + 0.5, Math.round(ry) + 0.5, Math.round(rw), Math.round(rh));
                                                ctx.restore();
                                            }

                                            // Log arrays to console for deeper inspection
                                            console.info('[analytics-debug] camera', cameraId, 'rawObjects:', (state.objects || []).map(o=> o && o.box ? {box:o.box,label:o.label,conf:o.confidence||o.score} : o));
                                            console.info('[analytics-debug] camera', cameraId, 'detectionsPx:', detectionsPx.map(d=>({x:Math.round(d.x),y:Math.round(d.y),w:Math.round(d.w),h:Math.round(d.h),conf:d.conf} )));
                                            console.info('[analytics-debug] camera', cameraId, 'drawObjs(normalized):', drawObjs.map(d=>({box:d.box,label:d.label,score:d.score,__fromTracker:d.__fromTracker})));

                                            // Also draw small textual inspection info on overlay (right side)
                                            try {
                                                ctx.save();
                                                ctx.resetTransform();
                                                ctx.scale(dpr, dpr);
                                                ctx.font = '11px monospace';
                                                ctx.fillStyle = 'rgba(0,0,0,0.6)';
                                                const infoLines = [];
                                                const firstRaw = (state.objects && state.objects[0] && state.objects[0].box) ? state.objects[0].box : null;
                                                const firstDet = (detectionsPx && detectionsPx[0]) ? detectionsPx[0] : null;
                                                const firstDraw = (drawObjs && drawObjs[0] && drawObjs[0].box) ? drawObjs[0].box : null;
                                                if (firstRaw) infoLines.push(`raw: x=${Math.round(firstRaw.x)} y=${Math.round(firstRaw.y)} w=${Math.round(firstRaw.w)} h=${Math.round(firstRaw.h)}`);
                                                if (firstDet) infoLines.push(`px: x=${Math.round(firstDet.x)} y=${Math.round(firstDet.y)} w=${Math.round(firstDet.w)} h=${Math.round(firstDet.h)}`);
                                                if (firstDraw) infoLines.push(`draw(norm): x=${firstDraw.x.toFixed(3)} y=${firstDraw.y.toFixed(3)} w=${firstDraw.w.toFixed(3)} h=${firstDraw.h.toFixed(3)}`);
                                                infoLines.push(`frame:${frameW}x${frameH} css:${Math.round(cssWidth)}x${Math.round(cssHeight)} disp:${Math.round(displayW)}x${Math.round(displayH)}`);
                                                let ix = Math.max(6, cssWidth - 280);
                                                let iy = 6;
                                                for (const L of infoLines) {
                                                    ctx.fillText(L, ix, iy);
                                                    iy += 14;
                                                }
                                                ctx.restore();
                                            } catch (e) { /* ignore */ }
                                        } catch (e) { }
                                    }

                                    // Draw using tracked objects
                                    for (const obj of drawObjs) {
                                        if (!obj.box) continue;
                                        // Determine normalized coordinates (nx,ny,nw,nh) exactly once.
                                        let nx = 0, ny = 0, nw = 0, nh = 0;

                                        try {
                                            // Tracker outputs normalized top-left boxes. Respect that to avoid re-normalizing.
                                            if (obj.__fromTracker) {
                                                nx = (typeof obj.box.x === 'number') ? obj.box.x : 0;
                                                ny = (typeof obj.box.y === 'number') ? obj.box.y : 0;
                                                nw = (typeof obj.box.w === 'number') ? obj.box.w : 0;
                                                nh = (typeof obj.box.h === 'number') ? obj.box.h : 0;
                                            } else {
                                                // For non-tracker objects decide whether they are provided in pixels (>1)
                                                const looksLikePixels = (typeof obj.box.x === 'number' && obj.box.x > 1) || (typeof obj.box.y === 'number' && obj.box.y > 1) || (typeof obj.box.w === 'number' && obj.box.w > 1) || (typeof obj.box.h === 'number' && obj.box.h > 1);
                                                if (looksLikePixels) {
                                                    // Pixel-space -> convert to normalized
                                                    let bx = (typeof obj.box.x === 'number') ? obj.box.x : 0;
                                                    let by = (typeof obj.box.y === 'number') ? obj.box.y : 0;
                                                    let bw = (typeof obj.box.w === 'number') ? obj.box.w : 0;
                                                    let bh = (typeof obj.box.h === 'number') ? obj.box.h : 0;
                                                    if (useCenterCoords) {
                                                        bx = bx - bw / 2;
                                                        by = by - bh / 2;
                                                    }
                                                    nx = bx / frameW;
                                                    ny = by / frameH;
                                                    nw = bw / frameW;
                                                    nh = bh / frameH;
                                                } else {
                                                    // Already normalized
                                                    nx = (typeof obj.box.x === 'number') ? obj.box.x : 0;
                                                    ny = (typeof obj.box.y === 'number') ? obj.box.y : 0;
                                                    nw = (typeof obj.box.w === 'number') ? obj.box.w : 0;
                                                    nh = (typeof obj.box.h === 'number') ? obj.box.h : 0;
                                                    // If analytics produced normalized center coords (edge-case), convert in normalized space
                                                    if (!isNormalized && useCenterCoords) {
                                                        nx = nx - nw / 2;
                                                        ny = ny - nh / 2;
                                                    }
                                                }
                                            }
                                        } catch (e) {
                                            // Fallback to safe zeros on unexpected data
                                            nx = ny = nw = nh = 0;
                                        }

                                        // Clamp normalized values to [0,1]
                                        nx = Math.max(0, Math.min(1, nx));
                                        ny = Math.max(0, Math.min(1, ny));
                                        nw = Math.max(0, Math.min(1, nw));
                                        nh = Math.max(0, Math.min(1, nh));

                                        // Map normalized coords to display pixels using displayW/displayH and offsets
                                        const x = offsetX + nx * displayW;
                                        const y = offsetY + ny * displayH;
                                        const w = nw * displayW;
                                        const h = nh * displayH;

                                            // Save mapped boxes for debugging/inspection
                                            try {
                                                window.__analytics_diag = window.__analytics_diag || {};
                                                window.__analytics_diag[cameraId] = window.__analytics_diag[cameraId] || {};
                                                window.__analytics_diag[cameraId].lastBoxes = window.__analytics_diag[cameraId].lastBoxes || [];
                                                window.__analytics_diag[cameraId].lastBoxes.push({ x: Math.round(x), y: Math.round(y), w: Math.round(w), h: Math.round(h), label: obj.label });
                                            } catch (e) { /* ignore */ }

                                            // Draw diagnostic overlay text (small, top-left) when debug enabled
                                            if (ANALYTICS_DEBUG_DRAW) {
                                                ctx.save();
                                                ctx.resetTransform();
                                                // Draw on CSS pixels (canvas is already DPR-scaled but ctx was scaled earlier)
                                                ctx.scale(dpr, dpr);
                                                ctx.fillStyle = 'rgba(0,0,0,0.6)';
                                                ctx.font = '12px monospace';
                                                const diagLines = [
                                                    `frame: ${frameW}x${frameH}`,
                                                    `css: ${Math.round(cssWidth)}x${Math.round(cssHeight)}`,
                                                    `disp: ${Math.round(displayW)}x${Math.round(displayH)}`,
                                                    `scale: ${scale.toFixed(3)}`,
                                                    `off: ${Math.round(offsetX)},${Math.round(offsetY)}`
                                                ];
                                                let ty = 4;
                                                for (const line of diagLines) {
                                                    ctx.fillText(line, 6, ty);
                                                    ty += 14;
                                                }
                                                ctx.restore();
                                            }
                                        const color = colorFor(obj.label || 'obj');

                                        // Draw box (use 0.5 to get crisp 1px lines on integer coords)
                                        ctx.strokeStyle = 'lime';
                                        ctx.fillStyle = 'lime';
                                        ctx.globalAlpha = 1;
                                        ctx.strokeRect(Math.round(x) + 0.5, Math.round(y) + 0.5, Math.round(w), Math.round(h));

                                            // Extra visible debug outline when enabled
                                            if (ANALYTICS_DEBUG_DRAW) {
                                                ctx.save();
                                                ctx.lineWidth = 3;
                                                ctx.strokeStyle = '#ff00ff';
                                                ctx.globalAlpha = 0.9;
                                                ctx.strokeRect(Math.round(x) + 0.5, Math.round(y) + 0.5, Math.round(w), Math.round(h));
                                                ctx.restore();
                                            }

                                        // Optional debug markers: show mapped center and source corner projection
                                        if (ANALYTICS_DEBUG_DRAW) {
                                            // mapped center
                                            ctx.fillStyle = '#ffffff';
                                            ctx.fillRect(Math.round(x + w/2) - 2, Math.round(y + h/2) - 2, 4, 4);
                                            // top-left of source (0,0) maps to offsetX,offsetY
                                            ctx.fillStyle = '#ff00ff';
                                            ctx.fillRect(Math.round(offsetX) - 2, Math.round(offsetY) - 2, 4, 4);
                                            // bottom-right of source maps to offsetX+displayW, offsetY+displayH
                                            ctx.fillStyle = '#00ffff';
                                            ctx.fillRect(Math.round(offsetX + displayW) - 2, Math.round(offsetY + displayH) - 2, 4, 4);
                                        }

                                        // label background and text
                                        const label = (obj.label || '') + (obj.score ? ` ${Math.round(obj.score*100)}%` : '');
                                        if (label) {
                                            const padding = 4;
                                            const textW = ctx.measureText(label).width;
                                            const bgW = textW + padding * 2;
                                            const bgH = 16 + padding;
                                            const tx = Math.round(x);
                                            const ty = Math.max(0, Math.round(y - bgH));
                                            ctx.globalAlpha = 0.85;
                                            ctx.fillStyle = color;
                                            ctx.fillRect(tx, ty, bgW, bgH);
                                            ctx.globalAlpha = 1;
                                            ctx.fillStyle = '#ffffff';
                                            ctx.fillText(label, tx + padding, ty + (padding/2));
                                        }
                                    }
                                }
                            }
                        } catch (e) {

                        }
                        // Ensure overlay is visible if we have objects
                        if (state.objects && state.objects.length > 0 && overlayCanvas) {
                            overlayCanvas.style.display = '';
                        }
                    }
                }
                animationFrameId = requestAnimationFrame(renderAnalytics);
            }
            renderAnalytics();
        }

        function getActiveLayoutState() {
            const { layouts, activeLayoutId } = stateManager.state;
            if (!layouts || layouts.length === 0) return null;
            return layouts.find(l => l.id === activeLayoutId) || layouts[0];
        }

        function getGridState() {
            const activeLayout = getActiveLayoutState();
            return activeLayout ? activeLayout.gridState : Array(64).fill(null);
        }

        function updatePlaceholdersLanguage() {
            const placeholderHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
            gridCells.forEach(cell => {
                const video = cell.querySelector('.video-player');
                if (!video || !video.srcObject) {
                    cell.innerHTML = placeholderHTML;
                }
            });
        }

        function initializeLayoutControls() {
            const layouts = ["1x1", "2x2", "3x3", "4x4", "5x5", "8x4", "8x8"];
            layoutControls.innerHTML = '';
            layouts.forEach(layout => {
                const btn = document.createElement('button');
                btn.className = 'layout-btn';
                btn.dataset.layout = layout;
                btn.textContent = layout.split('x').reduce((a, b) => a * b, 1);
                btn.title = `Layout ${layout}`;
                btn.onclick = () => {
                    const [cols, rows] = layout.split('x').map(Number);
                    stateManager.updateGridLayout({ cols, rows });
                };
                layoutControls.appendChild(btn);
            });
        }

        function updateActiveLayoutButton() {
            const activeLayout = getActiveLayoutState();
            if (!activeLayout || !activeLayout.layout) return;
            const { layout } = activeLayout;
            const currentLayout = `${layout.cols}x${layout.rows}`;
            layoutControls.querySelectorAll('.layout-btn').forEach(btn => {
                btn.classList.toggle('active', btn.dataset.layout === currentLayout);
            });
        }

        function updateGridLayoutView() {
            const activeLayout = getActiveLayoutState();
            if (!activeLayout || !activeLayout.layout) return;
            const { layout } = activeLayout;
            const totalVisibleCells = layout.cols * layout.rows;
            const cellWidth = 100 / layout.cols;
            const cellHeight = 100 / layout.rows;
            gridCells.forEach((cell, i) => {
                if (i < totalVisibleCells) {
                    cell.style.display = 'flex';
                    cell.style.top = `${Math.floor(i / layout.cols) * cellHeight}%`;
                    cell.style.left = `${i % layout.cols * cellWidth}%`;
                    cell.style.width = `${cellWidth}%`;
                    cell.style.height = `${cellHeight}%`;
                } else {
                    stopPlayerForCell(i);
                    cell.style.display = 'none';
                }
            });
            updateActiveLayoutButton();
        }
        
        function stopPlayerForCell(cellIndex) {
            const pc = peerConnections[cellIndex];
            if (pc) {
                pc.close();
                delete peerConnections[cellIndex];
            }
            const cell = gridCells[cellIndex];
            if (cell) {
                const video = cell.querySelector('.video-player');
                if (video && video.srcObject) {
                    video.srcObject.getTracks().forEach(track => track.stop());
                    video.srcObject = null;
                    delete video.dataset.streamPath;
                }
            }
        }

        function attachAllEvents(cellElement, cellIndex) {
            const cellState = getGridState()[cellIndex];
            const currentUser = App.stateManager.state.currentUser;

            cellElement.ondblclick = () => toggleFullscreen(cellIndex);
            cellElement.ondragover = (e) => { e.preventDefault(); cellElement.classList.add('drag-over'); };
            cellElement.ondragleave = () => { cellElement.classList.remove('drag-over'); };
            cellElement.ondrop = (e) => {
                e.preventDefault();
                cellElement.classList.remove('drag-over');
                if (currentUser?.role !== 'admin' && !currentUser?.permissions?.manage_layout) return;
                
                const newGrid = getGridState().map(g => g ? { ...g } : null);
                const cameraIdStr = e.dataTransfer.getData('application/x-camera-id');
                const sourceCellIdStr = e.dataTransfer.getData('application/x-source-cell-id');

                if (cameraIdStr) {
                    const cameraId = parseInt(cameraIdStr, 10);
                    if (isNaN(cameraId)) return;

                    if (sourceCellIdStr) {
                        const sourceCellId = parseInt(sourceCellIdStr, 10);
                        if (sourceCellId !== cellIndex) {
                            const sourceContent = newGrid[sourceCellId];
                            const targetContent = newGrid[cellIndex];
                            newGrid[cellIndex] = sourceContent;
                            newGrid[sourceCellId] = targetContent;
                        }
                    } else {
                        newGrid[cellIndex] = { camera: { id: cameraId }, streamId: 1 };
                    }
                    stateManager.updateGridState(newGrid);
                }
            };

            cellElement.oncontextmenu = (e) => {
                if (cellState && cellState.camera) {
                    e.preventDefault();
                    const cameraId = cellState.camera.id;
                    const menuItems = {
                        open_in_browser: `🌐  ${App.i18n.t('context_open_in_browser')}`,
                        files: `🗂️  ${App.i18n.t('context_file_manager')}`,
                        ssh: `💻  ${App.i18n.t('context_ssh')}`,
                        archive: `🗄️  ${App.i18n.t('archive_title')}`
                    };

                    if (currentUser.role === 'admin' || currentUser.permissions?.edit_cameras) {
                        menuItems.edit = `✏️  ${App.i18n.t('context_edit')}`;
                    }
                    if (currentUser.role === 'admin' || currentUser.permissions?.delete_cameras) {
                        menuItems.delete = `🗑️  ${App.i18n.t('context_delete')}`;
                    }
                    
                    window.api.showCameraContextMenu({ cameraId, labels: menuItems });
                }
            };

            // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ --- VVVVVV
            // Переносим обработчики кнопок внутрь, чтобы они всегда были актуальны
            const controls = cellElement.querySelector('.cell-controls');
            if (!controls) return;

            const streamSwitchBtn = controls.querySelector('.stream-switch-btn');
            if (streamSwitchBtn) {
                streamSwitchBtn.onclick = (e) => {
                    e.stopPropagation();
                    const currentGridState = getGridState()[cellIndex];
                    if (!currentGridState) return;

                    const newStreamId = currentGridState.streamId === 0 ? 1 : 0;
                    
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (newGrid[cellIndex]) {
                        newGrid[cellIndex].streamId = newStreamId;
                        stateManager.updateGridState(newGrid);
                    }
                };
            }

            const pauseBtn = controls.querySelector('.pause-btn');
            if (pauseBtn) {
                pauseBtn.onclick = (e) => {
                    e.stopPropagation();
                    const newGrid = getGridState().map(g => g ? { ...g } : null);
                    if (!newGrid[cellIndex]) return;
                    
                    // Инвертируем состояние паузы
                    if (newGrid[cellIndex].paused) {
                        delete newGrid[cellIndex].paused;
                    } else {
                        newGrid[cellIndex].paused = true;
                    }
                    stateManager.updateGridState(newGrid);
                };
            }
            // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^

            if (!cellState) return;

            const screenshotBtn = controls.querySelector('.screenshot-btn');
            if (screenshotBtn) {
                screenshotBtn.onclick = async (e) => {
                    e.stopPropagation();
                    const video = cellElement.querySelector('.video-player');
                    if (!video || cellElement.classList.contains('paused-state') || !video.srcObject) {
                        App.modalHandler.showToast('Нельзя сделать скриншот, когда видео на паузе или не загружено.', true);
                        return;
                    }
                    const canvas = document.createElement('canvas');
                    canvas.width = video.videoWidth;
                    canvas.height = video.videoHeight;
                    const ctx = canvas.getContext('2d');
                    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
                    
                    const dataUrl = canvas.toDataURL('image/jpeg');
                    const cameraName = cellState.camera.name || 'screenshot';

                    try {
                        const result = await window.api.saveScreenshot({ dataUrl, cameraName });
                        if (result.success) {
                            App.modalHandler.showToast(`Скриншот сохранен: ${result.path}`);
                        } else {
                            App.modalHandler.showToast(`Ошибка сохранения: ${result.error}`, true);
                        }
                    } catch (err) {

                        App.modalHandler.showToast(`Критическая ошибка: ${err.message}`, true);
                    }
                };
            }

            controls.querySelector('.fullscreen-btn').onclick = (e) => { e.stopPropagation(); toggleFullscreen(cellIndex); };

            controls.querySelector('.close-btn').onclick = (e) => {
                e.stopPropagation();
                const newGrid = getGridState().map(g => g ? { ...g } : null);
                newGrid[cellIndex] = null;
                stateManager.updateGridState(newGrid);
            };

            const audioBtn = controls.querySelector('.audio-btn');
            if (audioBtn) {
                audioBtn.onclick = (e) => {
                    e.stopPropagation();
                    const video = cellElement.querySelector('.video-player');
                    if (video) {
                        video.muted = !video.muted;
                        audioBtn.innerHTML = video.muted ? '<i class="material-icons">volume_off</i>' : '<i class="material-icons">volume_up</i>';
                    }
                };
            }

            const recordBtn = controls.querySelector('.record-btn');
            const camera = stateManager.state.cameras.find(c => c.id === cellState.camera.id);
            if (recordBtn && camera) {
                recordBtn.onclick = (e) => {
                    e.stopPropagation();
                    window.api.toggleRecording(camera);
                };
            }

            const ptzButtons = cellElement.querySelectorAll('.ptz-btn');
            const sendPtzCommand = (command, action) => {
                window.api.ptzControl({ cameraId: cellState.camera.id, command, action });
            };
            ptzButtons.forEach(btn => {
                const command = btn.dataset.command;
                if (!command) return;
                btn.addEventListener('mousedown', (e) => { e.stopPropagation(); sendPtzCommand(command, 'start'); });
                btn.addEventListener('mouseup', (e) => { e.stopPropagation(); sendPtzCommand('stop', 'stop'); });
                btn.addEventListener('mouseleave', (e) => { if (e.buttons !== 1) { sendPtzCommand('stop', 'stop'); }});
            });
        }

        function createVideoPlayer(cell, streamUrl) {
            // Remove old video if exists
            const oldVideo = cell.querySelector('.video-player');
            if (oldVideo) cell.removeChild(oldVideo);
            const video = document.createElement('video');
            video.className = 'video-player';
            video.setAttribute('playsinline', '');
            video.setAttribute('controls', '');
            video.setAttribute('autoplay', '');
            video.style.width = '100%';
            video.style.height = '100%';
            // Detect device and stream type
            if (/\.m3u8($|\?)/.test(streamUrl)) {
                // HLS stream
                window.HlsLoader.attachHlsStream(video, streamUrl);
            } else {
                // Fallback: direct src
                video.src = streamUrl;
            }
            cell.appendChild(video);
            return video;
        }

        function getPreferredStreamUrl(cameraId) {
            // Проверяем доступность HLS потока
            const hlsUrl = 'http://127.0.0.1:8888/' + cameraId + '/hls.m3u8';
            // Проверяем доступность WebRTC (WHEP)
            const whepUrl = 'http://127.0.0.1:8889/' + cameraId + '/whep';
            // Для мобильных устройств и ТВ пробуем HLS, иначе WebRTC
            const isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini|SmartTV|GoogleTV/i.test(navigator.userAgent);
            // Попробуем HLS, если не доступен — fallback на WHEP
            return fetch(hlsUrl, { method: 'HEAD' })
                .then(resp => resp.ok ? hlsUrl : whepUrl)
                .catch(() => whepUrl);
        }

        async function attachStreamToCell(cell, cameraId) {
            cell.innerHTML = '';
            const streamUrl = await getPreferredStreamUrl(cameraId);
            createVideoPlayer(cell, streamUrl);
        }

        async function render() {
            if (isRendering) {
                return;
            }
            isRendering = true;
            try {
                updateGridLayoutView();
                const activeLayout = getActiveLayoutState();
                if (!activeLayout) {
                    gridCells.forEach((cell, i) => stopPlayerForCell(i));
                    isRendering = false;
                    return;
                }

                const { gridState } = activeLayout;
                const { cameras, recordingStates } = stateManager.state;

                for (let i = 0; i < gridCells.length; i++) {
                    const cellElement = gridCells[i];
                    const cellState = gridState[i];
                    
                    cellElement.setAttribute('draggable', 'false');
                    cellElement.ondragstart = null;

                    // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ: НОВАЯ ЛОГИКА РЕНДЕРА --- VVVVVV
                    if (cellState) {
                        const camera = cameras.find(c => c.id === cellState.camera.id);
                        if (!camera) {
                            stopPlayerForCell(i);
                            continue;
                        }

                        // Если в ячейке нет контента, создаем его
                        if (!cellElement.querySelector('.video-wrapper')) {
                            const template = document.getElementById('grid-cell-content-template');
                            const content = template.content.cloneNode(true);
                            cellElement.innerHTML = '';
                            cellElement.appendChild(content);
                            App.i18n.applyTranslationsToDOM(cellElement);
                        }

                        cellElement.setAttribute('draggable', 'true');
                        cellElement.ondragstart = (e) => {
                            e.stopPropagation();
                            e.dataTransfer.setData('application/x-camera-id', camera.id.toString());
                            e.dataTransfer.setData('application/x-source-cell-id', i.toString());
                        };

                        const videoElToUse = cellElement.querySelector('.video-player');
                        const currentStream = videoElToUse ? videoElToUse.dataset.streamPath : null;

                        if (cellState.paused) {
                            stopPlayerForCell(i);
                            cellElement.classList.add('paused-state');
                        } else {
                            cellElement.classList.remove('paused-state');
                            const streamId = cellState.streamId === 0 ? 0 : 1;
                            const streamPath = `cam${camera.id}_${streamId}`;

                            if (currentStream !== streamPath) {
                                // Останавливаем старый поток перед запуском нового
                                if (videoElToUse) {
                                    // Радикальная очистка
                                    if (videoElToUse.srcObject) {
                                        videoElToUse.srcObject.getTracks().forEach(track => {
                                            track.stop();
                                            videoElToUse.srcObject.removeTrack(track);
                                        });
                                        videoElToUse.srcObject = null;
                                    }
                                    videoElToUse.load();
                                    delete videoElToUse.dataset.streamPath;
                                }
                                if (peerConnections[i]) {
                                    try { peerConnections[i].close(); } catch(e) {}
                                    peerConnections[i] = null;
                                }
                                videoElToUse.dataset.streamPath = streamPath;
                                // Даем время на очистку перед запуском нового потока
                                setTimeout(() => {
                                    startWebRTCPlayer(videoElToUse, streamPath, i, cellElement);
                                }, 150);
                            }
                            cellElement.classList.add('active');
                        }

                        // Обновляем UI элементы
                        const nameDiv = cellElement.querySelector('.cell-name');
                        if (nameDiv) {
                            const qualityLabel = (cellState.streamId === 0) ? 'HD' : 'SD';
                            nameDiv.textContent = `${camera.name} (${qualityLabel})`;
                        }
                        const streamSwitchBtn = cellElement.querySelector('.stream-switch-btn');
                        if(streamSwitchBtn) streamSwitchBtn.innerHTML = (cellState.streamId === 0) ? '<i class="material-icons">hd</i>' : '<i class="material-icons">sd</i>';
                        
                        const recordBtn = cellElement.querySelector('.record-btn');
                        if (recordBtn) recordBtn.classList.toggle('recording', !!recordingStates[camera.id]);

                        const pauseBtn = cellElement.querySelector('.pause-btn i');
                        if (pauseBtn) pauseBtn.textContent = cellState.paused ? 'play_arrow' : 'pause';

                    } else { // Если cellState пустой
                        stopPlayerForCell(i);
                        cellElement.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                        cellElement.classList.remove('active', 'error-state', 'paused-state');
                    }
                    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^
                    
                    attachAllEvents(cellElement, i);
                }
            } finally {
                isRendering = false;
            }
        }
        
        async function startWebRTCPlayer(videoElement, streamPath, cellIndex, cellElement) {
            const loadingOverlay = cellElement.querySelector('.loading-overlay');
            const errorOverlay = cellElement.querySelector('.error-overlay');
            
            loadingOverlay.classList.remove('hidden');
            errorOverlay.classList.add('hidden');
            cellElement.classList.remove('error-state');

            try {
                // Очищаем старый поток и PeerConnection более надежно
                if (videoElement.srcObject) {
                    const tracks = videoElement.srcObject.getTracks();
                    tracks.forEach(track => {
                        track.stop();
                        videoElement.srcObject.removeTrack(track);
                    });
                    videoElement.srcObject = null;
                }
                // Дополнительная очистка: принудительно останавливаем все треки
                if (videoElement.captureStream) {
                    try {
                        const captureStream = videoElement.captureStream();
                        captureStream.getTracks().forEach(track => track.stop());
                    } catch(e) {}
                }
                videoElement.load(); // Перезагружаем элемент для полной очистки
                if (peerConnections[cellIndex]) {
                    try { 
                        peerConnections[cellIndex].close(); 
                        peerConnections[cellIndex] = null;
                    } catch(e) { 
                        console.warn('Error closing old PeerConnection:', e);
                    }
                }

                // Даем больше времени на очистку
                await new Promise(resolve => setTimeout(resolve, 200));

                const pc = new RTCPeerConnection();
                peerConnections[cellIndex] = pc;

                pc.ontrack = (event) => {
                    // Очищаем старые треки перед добавлением новых
                    if (videoElement.srcObject) {
                        const existingTracks = videoElement.srcObject.getTracks();
                        existingTracks.forEach(track => {
                            if (track.kind === event.track.kind) {
                                videoElement.srcObject.removeTrack(track);
                                track.stop();
                            }
                        });
                    }
                    if (!videoElement.srcObject) {
                        videoElement.srcObject = new MediaStream();
                    }
                    videoElement.srcObject.addTrack(event.track);
                };

                pc.addTransceiver('video', { 'direction': 'recvonly' });
                pc.addTransceiver('audio', { 'direction': 'recvonly' });

                await pc.setLocalDescription(await pc.createOffer());

                // Try multiple times to POST offer to MediaMTX WHEP endpoint (handles transient races)
                let response = null;
                const maxAttempts = 3;
                for (let attempt = 1; attempt <= maxAttempts; attempt++) {
                    try {
                        response = await fetch(`http://127.0.0.1:8889/${streamPath}/whep`, {
                            method: 'POST',
                            body: pc.localDescription.sdp,
                            headers: { 'Content-Type': 'application/sdp' }
                        });
                        if (response && response.ok) break;
                    } catch (err) {
                        // network/connection error - will retry
                    }
                    // exponential backoff: 500ms, 1000ms, 2000ms
                    await new Promise(r => setTimeout(r, 500 * Math.pow(2, attempt-1)));
                }

                if (!response || !response.ok) {
                    const errorText = response ? await response.text() : '';
                    // VVVVVV --- НАЧАЛО ИЗМЕНЕНИЙ: Улучшенная обработка ошибок --- VVVVVV
                    let friendlyError = response ? `WHEP: ${response.status} ${response.statusText}` : App.t('error_connection_refused');
                    try {
                        const errJson = JSON.parse(errorText || '{}');
                        if (errJson.error && errJson.error.includes('no one is publishing to path')) {
                            friendlyError = App.t('error_connection_refused');
                        } else if (errJson.error) {
                            friendlyError = errJson.error;
                        }
                    } catch(e) { /* ignore json parse error */ }
                    throw new Error(friendlyError);
                    // ^^^^^^ --- КОНЕЦ ИЗМЕНЕНИЙ --- ^^^^^^
                }

                const answer = await response.text();
                await pc.setRemoteDescription({ type: 'answer', sdp: answer });

                videoElement.onplaying = () => {
                    loadingOverlay.classList.add('hidden');
                };

            } catch (e) {

                errorOverlay.classList.remove('hidden');
                errorOverlay.querySelector('.error-message').textContent = e.message;
                cellElement.classList.add('error-state');
                stopPlayerForCell(cellIndex);

                // === Добавляем обработчики кнопок ===
                const retryBtn = errorOverlay.querySelector('.retry-button');
                const closeBtn = errorOverlay.querySelector('.close-on-error-btn');
                if (retryBtn) {
                    retryBtn.onclick = (ev) => {
                        ev.stopPropagation();
                        errorOverlay.classList.add('hidden');
                        cellElement.classList.remove('error-state');
                        // Повторно инициируем подключение
                        startWebRTCPlayer(videoElement, streamPath, cellIndex, cellElement);
                    };
                }
                if (closeBtn) {
                    closeBtn.onclick = (ev) => {
                        ev.stopPropagation();
                        errorOverlay.classList.add('hidden');
                        cellElement.classList.remove('error-state');
                        // Очищаем ячейку
                        stopPlayerForCell(cellIndex);
                        cellElement.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                        attachAllEvents(cellElement, cellIndex);
                    };
                }
                // === Конец обработчиков ===
            } finally {
                loadingOverlay.classList.add('hidden');
            }
        }

        function toggleFullscreen(cellIndex) {
            const cell = gridCells[cellIndex];
            if (!cell) return;
            const newGrid = getGridState().map(g => g ? { ...g } : null);
            const cellState = newGrid[cellIndex];
            if (!cellState) return;

            const isCurrentlyFullscreen = cell.classList.contains('fullscreen');
            const fsBtnIcon = cell.querySelector('.fullscreen-btn i');

            if (isCurrentlyFullscreen) {
                // Очищаем поток перед выходом из полноэкранного режима
                const videoEl = cell.querySelector('.video-player');
                if (videoEl) {
                    if (videoEl.srcObject) {
                        videoEl.srcObject.getTracks().forEach(track => track.stop());
                        videoEl.srcObject = null;
                    }
                    const newVideoEl = document.createElement('video');
                    newVideoEl.className = 'video-player';
                    newVideoEl.setAttribute('playsinline', '');
                    newVideoEl.setAttribute('autoplay', '');
                    newVideoEl.style.width = '100%';
                    newVideoEl.style.height = '100%';
                    videoEl.parentNode.replaceChild(newVideoEl, videoEl);
                }
                // Восстанавливаем overlay-canvas для SD, но не показываем full-frame analytics image
                // по умолчанию — это мешает отображению SD-превью (ведёт к дёрганью).
                // Если необходимо увидеть реальное кадр-из-аналитики, установите
                // window.__analytics_forceImageOverlay = true в DevTools.
                const cameraId = cellState?.camera?.id;
                if (cameraId && analyticsState[cameraId]) {
                    // Prefer drawing boxes on overlay canvas for SD preview to avoid overlaying
                    // a full-frame analytics image over the SD player which creates visual jitter.
                    let overlayCanvas = cell.querySelector('.overlay-canvas');
                    if (!overlayCanvas) {
                        overlayCanvas = document.createElement('canvas');
                        overlayCanvas.className = 'overlay-canvas';
                        overlayCanvas.style.position = 'absolute';
                        overlayCanvas.style.top = '0';
                        overlayCanvas.style.left = '0';
                        overlayCanvas.style.width = '100%';
                        overlayCanvas.style.height = '100%';
                        overlayCanvas.style.zIndex = '15';
                        overlayCanvas.style.pointerEvents = 'none';
                        cell.appendChild(overlayCanvas);
                    }
                    overlayCanvas.style.display = '';

                    // Only attach full-frame analytics image if explicitly forced (debug) or
                    // when the cell is fullscreen (HD), otherwise keep it hidden so SD stream
                    // remains visible and stable.
                    const shouldShowImageOverlay = !!window.__analytics_forceImageOverlay || cell.classList.contains('fullscreen');
                    let analyticsImg = cell.querySelector('.analytics-img');
                    if (shouldShowImageOverlay) {
                        if (!analyticsImg) {
                            analyticsImg = document.createElement('img');
                            analyticsImg.className = 'analytics-img';
                            analyticsImg.style.position = 'absolute';
                            analyticsImg.style.top = '0';
                            analyticsImg.style.left = '0';
                            analyticsImg.style.width = '100%';
                            analyticsImg.style.height = '100%';
                            analyticsImg.style.objectFit = 'contain';
                            analyticsImg.style.zIndex = '10';
                            analyticsImg.style.pointerEvents = 'none';
                            cell.appendChild(analyticsImg);
                        }
                        // Only set source/display when we actually intend to show the image overlay
                        if (analyticsState[cameraId] && analyticsState[cameraId].frame_base64) {
                            analyticsImg.src = analyticsState[cameraId].frame_base64;
                            analyticsImg.style.display = '';
                        } else {
                            analyticsImg.style.display = 'none';
                        }
                    } else if (analyticsImg) {
                        // hide if present
                        analyticsImg.style.display = 'none';
                    }
                }
                if (peerConnections[cellIndex]) {
                    try { peerConnections[cellIndex].close(); } catch(e) {}
                    peerConnections[cellIndex] = null;
                }
                cellState.streamId = 1;
                gridContainer.classList.remove('fullscreen-mode');
                cell.classList.remove('fullscreen');
                if (fsBtnIcon) fsBtnIcon.textContent = 'fullscreen';
                fullscreenCellIndex = null;
            } else {
                // Полностью удаляем analytics-img и overlay-canvas при входе в fullscreen (HD)
                const analyticsImg = cell.querySelector('.analytics-img');
                if (analyticsImg) analyticsImg.remove();
                const overlayCanvas = cell.querySelector('.overlay-canvas');
                if (overlayCanvas) overlayCanvas.remove();
                if (fullscreenCellIndex !== null) {
                    const oldFullscreenCell = gridCells[fullscreenCellIndex];
                    if (oldFullscreenCell && newGrid[fullscreenCellIndex]) {
                        newGrid[fullscreenCellIndex].streamId = 1;
                        oldFullscreenCell.classList.remove('fullscreen');
                        const oldFsBtnIcon = oldFullscreenCell.querySelector('.fullscreen-btn i');
                        if (oldFsBtnIcon) oldFsBtnIcon.textContent = 'fullscreen';
                    }
                }
                // Очищаем старый поток перед переключением на HD
                const videoEl = cell.querySelector('.video-player');
                if (videoEl) {
                    if (videoEl.srcObject) {
                        videoEl.srcObject.getTracks().forEach(track => track.stop());
                        videoEl.srcObject = null;
                    }
                    const newVideoEl = document.createElement('video');
                    newVideoEl.className = 'video-player';
                    newVideoEl.setAttribute('playsinline', '');
                    newVideoEl.setAttribute('autoplay', '');
                    newVideoEl.style.width = '100%';
                    newVideoEl.style.height = '100%';
                    videoEl.parentNode.replaceChild(newVideoEl, videoEl);
                }
                if (peerConnections[cellIndex]) {
                    try { peerConnections[cellIndex].close(); } catch(e) {}
                    peerConnections[cellIndex] = null;
                }
                cellState.streamId = 0;
                fullscreenCellIndex = cellIndex;
                gridContainer.classList.add('fullscreen-mode');
                cell.classList.add('fullscreen');
                if (fsBtnIcon) fsBtnIcon.textContent = 'fullscreen_exit';

                // --- Исправление: сбрасываем analyticsState для HD ---
                const camId = cellState && cellState.camera && cellState.camera.id;
                if (camId && analyticsState[camId]) {
                    delete analyticsState[camId].inferredFrameW;
                    delete analyticsState[camId].inferredFrameH;
                    analyticsState[camId].forceFrameReset = Date.now();
                    delete analyticsState[camId].frame_base64;
                }
            }
            // Persist grid state change
            stateManager.updateGridState(newGrid);

            // Ensure overlay canvas stays aligned in fullscreen: when a cell becomes fullscreen
            // move its overlay canvas to fixed viewport sizing so getBoundingClientRect remains stable
            try {
                const overlayCanvas = cell.querySelector('.overlay-canvas');
                    if (overlayCanvas) {
                    if (!isCurrentlyFullscreen) {
                        // entering fullscreen - stash previous inline styles and apply fixed sizing
                        overlayCanvas.dataset._prevPosition = overlayCanvas.style.position || '';
                        overlayCanvas.dataset._prevLeft = overlayCanvas.style.left || '';
                        overlayCanvas.dataset._prevTop = overlayCanvas.style.top || '';
                        overlayCanvas.dataset._prevWidth = overlayCanvas.style.width || '';
                        overlayCanvas.dataset._prevHeight = overlayCanvas.style.height || '';
                        overlayCanvas.dataset._prevZ = overlayCanvas.style.zIndex || '';
                        overlayCanvas.style.position = 'fixed';
                        overlayCanvas.style.left = '0px';
                        overlayCanvas.style.top = '0px';
                        overlayCanvas.style.width = '100vw';
                        overlayCanvas.style.height = '100vh';
                        overlayCanvas.style.zIndex = '1001';
                        overlayCanvas.style.pointerEvents = 'none';
                        console.info('[analytics-diag] overlayCanvas pinned to fixed for fullscreen cell', cellIndex);
                    } else {
                        // leaving fullscreen - restore previous inline styles
                        overlayCanvas.style.position = overlayCanvas.dataset._prevPosition || 'absolute';
                        overlayCanvas.style.left = overlayCanvas.dataset._prevLeft || '';
                        overlayCanvas.style.top = overlayCanvas.dataset._prevTop || '';
                        overlayCanvas.style.width = overlayCanvas.dataset._prevWidth || '100%';
                        overlayCanvas.style.height = overlayCanvas.dataset._prevHeight || '100%';
                        overlayCanvas.style.zIndex = overlayCanvas.dataset._prevZ || '15';
                        try { delete overlayCanvas.dataset._prevPosition; } catch(e){}
                        try { delete overlayCanvas.dataset._prevLeft; } catch(e){}
                        try { delete overlayCanvas.dataset._prevTop; } catch(e){}
                        try { delete overlayCanvas.dataset._prevWidth; } catch(e){}
                        try { delete overlayCanvas.dataset._prevHeight; } catch(e){}
                        try { delete overlayCanvas.dataset._prevZ; } catch(e){}
                        console.info('[analytics-diag] overlayCanvas restored from fixed for cell', cellIndex);
                    }
                }
            } catch (e) {

            }

            // If analyticsState has an entry for this camera, clear inferred sizes so
            // the renderer will attempt to pick up video intrinsic resolution after
            // the stream switch. Also schedule a short delayed probe to capture
            // videoWidth/videoHeight (some players take a moment to update).
            try {
                const camId = cellState && cellState.camera && cellState.camera.id;
                if (camId && analyticsState[camId]) {
                    delete analyticsState[camId].inferredFrameW;
                    delete analyticsState[camId].inferredFrameH;
                    analyticsState[camId].forceFrameReset = Date.now();
                    setTimeout(() => {
                        try {
                            const c = gridCells[cellIndex];
                            if (!c) return;
                            const v = c.querySelector('.video-player');
                            if (v && v.videoWidth && v.videoHeight && analyticsState[camId]) {
                                analyticsState[camId].inferredFrameW = v.videoWidth;
                                analyticsState[camId].inferredFrameH = v.videoHeight;
                                console.info('[analytics-diag] delayed set inferred from video for camera', camId, v.videoWidth, v.videoHeight);
                            }
                        } catch (e) { /* ignore */ }
                    }, 450);
                }
            } catch (e) { /* ignore */ }
        }

        function updateAllStats(data) {
            if (!data || !Array.isArray(data.items)) {
                return;
            }
        
            const gridState = getGridState();
            const now = Date.now();
        
            gridState.forEach((cellState, i) => {
                const cellElement = gridCells[i];
                if (!cellElement) return;
                const statsDiv = cellElement.querySelector('.cell-stats');
                if (!statsDiv) return;
        
                if (cellState && !cellState.paused) {
                    const streamId = cellState.streamId === 0 ? 0 : 1;
                    const pathName = `cam${cellState.camera.id}_${streamId}`;
                    
                    const pathData = data.items.find(item => item.name === pathName);
        
                    if (!pathData) {
                        return;
                    }
                    
                    let bitrate = '...';

                    if (pathData.bytesReceived) {
                        const history = streamStatsHistory[pathName];

                        if (history) {
                            const timeDiffSeconds = (now - history.lastTime) / 1000;
                            const byteDiff = pathData.bytesReceived - history.lastBytes;

                            if (timeDiffSeconds > 0 && byteDiff >= 0) {
                                const bytesPerSecond = byteDiff / timeDiffSeconds;
                                bitrate = `${Math.round((bytesPerSecond * 8) / 1000)}kbps`;
                            }
                        }

                        streamStatsHistory[pathName] = {
                            lastBytes: pathData.bytesReceived,
                            lastTime: now
                        };
                    }
                    
                    const videoTrackName = pathData.tracks?.find(t => !t.startsWith('G7') && !t.startsWith('Opus'));
                    const codec = videoTrackName || '...';
                    
                    const resolution = ''; 

                    let statsText = `${codec} | ${bitrate}`;
                    if (resolution) {
                        statsText = `${codec} | ${resolution} | ${bitrate}`;
                    }
                    
                    statsDiv.textContent = statsText;
                    statsDiv.style.display = 'block';
        
                } else {
                    statsDiv.style.display = 'none';
                }
            });
        }

        function init() {
            for (let i = 0; i < MAX_GRID_SIZE; i++) {
                const cell = document.createElement('div');
                cell.className = 'grid-cell';
                cell.dataset.cellId = i;
                cell.innerHTML = `<span><i class="material-icons placeholder-icon">add_photo_alternate</i><br>${App.i18n.t('drop_camera_here')}</span>`;
                
                attachAllEvents(cell, i);
                
                gridContainer.appendChild(cell);
                gridCells.push(cell);
            }
            initializeLayoutControls();
            window.addEventListener('keydown', (e) => { if (e.key === 'Escape' && fullscreenCellIndex !== null) { toggleFullscreen(fullscreenCellIndex); } });
            window.addEventListener('language-changed', updatePlaceholdersLanguage);
            startRenderLoop();

            // Only register Electron API event if available
            if (window.api && typeof window.api.onMediamtxStatsUpdate === 'function') {
                window.api.onMediamtxStatsUpdate((statsData) => {
                    updateAllStats(statsData);
                });
            }
        }
        
        function handleAnalyticsUpdate(data) {
            const { cameraId, result } = data;

                // Lightweight diagnostics: log arrival of analytics update for this camera
                try { console.debug('[analytics] update received for', cameraId, 'status=', result && result.status); } catch(e) {}

                if (result && result.status === 'objects_detected' && result.objects) {
                    const state = analyticsState[cameraId] || {};
                    const objects = result.objects || [];

                    // update inferred frame size from incoming absolute-pixel bboxes when no frame metadata provided
                    if (!result.frame_width || !result.frame_height) {
                        // expand bbox extents to estimate source frame size
                        let inferred = state.inferredFrameSize || { minX: Infinity, minY: Infinity, maxX: 0, maxY: 0 };
                        for (const obj of objects) {
                            if (!obj || !obj.box) continue;
                            const b = obj.box;
                            // treat boxes with large numeric values as absolute pixels
                            if (typeof b.x === 'number' && typeof b.y === 'number' && typeof b.w === 'number' && typeof b.h === 'number') {
                                inferred.minX = Math.min(inferred.minX, b.x);
                                inferred.minY = Math.min(inferred.minY, b.y);
                                inferred.maxX = Math.max(inferred.maxX, b.x + b.w);
                                inferred.maxY = Math.max(inferred.maxY, b.y + b.h);
                            }
                        }
                        // if we have reasonable extents, store estimated size
                        if (inferred.maxX > 0 && inferred.maxY > 0 && inferred.minX !== Infinity) {
                            state.inferredFrameSize = inferred;
                            const estW = Math.ceil(inferred.maxX - Math.min(0, inferred.minX));
                            const estH = Math.ceil(inferred.maxY - Math.min(0, inferred.minY));
                            state.inferredFrameW = estW;
                            state.inferredFrameH = estH;
                            // Save per-stream inferred sizes (fallback for HD/SD ambiguity). If stream id is not known,
                            // populate both slots so renderer can pick a reasonable size for either stream.
                            state.inferredFrameByStream = state.inferredFrameByStream || {};
                            state.inferredFrameByStream[0] = state.inferredFrameByStream[0] || { w: estW, h: estH };
                            state.inferredFrameByStream[1] = state.inferredFrameByStream[1] || { w: estW, h: estH };
                        }
                    }

                    // Ensure timestamp is stored in milliseconds. Some analytics backends send seconds (float).
                    let tsMs = Date.now();
                    if (typeof result.timestamp === 'number') {
                        // if timestamp looks like seconds (e.g. ~1.7e9), convert to ms
                        tsMs = result.timestamp > 1e12 ? Math.floor(result.timestamp) : Math.floor(result.timestamp * 1000);
                    }
                    analyticsState[cameraId] = Object.assign(state, {
                        objects,
                        frame_width: result.frame_width,
                        frame_height: result.frame_height,
                        frame_base64: result.frame_base64,
                        timestamp: tsMs
                    });
                // Log whether overlay canvas exists in any cell that contains this camera (helpful for debugging)
                try {
                    const gridState = getGridState();
                    for (let i = 0; i < gridState.length; i++) {
                        const gs = gridState[i];
                        if (gs && gs.camera && gs.camera.id === cameraId) {
                            const cellEl = gridCells[i];
                            if (cellEl) {
                                const oc = cellEl.querySelector('.overlay-canvas');
                                console.debug('[analytics] camera', cameraId, 'cell', i, 'overlayCanvasExists=', !!oc, 'analyticsImgExists=', !!cellEl.querySelector('.analytics-img'));
                            }
                        }
                    }
                } catch (e) { /* ignore */ }
                if (result.frame_base64) {

                } else {

                }
            }
        }

        return {
            init,
            render,
            getGridState,
            updateGridLayoutView,
            updatePlaceholdersLanguage,
            handleAnalyticsUpdate
        };
    };
})(window);

// Replace direct srcObject assignment with adaptive stream attachment
// Example: when you need to show a stream in a cell
// attachStreamToCell(cell, streamUrl);
// Remove or refactor old srcObject logic for mobile compatibility