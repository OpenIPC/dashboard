import time
import numpy as np
import onnxruntime as ort
import sys

print('onnxruntime version', ort.__version__)
print('available providers', ort.get_available_providers())

model_path = 'python_src/license_plate_detection.onnx'

# Try to inspect model input shape via CPU session if file exists
try:
    s_cpu = ort.InferenceSession(model_path, providers=['CPUExecutionProvider'])
    inp = s_cpu.get_inputs()[0]
    model_input_shape = tuple([1 if d is None else d for d in inp.shape])
    print('model input shape from file:', inp.shape, 'interpreted as', model_input_shape)
except Exception as e:
    print('Could not load model file or inspect inputs:', e)
    # default
    model_input_shape = (1, 3, 240, 320)


def bench(prefer_dml=False):
    providers = ['DmlExecutionProvider', 'CPUExecutionProvider'] if prefer_dml else ['CPUExecutionProvider']
    print('\ncreating session prefer_dml=%s providers=%s' % (prefer_dml, providers))
    try:
        s = ort.InferenceSession(model_path, providers=providers)
    except Exception as e:
        print('session create failed:', e)
        return
    print('session providers in use:', s.get_providers())
    # prepare dummy input based on model
    inp = s.get_inputs()[0]
    in_shape = tuple([1 if d is None else d for d in inp.shape])
    x = np.random.randn(*in_shape).astype(np.float32)
    # warmup
    for _ in range(3):
        s.run(None, {inp.name: x})
    # time
    t0 = time.time()
    n = 20
    for _ in range(n):
        s.run(None, {inp.name: x})
    t = (time.time() - t0) / n * 1000
    print('avg infer ms:', t)


bench(False)
bench(True)

print('\nDone')
