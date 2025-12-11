import os
from ultralytics import YOLO

# Define paths
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
input_path = os.path.join(project_root, "external", "yolo11n_ object .pt")
output_path = os.path.join(project_root, "external", "yolo11n.onnx")

# Check if input file exists
if not os.path.exists(input_path):
    print(f"Error: Input file not found at {input_path}")
    exit(1)

print(f"Loading model from {input_path}...")
try:
    model = YOLO(input_path)
    print("Model loaded successfully.")

    print("Exporting to ONNX...")
    # Export the model
    # opset=12 is usually a safe bet for compatibility, or 11. 
    # dynamic=False is often better for specific hardware acceleration like DirectML if input size is fixed.
    # The current code uses 640x640.
    path = model.export(format="onnx", imgsz=640, opset=12)
    print(f"Model exported successfully to {path}")
    
    # Rename if necessary (YOLO export usually names it same as input but with .onnx)
    # The export method returns the path to the exported file.
    
except Exception as e:
    print(f"An error occurred: {e}")
    exit(1)
