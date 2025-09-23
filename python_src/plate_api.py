from flask import Flask, request, jsonify

app = Flask(__name__)


@app.route('/recognize_plate', methods=['POST'])
def recognize_plate():
    """
    Endpoint to run plate recognition on a given RTSP URL.
    Requires the request JSON to include:
      - rtsp_url: string
      - user_initiated: boolean (must be True)

    This prevents accidental or automatic execution when the app starts.
    """
    data = request.get_json() or {}
    rtsp_url = data.get('rtsp_url')
    user_initiated = data.get('user_initiated', False)

    if not rtsp_url:
        return jsonify({'error': 'rtsp_url is required'}), 400

    if not user_initiated:
        return jsonify({'error': 'recognition must be user-initiated'}), 400

    # Lazy import heavy recognizer only when endpoint is called by user
    try:
        from plate_recognizer import recognize_plate_from_rtsp
    except Exception as e:
        return jsonify({'error': 'failed to load recognizer', 'details': str(e)}), 500

    # Можно добавить обработку других параметров (frame_skip и т.д.)
    results = recognize_plate_from_rtsp(rtsp_url)
    return jsonify({'results': results})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001, debug=True)
