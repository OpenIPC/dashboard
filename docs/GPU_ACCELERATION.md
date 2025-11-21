# GPU Acceleration with DirectML

## Overview

The VMS Dashboard supports GPU acceleration for analytics using **DirectML** (DirectX Machine Learning). This significantly reduces CPU load and improves performance for video analytics tasks like license plate recognition, face detection, and object counting.

## 🚀 Benefits

- **60-90% reduction in CPU usage** during analytics
- **2-5x faster inference** compared to CPU-only mode
- **Lower system temperatures** and power consumption
- **Better multi-camera performance** - handle more cameras simultaneously

## 📋 Requirements

### Hardware
- **Windows 10 version 1903 or later** (Windows 11 recommended)
- **DirectX 12 compatible GPU**:
  - NVIDIA: GTX 900 series or newer
  - AMD: Radeon RX 400 series or newer
  - Intel: HD Graphics 600 series or newer (Kaby Lake+)

### Software
- **Updated GPU drivers**:
  - NVIDIA: Latest Game Ready or Studio drivers
  - AMD: Latest Adrenalin drivers
  - Intel: Latest graphics drivers from Intel website

## ⚙️ Configuration

### 1. Enable GPU Acceleration

1. Open **Settings** (⚙️ icon)
2. Go to **Analytics** tab
3. Find **"Analytics Provider (GPU/CPU)"** dropdown
4. Select one of:
   - **Auto** (recommended) - Automatically uses GPU if available
   - **GPU (DirectML)** - Forces GPU usage
   - **CPU only** - Disables GPU acceleration

### 2. Verify GPU is Active

After saving settings and enabling a module:

1. Open **Developer Tools** (F12)
2. Check **Console** for messages like:
   ```
   ✓ DirectML GPU acceleration is ENABLED and available
   → Analytics will use GPU for significantly better performance
   ```

If you see warnings about DirectML not available:
- Update your GPU drivers
- Ensure Windows 10 1903+ or Windows 11
- Check GPU supports DirectX 12

## 📊 Performance Comparison

### Example: License Plate Detection (1920x1080, 30fps)

| Mode | CPU Usage | GPU Usage | Inference Time |
|------|-----------|-----------|----------------|
| CPU only | 40-60% | 0% | ~80-120ms |
| Auto/DirectML | 10-20% | 30-50% | ~20-40ms |

### Recommendations by Camera Count

- **1-2 cameras**: CPU mode acceptable
- **3-5 cameras**: GPU recommended
- **6+ cameras**: GPU strongly recommended

## 🔧 Troubleshooting

### GPU Not Detected

**Symptoms**: Console shows "DirectML provider not available"

**Solutions**:
1. Update GPU drivers to latest version
2. Run Windows Update to get DirectX 12 updates
3. Check GPU supports DX12: `dxdiag` → Display → Feature Levels
4. Try force mode: Set provider to "GPU (DirectML)"

### High GPU Temperature

**Symptoms**: GPU temperature over 80°C

**Solutions**:
1. Improve case airflow
2. Reduce number of active cameras
3. Increase `analytics_frame_skip` in settings (analyze fewer frames)
4. Lower `analytics_resize_width` (process smaller frames)

### System Instability

**Symptoms**: Crashes or freezes during analytics

**Solutions**:
1. Update GPU drivers
2. Switch to "Auto" mode instead of forced DirectML
3. Try "CPU only" mode to isolate GPU issue
4. Check Windows Event Viewer for driver errors

## 🎯 Best Practices

1. **Start with Auto mode** - Let system decide GPU availability
2. **Monitor CPU/GPU usage** - Use built-in resource monitor
3. **Update drivers regularly** - New drivers improve stability
4. **Optimize frame processing**:
   - Set `analytics_resize_width` to 640-800 for most cameras
   - Use `analytics_frame_skip` = 3-5 to skip frames
5. **Test with one camera first** - Verify GPU works before scaling up

## 📱 Mobile/ARM Support

DirectML is **Windows-only**. For other platforms:
- **Linux**: CPU mode only (CUDA support planned)
- **macOS**: CPU mode only (Metal support planned)
- **Android**: CPU mode via NNAPI (future)

## 🔍 Technical Details

### DirectML Architecture

```
Video Frame → Resize/Preprocess → YOLO Detection (GPU) → Post-process → Results
                                          ↓
                                    OCR Recognition (GPU)
                                          ↓
                                    Format Validation (CPU)
```

### Models Using GPU

When DirectML is enabled:
- ✅ YOLO v8 License Plate Detection
- ✅ CRNN OCR Recognition
- ✅ YOLO v8 Face Detection
- ✅ YOLO v8 Object Detection/Counting

### Performance Tuning

Advanced users can tune ANPR settings for GPU optimization:

1. **Higher crop expansion** (1.3-1.5) - GPU handles larger images better
2. **Lower frame skip** (1-3) - GPU can process more frames
3. **Enable Python OCR** - GPU handles perspective correction efficiently

## 📖 Additional Resources

- [ONNX Runtime DirectML Documentation](https://onnxruntime.ai/docs/execution-providers/DirectML-ExecutionProvider.html)
- [Windows ML DirectML](https://learn.microsoft.com/en-us/windows/ai/directml/dml-intro)
- [GPU Monitoring Tools](https://www.techpowerup.com/gpuz/)

## 🆘 Support

If GPU acceleration isn't working:

1. Collect logs from Developer Tools (F12) → Console
2. Run `dxdiag` and save output
3. Check GPU model and driver version
4. Report issue with:
   - Windows version
   - GPU model
   - Driver version
   - Console logs showing DirectML error

---

**Last Updated**: November 2025  
**Version**: 0.1.2
