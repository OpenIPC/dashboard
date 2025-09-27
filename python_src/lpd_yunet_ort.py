import numpy as np
import cv2 as cv
import onnxruntime as ort


class LPD_YuNetORT:
    """
    ONNX Runtime (DirectML when available) wrapper for the YuNet license-plate detector.

    API intentionally mirrors the existing OpenCV-based `LPD_YuNet` so it can be
    swapped in for testing/acceleration. The infer() method expects an image already
    resized to `input_size` (width, height) like the original class.
    """

    def __init__(self, modelPath, inputSize=[320, 240], confThreshold=0.8, nmsThreshold=0.3,
                 topK=5000, keepTopK=750, prefer_dml=True):
        self.model_path = modelPath
        self.input_size = np.array(inputSize)
        self.confidence_threshold = confThreshold
        self.nms_threshold = nmsThreshold
        self.top_k = topK
        self.keep_top_k = keepTopK

        self.output_names = ['loc', 'conf', 'iou']
        self.min_sizes = [[10, 16, 24], [32, 48], [64, 96], [128, 192, 256]]
        self.steps = [8, 16, 32, 64]
        self.variance = [0.1, 0.2]

        # Try to create an ONNX Runtime session with DirectML (DML) if available.
        providers = ort.get_available_providers()
        # Log available providers for diagnostics
        try:
            print('[LP-ORT] onnxruntime available providers:', providers)
        except Exception:
            pass
        chosen_providers = None
        # Be tolerant for various provider name spellings across ONNX builds
        dml_candidates = ['DmlExecutionProvider', 'DMLExecutionProvider']
        selected_dml = None
        if prefer_dml:
            for c in dml_candidates:
                if c in providers:
                    selected_dml = c
                    break
        if selected_dml:
            chosen_providers = [selected_dml, 'CPUExecutionProvider']
        else:
            chosen_providers = ['CPUExecutionProvider']
        try:
            print('[LP-ORT] Chosen providers for session creation:', chosen_providers)
        except Exception:
            pass

        so = ort.SessionOptions()
        so.graph_optimization_level = ort.GraphOptimizationLevel.ORT_ENABLE_ALL

        try:
            self.session = ort.InferenceSession(self.model_path, sess_options=so, providers=chosen_providers)
        except Exception as e:
            # fallback to CPU provider alone
            self.session = ort.InferenceSession(self.model_path, sess_options=so, providers=['CPUExecutionProvider'])

        # Report the provider(s) actually in use by the session
        try:
            actual = self.session.get_providers()
            print('[LP-ORT] ONNX Runtime session providers in use:', actual)
        except Exception:
            pass

        # input name
        self.input_name = self.session.get_inputs()[0].name

        # Determine model's expected/static input size (width, height) if available
        try:
            input_shape = self.session.get_inputs()[0].shape  # e.g. [1, 3, H, W]
            # Get last two dims as H, W if they are integers
            h_dim = input_shape[-2]
            w_dim = input_shape[-1]
            if isinstance(h_dim, int) and isinstance(w_dim, int):
                self.model_input_size = np.array([int(w_dim), int(h_dim)])
            else:
                # fallback to provided inputSize
                self.model_input_size = np.array(inputSize)
        except Exception:
            self.model_input_size = np.array(inputSize)

        # Initialize effective input_size to the model's required size
        # Priors/scale are generated based on this size to match model expectations
        self.input_size = np.array(self.model_input_size)

        # generate anchors/priorboxes
        self._priorGen()

    @property
    def name(self):
        return self.__class__.__name__

    def setBackendAndTarget(self, backendId, targetId):
        # No-op for ONNX Runtime wrapper — backend selection happens at init via providers.
        pass

    def setInputSize(self, inputSize):
        # Allow callers to request a different input size, but if the ONNX model
        # has a static required input shape we must not change that — warn instead.
        try:
            requested = np.array(inputSize)
        except Exception:
            print('[LP-ORT] Warning: invalid inputSize provided to setInputSize:', inputSize)
            return

        # If model_input_size was inferred from the ONNX graph and differs, warn and keep it
        try:
            if hasattr(self, 'model_input_size') and not np.array_equal(self.model_input_size, requested):
                print(f"[LP-ORT] Warning: ONNX model expects fixed input size {self.model_input_size.tolist()}, ignoring requested {requested.tolist()}")
                # keep self.input_size aligned with model_input_size
                self.input_size = np.array(self.model_input_size)
            else:
                self.input_size = requested
        except Exception:
            self.input_size = requested

        # regenerate priors based on effective input_size
        self._priorGen()

    def _preprocess(self, image):
        # Expect image in HxWxC BGR (OpenCV default). Match original OpenCV blob behavior:
        # swapRB=True (BGR->RGB), no mean or scale applied.
        img = image.astype(np.float32)
        # BGR to RGB
        img = img[..., ::-1]
        # transpose to NCHW
        img = np.transpose(img, (2, 0, 1))
        img = np.expand_dims(img, 0)
        return img

    def infer(self, image):
        # If incoming image doesn't match the configured input_size, resize it.
        # Use the ONNX model's required input size when available; otherwise fall back
        # to the configured input_size.
        try:
            expected = getattr(self, 'model_input_size', None)
            if expected is None:
                expected = self.input_size
            expected_w = int(expected[0])
            expected_h = int(expected[1])
        except Exception:
            expected_w = image.shape[1]
            expected_h = image.shape[0]

        if image.shape[0] != expected_h or image.shape[1] != expected_w:
            try:
                print(f'[LP-ORT] Warning: input image size {image.shape[1]}x{image.shape[0]} does not match model expected {expected_w}x{expected_h}. Resizing input.')
            except Exception:
                pass
            image = cv.resize(image, (expected_w, expected_h))

        input_blob = self._preprocess(image)
        # Run inference
        try:
            outputs = self.session.run(self.output_names, {self.input_name: input_blob})
        except Exception:
            # fallback: run and let runtime return outputs (order may vary)
            outputs = self.session.run(None, {self.input_name: input_blob})

        # Outputs should be [loc, conf, iou]
        if isinstance(outputs, list) and len(outputs) == 3:
            loc, conf, iou = outputs
        else:
            # Try to map by name
            out_map = {o.name: outputs[idx] for idx, o in enumerate(self.session.get_outputs())}
            loc = out_map.get('loc') or out_map.get('loc_out') or outputs[0]
            conf = out_map.get('conf') or out_map.get('conf_out') or outputs[1]
            iou = out_map.get('iou') or out_map.get('iou_out') or outputs[2]

        # Normalize shapes into expected 2D arrays (num_priors x channels)
        loc = np.array(loc)
        conf = np.array(conf)
        iou = np.array(iou)

        # remove leading batch dim if present
        if loc.ndim > 1 and loc.shape[0] == 1:
            loc = loc[0]
        if conf.ndim > 1 and conf.shape[0] == 1:
            conf = conf[0]
        if iou.ndim > 1 and iou.shape[0] == 1:
            iou = iou[0]

        # Postprocess
        results = self._postprocess((loc, conf, iou))
        return results

    def _postprocess(self, blob):
        loc, conf, iou = blob

        # ensure numpy arrays
        loc = np.array(loc)
        conf = np.array(conf)
        iou = np.array(iou)

        # get score: handle various output shapes robustly
        if conf.ndim == 1:
            cls_scores = conf
        else:
            # if second dim exists and represents class scores, pick index 1 as positive class
            if conf.shape[1] == 1:
                cls_scores = conf[:, 0]
            else:
                cls_scores = conf[:, 1]

        if iou.ndim == 1:
            iou_scores = iou
        else:
            iou_scores = iou[:, 0]

        # clamp
        iou_scores = np.clip(iou_scores, 0.0, 1.0)
        scores = np.sqrt(cls_scores * iou_scores)
        scores = scores[:, np.newaxis]

        scale = self.input_size

        # get four corner points for bounding box
        bboxes = np.hstack((
            (self.priors[:, 0:2] + loc[:,  4: 6] * self.variance[0] * self.priors[:, 2:4]) * scale,
            (self.priors[:, 0:2] + loc[:,  6: 8] * self.variance[0] * self.priors[:, 2:4]) * scale,
            (self.priors[:, 0:2] + loc[:, 10:12] * self.variance[0] * self.priors[:, 2:4]) * scale,
            (self.priors[:, 0:2] + loc[:, 12:14] * self.variance[0] * self.priors[:, 2:4]) * scale
        ))

        dets = np.hstack((bboxes, scores))

        # Use OpenCV NMS (same as original implementation) for consistent results
        keepIdx = cv.dnn.NMSBoxes(
            bboxes=dets[:, 0:4].tolist(),
            scores=dets[:, -1].tolist(),
            score_threshold=self.confidence_threshold,
            nms_threshold=self.nms_threshold,
            top_k=self.top_k
        )

        if len(keepIdx) > 0:
            dets = dets[keepIdx]
            return dets[:self.keep_top_k]
        else:
            return np.empty(shape=(0, 9))

    def _priorGen(self):
        w, h = self.input_size
        feature_map_2th = [int(int((h + 1) / 2) / 2),
                           int(int((w + 1) / 2) / 2)]
        feature_map_3th = [int(feature_map_2th[0] / 2),
                           int(feature_map_2th[1] / 2)]
        feature_map_4th = [int(feature_map_3th[0] / 2),
                           int(feature_map_3th[1] / 2)]
        feature_map_5th = [int(feature_map_4th[0] / 2),
                           int(feature_map_4th[1] / 2)]
        feature_map_6th = [int(feature_map_5th[0] / 2),
                           int(feature_map_5th[1] / 2)]

        feature_maps = [feature_map_3th, feature_map_4th,
                        feature_map_5th, feature_map_6th]

        priors = []
        for k, f in enumerate(feature_maps):
            min_sizes = self.min_sizes[k]
            for i in range(f[0]):
                for j in range(f[1]):
                    for min_size in min_sizes:
                        s_kx = min_size / w
                        s_ky = min_size / h

                        cx = (j + 0.5) * self.steps[k] / w
                        cy = (i + 0.5) * self.steps[k] / h

                        priors.append([cx, cy, s_kx, s_ky])
        self.priors = np.array(priors, dtype=np.float32)
