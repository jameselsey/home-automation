#include <Bluepad32.h>
#include <ESP32Servo.h>

// Servo control pins
#define PAN_SERVO_PIN 16
#define TILT_SERVO_PIN 17

// Angle limits
#define PAN_MIN_ANGLE   0
#define PAN_MAX_ANGLE   180
#define TILT_MIN_ANGLE  45
#define TILT_MAX_ANGLE  135

#define PAN_STOP_ANGLE 94
#define TILT_STOP_ANGLE 94

int panAngle = 90;   // Start centered
int tiltAngle = 90;

Servo panServo;
Servo tiltServo;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS] = {};

void setup() {
  Serial.begin(115200);

  panServo.setPeriodHertz(50);
  tiltServo.setPeriodHertz(50);

  panServo.attach(PAN_SERVO_PIN, 500, 2400);   // min/max pulse width in µs
  tiltServo.attach(TILT_SERVO_PIN, 500, 2400);
  
  panServo.write(PAN_STOP_ANGLE);
  tiltServo.write(TILT_STOP_ANGLE); 
  
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
  BP32.forgetBluetoothKeys();
}

void loop() {
  BP32.update();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    GamepadPtr gp = myGamepads[i];
    if (!gp || !gp->isConnected())
      continue;

    int joyX = gp->axisX();  // Left stick X (pan)
    int joyY = gp->axisY();  // Left stick Y (tilt)
    int deadzone = 50;

    // For continuous servo:
    int panSpeed = PAN_STOP_ANGLE;
    if (abs(joyX) > deadzone) {
      panSpeed = map(joyX, 512, -512, 60, 120);  // 90 = stop
      panServo.write(panSpeed);
    } else {
      panServo.write(panSpeed);  // Stop
    }

    // Jog tilt
    int tiltSpeed = TILT_STOP_ANGLE;
    if (abs(joyY) > deadzone) {
      tiltSpeed = map(joyY, 512, -512, 60, 120);  // 90 = stop
      tiltServo.write(tiltSpeed);
    } else {
      tiltServo.write(tiltSpeed);  // Stop
    }

    Serial.print("Pan Axis: ");
    Serial.print(joyX);
    Serial.print(" Pan: ");
    Serial.print(panSpeed);
    Serial.print(" | Tilt: ");
    Serial.println(tiltAngle);
  }

  delay(20);  // smooth motion
}

void onConnectedGamepad(GamepadPtr gp) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == nullptr) {
      myGamepads[i] = gp;
      Serial.println("Gamepad connected");
      break;
    }
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == gp) {
      myGamepads[i] = nullptr;
      Serial.println("Gamepad disconnected");
      break;
    }
  }
}
