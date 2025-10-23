import onnxruntime as ort, os
print('onnxruntime version', ort.__version__)
print('available providers', ort.get_available_providers())
try:
    p = os.path.dirname(ort.__file__)
    print('onnxruntime package path', p)
    for root, dirs, files in os.walk(p):
        for f in files:
            if 'dml' in f.lower() or 'dml' in root.lower():
                print('file with dml in name:', os.path.join(root,f))
except Exception as e:
    print('error inspecting ort package', e)
