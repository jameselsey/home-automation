#include <Bluepad32.h>

// Define motor control pins
#define ENA 14
#define IN1 27
#define IN2 26

#define ENB 25
#define IN3 33
#define IN4 32

// Define PWM channels
#define PWM_CHANNEL_LEFT 0
#define PWM_CHANNEL_RIGHT 1
#define PWM_FREQ 1000
#define PWM_RESOLUTION 8

GamepadPtr myGamepads[BP32_MAX_GAMEPADS] = {};

void setup() {
  Serial.begin(115200);

  // Initialize motor control pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Setup PWM for motor speed control
  ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(ENA, PWM_CHANNEL_LEFT);

  ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(ENB, PWM_CHANNEL_RIGHT);

  // Initialize Bluepad32
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
  BP32.forgetBluetoothKeys();
}

void loop() {
  BP32.update();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    GamepadPtr gp = myGamepads[i];
    if (!gp || !gp->isConnected())
      continue;

    // Read joystick values
    int leftY = gp->axisY();   // Left stick Y-axis
    int rightY = gp->axisRY(); // Right stick Y-axis

    // Map joystick values to motor power (-255 to 255)
    int leftPower = map(leftY, -512, 512, 255, -255);
    int rightPower = map(rightY, -512, 512, 255, -255);

    // Debug logging
    Serial.print("Left Stick: ");
    Serial.print(leftY);
    Serial.print(" => Left Power: ");
    Serial.print(leftPower);

    Serial.print(" | Right Stick: ");
    Serial.print(rightY);
    Serial.print(" => Right Power: ");
    Serial.println(rightPower);

    // Control motors
    driveMotor(PWM_CHANNEL_LEFT, IN1, IN2, leftPower);
    driveMotor(PWM_CHANNEL_RIGHT, IN3, IN4, rightPower);
  }

  delay(100); // Slight delay for readability in serial output
}


void driveMotor(int pwmChannel, int inPin1, int inPin2, int power) {
  if (power > 15) {
    digitalWrite(inPin1, HIGH);
    digitalWrite(inPin2, LOW);
    ledcWrite(pwmChannel, power);
  } else if (power < -15) {
    digitalWrite(inPin1, LOW);
    digitalWrite(inPin2, HIGH);
    ledcWrite(pwmChannel, -power);
  } else {
    digitalWrite(inPin1, LOW);
    digitalWrite(inPin2, LOW);
    ledcWrite(pwmChannel, 0);
  }
}

void onConnectedGamepad(GamepadPtr gp) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == nullptr) {
      myGamepads[i] = gp;
      break;
    }
  }
}

void onDisconnectedGamepad(GamepadPtr gp) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myGamepads[i] == gp) {
      myGamepads[i] = nullptr;
      break;
    }
  }
}
