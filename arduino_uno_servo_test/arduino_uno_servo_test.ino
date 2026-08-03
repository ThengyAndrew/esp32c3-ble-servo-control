#include <Servo.h>

// Arduino Uno 舵机输出脚。D9 使用 Servo 库的 Timer1 定时脉冲，
// 与 ESP32-C3 的 LEDC 无关，可用于独立验证舵机、供电和接线。
constexpr uint8_t SERVO_PIN = 9;
constexpr int MIN_ANGLE = 0;
constexpr int MAX_ANGLE = 180;
constexpr int MIN_PULSE_US = 500;
constexpr int MAX_PULSE_US = 2400;
constexpr uint16_t SWEEP_STEP_DELAY_MS = 15;

Servo servo;
int currentAngle = 0;
char commandBuffer[24];
uint8_t commandLength = 0;

void reportStatus() {
  const int pulseUs = map(currentAngle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
  Serial.print(F("STATUS:ANGLE="));
  Serial.print(currentAngle);
  Serial.print(F(";PULSE_US="));
  Serial.print(pulseUs);
  Serial.println(F(";PIN=D9;FREQ=50"));
}

bool setServoAngle(int angle) {
  if (angle < MIN_ANGLE || angle > MAX_ANGLE) {
    Serial.println(F("ERROR:ANGLE_OUT_OF_RANGE"));
    return false;
  }

  currentAngle = angle;
  const int pulseUs = map(currentAngle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
  servo.writeMicroseconds(pulseUs);
  Serial.print(F("ANGLE:"));
  Serial.println(currentAngle);
  reportStatus();
  return true;
}

bool parseAngle(const char *text, int &angle) {
  if (text[0] == '\0') return false;
  long value = 0;
  for (const char *cursor = text; *cursor != '\0'; ++cursor) {
    if (*cursor < '0' || *cursor > '9') return false;
    value = value * 10 + (*cursor - '0');
    if (value > MAX_ANGLE) return false;
  }
  angle = static_cast<int>(value);
  return true;
}

void sweepTest() {
  Serial.println(F("TEST:SWEEP_0_TO_180"));
  for (int angle = MIN_ANGLE; angle <= MAX_ANGLE; ++angle) {
    const int pulseUs = map(angle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
    servo.writeMicroseconds(pulseUs);
    currentAngle = angle;
    delay(SWEEP_STEP_DELAY_MS);
  }
  Serial.println(F("TEST:SWEEP_180_TO_0"));
  for (int angle = MAX_ANGLE; angle >= MIN_ANGLE; --angle) {
    const int pulseUs = map(angle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
    servo.writeMicroseconds(pulseUs);
    currentAngle = angle;
    delay(SWEEP_STEP_DELAY_MS);
  }
  reportStatus();
  Serial.println(F("TEST:DONE"));
}

void handleCommand(char *command) {
  if (strcmp(command, "STATUS") == 0) {
    reportStatus();
    return;
  }
  if (strcmp(command, "TEST") == 0) {
    sweepTest();
    return;
  }
  const char anglePrefix[] = "ANGLE:";
  if (strncmp(command, anglePrefix, sizeof(anglePrefix) - 1) == 0) {
    int angle = 0;
    if (parseAngle(command + sizeof(anglePrefix) - 1, angle)) {
      setServoAngle(angle);
    } else {
      Serial.println(F("ERROR:INVALID_ANGLE"));
    }
    return;
  }
  Serial.println(F("ERROR:EXPECTED_ANGLE_STATUS_OR_TEST"));
}

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN, MIN_PULSE_US, MAX_PULSE_US);
  delay(300);
  Serial.println(F("Arduino Uno servo test ready"));
  Serial.println(F("Commands: ANGLE:0-180, STATUS, TEST"));
  setServoAngle(0);
}

void loop() {
  while (Serial.available() > 0) {
    const char received = static_cast<char>(Serial.read());
    if (received == '\r') continue;
    if (received == '\n') {
      commandBuffer[commandLength] = '\0';
      handleCommand(commandBuffer);
      commandLength = 0;
    } else if (commandLength < sizeof(commandBuffer) - 1) {
      commandBuffer[commandLength++] = received;
    } else {
      commandLength = 0;
      Serial.println(F("ERROR:COMMAND_TOO_LONG"));
    }
  }
}
