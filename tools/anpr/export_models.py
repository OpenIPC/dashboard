"""Utility script to export ANPR models from Runoi/ANPR-System formats into ONNX for ONNX Runtime.

Steps performed:
- Export YOLOv8 detector `.pt` weights to ONNX via Ultralytics API
- Export quantized CRNN OCR PyTorch checkpoint to ONNX with static input shape (1 x 1 x 32 x 128)

Usage:
    python export_models.py --repo-path ../external/anpr-system \
        --yolo-weights models/yolo/model/best.pt \
        --ocr-weights models/ocr_crnn/quant/crnn_ocr_model_int8_fx.pth \
        --out-dir ../../artifacts/anpr
"""

import argparse
from pathlib import Path

import torch
from torch import nn
from ultralytics import YOLO


class CRNN(nn.Module):
    """Minimal CRNN definition matching Runoi/ANPR-System architecture."""

    def __init__(self, num_classes: int = 36):
        super().__init__()
        self.cnn = nn.Sequential(
            nn.Conv2d(1, 64, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.MaxPool2d(2, 2),
            nn.Conv2d(64, 128, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.MaxPool2d(2, 2),
            nn.Conv2d(128, 256, kernel_size=3, padding=1),
            nn.BatchNorm2d(256),
            nn.ReLU(inplace=True),
            nn.Conv2d(256, 256, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.MaxPool2d((2, 1), (2, 1)),
            nn.Conv2d(256, 512, kernel_size=3, padding=1),
            nn.BatchNorm2d(512),
            nn.ReLU(inplace=True),
            nn.Conv2d(512, 512, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.MaxPool2d((2, 1), (2, 1)),
        )
        self.rnn = nn.LSTM(512 * 2, 256, bidirectional=True, num_layers=2, batch_first=True)
        self.classifier = nn.Linear(512, num_classes)

    def forward(self, x):
        x = self.cnn(x)
        batch, channels, height, width = x.size()
        x = x.reshape(batch, channels * height, width)
        x = x.permute(0, 2, 1)
        x, _ = self.rnn(x)
        x = self.classifier(x)
        x = x.permute(1, 0, 2)
        return torch.nn.functional.log_softmax(x, dim=2)


def export_yolo_pt_to_onnx(weights_path: Path, out_path: Path):
    model = YOLO(str(weights_path))
    export_result = model.export(
        format="onnx",
        imgsz=640,
        simplify=True,
        dynamic=False,
        opset=12,
    )
    exported_path = None
    if isinstance(export_result, (list, tuple)):
        for candidate in export_result:
            candidate_path = Path(candidate)
            if candidate_path.suffix == ".onnx" and candidate_path.is_file():
                exported_path = candidate_path
                break
    elif isinstance(export_result, (str, Path)):
        candidate_path = Path(export_result)
        if candidate_path.suffix == ".onnx" and candidate_path.is_file():
            exported_path = candidate_path
        elif candidate_path.is_dir():
            potential = list(candidate_path.glob("*.onnx"))
            if potential:
                exported_path = potential[0]

    if exported_path is None:
        raise RuntimeError(f"Ultralytics export completed but no ONNX file was produced: {export_result}")

    exported_path.replace(out_path)
    print(f"YOLO ONNX saved to {out_path}")


def export_crnn_to_onnx(weights_path: Path, out_path: Path):
    state = torch.load(weights_path, map_location="cpu")
    if any("_packed_params" in key or key.endswith(".scale") for key in state):
        raise ValueError(
            "Quantized CRNN checkpoints are not supported. Provide the float checkpoint, e.g. models/ocr_crnn/model/crnn_ocr_model_best.pth"
        )
    num_classes = state.get("classifier.weight", torch.empty((36,))).shape[0]  # fallback keeps default shape
    model = CRNN(num_classes=num_classes)
    model.load_state_dict(state)
    model.eval()

    dummy = torch.randn(1, 1, 32, 128)
    torch.onnx.export(
        model,
        dummy,
        str(out_path),
        opset_version=13,
        input_names=["input"],
        output_names=["logits"],
        dynamic_axes={},
    )
    print(f"CRNN ONNX saved to {out_path}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-path", type=Path, required=True)
    parser.add_argument("--yolo-weights", type=Path, default=Path("models/yolo/model/best.pt"))
    parser.add_argument(
        "--ocr-weights",
        type=Path,
        default=Path("models/ocr_crnn/model/crnn_ocr_model_best.pth"),
        help="Path to the float32 CRNN checkpoint. Quantized FX checkpoints are not yet supported.",
    )
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()

    repo_path = args.repo_path.resolve()
    yolo_weights = (repo_path / args.yolo_weights).resolve()
    ocr_weights = (repo_path / args.ocr_weights).resolve()
    out_dir = args.out_dir.resolve()

    out_dir.mkdir(parents=True, exist_ok=True)

    export_yolo_pt_to_onnx(yolo_weights, out_dir / "anpr_yolov8.onnx")
    export_crnn_to_onnx(ocr_weights, out_dir / "anpr_crnn.onnx")


if __name__ == "__main__":
    main()
