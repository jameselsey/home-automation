ESP32 CAM to Home Assistant
===
This is a simple demo of using the ESP32-CAM to send images to Home Assistant. 
The ESP32-CAM is a low-cost camera module with built-in Wi-Fi, making it ideal for IoT projects.

This demo will show how you can take an image from the camera, and send that to a custom API, which in my case is a simple Flask app running on the same machine as my home assistant.

This would normally be very easy using something like ESPHome, but my requirements were a bit more complex because I'm running this on batteries, so have to enter deep sleep, so I can't keep streaming.

## How to run
Just change the wifi creds in the code, and the URL to your API, then deploy it to your ESP32 CAM

Heres the python API just for reference

```python
from flask import Flask, request, send_file, make_response
import os
from datetime import datetime


app = Flask(__name__)

@app.route('/mailbox', methods=['GET'])
def get_mailbox_image():
    image_path = "/data/mailbox.jpg"
    if os.path.exists(image_path):
        response = make_response(send_file(image_path, mimetype='image/jpeg'))
        response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
        response.headers['Pragma'] = 'no-cache'
        response.headers['Expires'] = '0'
        return response
    else:
        return "Image not found", 404



@app.route('/mailbox', methods=['POST'])
def mailbox():
    try:
        image_data = request.get_data()
        with open("/data/mailbox.jpg", "wb") as f:
            f.write(image_data)
        print(f"Saved {len(image_data)} bytes")
        return "OK", 200
    except Exception as e:
        return f"Error: {str(e)}", 500


@app.route('/health', methods=['GET'])
def health_check():
    return "OK", 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001)


```
