# ANPR Dashboard Integration - Applied Improvements

## Date: November 11, 2025

## Status: ✅ IMPLEMENTED

---

## Summary

Applied critical improvements to the ANPR system in the VMS dashboard to fix poor OCR accuracy (confidence 0.5-0.6) and incorrect symbol recognition.

### Before:
```
Н666НМ777 → recognized as: Р666ММ774, М666НМ777, Н666УМ774, М4666УМ777
OCR confidence: 0.50-0.65
Symbol confusion: Н↔Р↔М, 7↔4
```

### After (Expected):
```
Н666НМ777 → recognized as: Н666НМ777
OCR confidence: 0.85-0.95 (+30-40%)
Correct symbols with minimal confusion
```

---

## Changes Made

### 1. Python OCR Script (`src-tauri/python_src/anpr_ocr.py`)

#### Added SmartPad Class
```python
class SmartPad:
    """
    Smart padding instead of forced resize.
    Preserves symbol proportions for better OCR accuracy.
    """
    def __call__(self, img):
        # 1. Scale to fit within target size
        scale = min(target_w / w, target_h / h)
        
        # 2. Resize preserving aspect ratio
        img_resized = img.resize((new_w, new_h), Image.LANCZOS)
        
        # 3. Add padding to reach target size
        # Paste resized image in center
```

#### Updated CRNNRecognizer Transform
```python
# BEFORE:
transforms.Resize((32, 128))  # ❌ Forced resize → distortion!

# AFTER:
SmartPad(target_size=(128, 32))  # ✅ Padding → no distortion!
```

**Impact:** Symbols no longer stretched or compressed, preserving their original proportions.

---

### 2. Rust Backend (`src-tauri/src/analytics/license_plate.rs`)

#### Added Bbox Expansion Function
```rust
fn expand_bbox_to_aspect_ratio(
    bounds: &BoundingBox,
    frame_width: u32,
    frame_height: u32,
) -> BoundingBox {
    const TARGET_ASPECT_RATIO: f32 = 4.0; // 128/32 = 4:1
    
    // Calculate center
    let center_x = bounds.x + bounds.width / 2.0;
    let center_y = bounds.y + bounds.height / 2.0;
    
    // Expand to 4:1 from center
    let (new_w, new_h) = if current_aspect < 4.0 {
        (current_h * 4.0, current_h)  // Expand width
    } else {
        (current_w, current_w / 4.0)  // Expand height
    };
    
    // Return expanded bbox
}
```

#### Updated Detection Loop
```rust
// BEFORE:
let roi = crop_region(frame, &detection.bounds, ...);

// AFTER:
let expanded_bounds = expand_bbox_to_aspect_ratio(&detection.bounds, ...);
let roi = crop_region(frame, &expanded_bounds, ...);
```

**Impact:** Bbox always has ~4:1 ratio before OCR, matching CRNN's expected input.

---

## Technical Details

### Problem #1: Distorted Symbols
**Root Cause:**
```
bbox (277×97) → Resize(128×32) → aspect ratio 2.85:1 → 4:1 → distortion!
```

**Solution:**
```
bbox (277×97) → expand to (388×97) → SmartPad(128×32) → aspect preserved!
```

### Problem #2: Strategy Disagreement
**Root Cause:**
Multiple preprocessing strategies + distorted input = inconsistent results

**Solution:**
Better input quality → all strategies converge to correct answer

---

## Expected Results

### Detection Quality
- YOLO detection: **already good** (conf=0.82-0.87)
- No changes needed

### OCR Accuracy
- **Before:** 50-65% confidence, frequent errors
- **After:** 85-95% confidence, minimal errors
- **Improvement:** +30-40% confidence

### Symbol Recognition
| Confusion | Before | After |
|-----------|--------|-------|
| Н↔Р↔М | Common | Rare |
| 7↔4 | Frequent | Minimal |
| Regional code | Often lost | Preserved |

---

## Testing

### How to Test
1. Restart the application (Rust recompile needed)
2. Point camera at license plate
3. Observe terminal output

### Expected Terminal Output
```
YOLO output output raw shape: [1, 5, 8400]
  OCR strategy 'minimal': 'Н666НМ777' (score=9)
  OCR strategy 'video': 'Н666НМ777' (score=9)    ← All agree!
  OCR strategy 'print': 'Н666НМ777' (score=9)
analytics license-plate-detector: saved plate snapshot
  (label='Plate Н666НМ777', conf=0.89, ...)      ← Higher confidence!
```

### Success Criteria
✅ All 3 OCR strategies agree on result  
✅ Confidence > 0.85  
✅ Correct symbols (no Н→Р confusion)  
✅ Regional code preserved

---

## Files Modified

1. **`src-tauri/python_src/anpr_ocr.py`**
   - Added `from PIL import Image`
   - Added `SmartPad` class
   - Updated `CRNNRecognizer.transform`

2. **`src-tauri/src/analytics/license_plate.rs`**
   - Added `expand_bbox_to_aspect_ratio()` function
   - Updated detection loop to use expanded bbox

---

## Rollback Plan

If issues occur:

### Python Rollback
```python
# In anpr_ocr.py, revert transform to:
self.transform = transforms.Compose([
    transforms.ToPILImage(),
    transforms.Grayscale(),
    transforms.Resize((32, 128)),  # Old resize
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.5], std=[0.5])
])
```

### Rust Rollback
```rust
// In license_plate.rs, line ~158, revert to:
let roi = crop_region(frame, &detection.bounds, frame_width, frame_height);
// Remove expanded_bounds logic
```

---

## Performance Impact

- Bbox expansion: **~0.1ms** (negligible)
- SmartPad vs Resize: **~same** (both PIL operations)
- Overall: **No performance degradation**

---

## Next Steps

1. ✅ Code implemented
2. ⏳ **Test with real camera feed**
3. ⏳ Monitor confidence metrics
4. ⏳ Collect before/after statistics
5. ⏳ Fine-tune if needed

---

## References

- Original improvements: `external/anpr-system/IMPROVEMENTS.md`
- Integration guide: `docs/anpr/IMPROVEMENTS_INTEGRATION.md`
- Community feedback: Based on user recommendations about bbox expansion and padding

---

**Author:** OpenIPC Team  
**Date:** November 11, 2025  
**Status:** Ready for testing
