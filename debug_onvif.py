import requests
import time

def debug_camera(ip, port, user, passw, token):
    # 1. Check Imaging Service Options
    print(f"\n--- Checking Imaging Service Options ---")
    # We need to find the Imaging Service URL first, but we'll guess or use the one from logs
    # Logs said: http://192.168.3.11:8899/onvif/imaging
    imaging_url = f"http://{ip}:{port}/onvif/imaging"
    
    body_opts = f"""<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope">
      <s:Body xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
        <GetOptions xmlns="http://www.onvif.org/ver20/imaging/wsdl">
          <VideoSourceToken>000</VideoSourceToken>
        </GetOptions>
      </s:Body>
    </s:Envelope>"""
    
    try:
        resp = requests.post(imaging_url, data=body_opts, auth=(user, passw), timeout=5, proxies={"http": None, "https": None})
        print(f"GetOptions Status: {resp.status_code}")
        print(f"GetOptions Body: {resp.text}")
    except Exception as e:
        print(f"GetOptions Error: {e}")

    # 2. Try Relative Move (Imaging)
    print(f"\n--- Testing Relative Move (Imaging) ---")
    body_rel = f"""<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope">
      <s:Body xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
        <Move xmlns="http://www.onvif.org/ver20/imaging/wsdl">
          <VideoSourceToken>000</VideoSourceToken>
          <Focus>
            <Relative>
              <Distance>0.1</Distance>
              <Speed>1.0</Speed>
            </Relative>
          </Focus>
        </Move>
      </s:Body>
    </s:Envelope>"""
    
    try:
        resp = requests.post(imaging_url, data=body_rel, auth=(user, passw), timeout=5, proxies={"http": None, "https": None})
        print(f"Relative Move Status: {resp.status_code}")
        print(f"Relative Move Body: {resp.text}")
    except Exception as e:
        print(f"Relative Move Error: {e}")

debug_camera("192.168.3.11", 8899, "admin", "USSKot125@", "000")
