#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

constexpr char DEVICE_NAME[] = "ESP32C3-Servo-Control";
constexpr char SERVICE_UUID[] = "5ec5a100-1f4f-4d3c-9a1a-7cbf493a0001";
constexpr char RX_UUID[]      = "5ec5a101-1f4f-4d3c-9a1a-7cbf493a0001"; // Computer -> ESP32
constexpr char TX_UUID[]      = "5ec5a102-1f4f-4d3c-9a1a-7cbf493a0001"; // ESP32 -> computer (notify)
constexpr char STATUS_UUID[]  = "5ec5a103-1f4f-4d3c-9a1a-7cbf493a0001"; // Readable status

// 与 esp32c3_servo_swing 工程保持一致，舵机信号线接 GPIO5。
constexpr int SERVO_PIN = 5;
constexpr int MIN_ANGLE = 0;
constexpr int MAX_ANGLE = 180;
constexpr uint32_t SERVO_FREQUENCY = 50;
// ESP32-C3 的 LEDC 最大分辨率为 14 位；设为 16 位会导致 LEDC 配置失败。
constexpr uint8_t PWM_RESOLUTION_BITS = 14;
constexpr uint32_t PWM_PERIOD_US = 1000000UL / SERVO_FREQUENCY;
constexpr uint32_t MIN_PULSE_US = 500;
constexpr uint32_t MAX_PULSE_US = 2400;
constexpr uint32_t MAX_DUTY = (1UL << PWM_RESOLUTION_BITS) - 1;

BLECharacteristic *txCharacteristic;
BLECharacteristic *statusCharacteristic;
BLEAdvertising *advertising;
bool connected = false;
bool handshakeComplete = false;
bool servoPwmReady = false;
uint32_t challenge = 0;
int currentAngle = MIN_ANGLE;

void setStatus(const String &status) {
  statusCharacteristic->setValue(status.c_str());
  Serial.println("Status: " + status);
}

void sendMessage(const String &message) {
  Serial.println("BLE -> PC: " + message);
  txCharacteristic->setValue(message.c_str());
  if (connected) txCharacteristic->notify();
}

bool setServoAngle(int angle) {
  if (!servoPwmReady) {
    Serial.println("ERROR: PWM is not initialized");
    return false;
  }

  const int targetAngle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
  const uint32_t pulseWidthUs = map(targetAngle, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_US, MAX_PULSE_US);
  const uint32_t duty = (pulseWidthUs * MAX_DUTY) / PWM_PERIOD_US;
  if (!ledcWrite(SERVO_PIN, duty)) {
    Serial.println("ERROR: LEDC duty write failed");
    servoPwmReady = false;
    return false;
  }

  currentAngle = targetAngle;
  Serial.println("Servo angle: " + String(currentAngle));
  Serial.printf("PWM GPIO%d: %lu us, duty %lu/%lu\n", SERVO_PIN, pulseWidthUs, duty, MAX_DUTY);
  return true;
}

bool isUnsignedInteger(const String &value) {
  if (value.length() == 0) return false;
  for (size_t i = 0; i < value.length(); ++i) {
    if (!isDigit(value[i])) return false;
  }
  return true;
}

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    connected = true;
    handshakeComplete = false;
    setStatus("CONNECTED");
    Serial.println("Computer connected; waiting for HELLO");
  }

  void onDisconnect(BLEServer *server) override {
    connected = false;
    handshakeComplete = false;
    setStatus("ADVERTISING");
    delay(100);
    advertising->start();
    Serial.println("Computer disconnected; restarting advertising");
  }
};

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) override {
    String command = characteristic->getValue();
    command.trim();
    Serial.println("PC -> BLE: " + command);

    if (command == "HELLO") {
      handshakeComplete = false;
      challenge = esp_random();
      setStatus("CHALLENGE_SENT");
      sendMessage("CHALLENGE:" + String(challenge));
      return;
    }

    const String confirmPrefix = "CONFIRM:";
    if (command.startsWith(confirmPrefix)) {
      const String received = command.substring(confirmPrefix.length());
      if (challenge != 0 && received == String(challenge)) {
        handshakeComplete = true;
        setStatus("READY");
        sendMessage("READY");
        sendMessage("ANGLE:" + String(currentAngle));
      } else {
        handshakeComplete = false;
        setStatus("FAILED");
        sendMessage("ERROR:BAD_CHALLENGE");
      }
      return;
    }

    const String anglePrefix = "ANGLE:";
    if (handshakeComplete && command.startsWith(anglePrefix)) {
      const String value = command.substring(anglePrefix.length());
      if (!isUnsignedInteger(value)) {
        sendMessage("ERROR:INVALID_ANGLE");
        return;
      }
      const long requestedAngle = value.toInt();
      if (requestedAngle < MIN_ANGLE || requestedAngle > MAX_ANGLE) {
        sendMessage("ERROR:ANGLE_OUT_OF_RANGE");
        return;
      }
      if (!setServoAngle(static_cast<int>(requestedAngle))) {
        setStatus("PWM_NOT_READY");
        sendMessage("ERROR:PWM_NOT_READY");
        return;
      }
      sendMessage("ANGLE:" + String(currentAngle));
      return;
    }

    if (handshakeComplete && command == "PING") {
      sendMessage("PONG");
      return;
    }

    sendMessage("ERROR:EXPECTED_HELLO_CONFIRM_OR_ANGLE");
  }
};

void setup() {
  Serial.begin(115200);
  servoPwmReady = ledcAttach(SERVO_PIN, SERVO_FREQUENCY, PWM_RESOLUTION_BITS);
  if (!servoPwmReady) {
    Serial.printf("ERROR: LEDC attach failed on GPIO%d\n", SERVO_PIN);
  } else if (!setServoAngle(MIN_ANGLE)) {
    Serial.println("ERROR: Initial servo PWM write failed");
  }

  BLEDevice::init(DEVICE_NAME);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);
  txCharacteristic = service->createCharacteristic(
    TX_UUID, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
  txCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *rxCharacteristic = service->createCharacteristic(
    RX_UUID, BLECharacteristic::PROPERTY_WRITE);
  rxCharacteristic->setCallbacks(new RxCallbacks());

  statusCharacteristic = service->createCharacteristic(
    STATUS_UUID, BLECharacteristic::PROPERTY_READ);
  setStatus("ADVERTISING");

  service->start();
  advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();
  Serial.println("Advertising as: " + String(DEVICE_NAME));
}

void loop() {
  delay(1000);
}
