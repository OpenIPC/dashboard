import json
import sys

def probe():
    try:
        import onnxruntime as ort
        providers = ort.get_available_providers()
        # We can't reliably create a session without a model on all platforms; report available providers only
        print(json.dumps({"providers": providers, "session_providers": []}))
        return 0
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        return 2


if __name__ == '__main__':
    sys.exit(probe())
