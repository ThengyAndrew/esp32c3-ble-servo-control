// Seeed XIAO ESP32-C3 GPIO 输出自检
// 不依赖 USB 串口或 BLE。每个候选 GPIO 依次输出 6 秒的 50 Hz、50% PWM 方波。
// 使用示波器测量目标 GPIO 对 GND，即可判断该引脚是否确实有输出。

#include <Arduino.h>
#include "esp32-hal-ledc.h"

// XIAO 板上可直接访问且适合此测试的芯片 GPIO。
constexpr uint8_t TEST_PINS[] = {3, 4, 5, 6, 7, 10};
constexpr size_t TEST_PIN_COUNT = sizeof(TEST_PINS) / sizeof(TEST_PINS[0]);
constexpr uint32_t TEST_FREQUENCY_HZ = 50;
constexpr uint8_t TEST_RESOLUTION_BITS = 14;
constexpr uint32_t TEST_DUTY = (1UL << (TEST_RESOLUTION_BITS - 1)); // 50% of 14-bit range
constexpr uint32_t PIN_TEST_DURATION_MS = 6000;
constexpr uint32_t CYCLE_PAUSE_MS = 3000;

void releasePin(uint8_t pin) {
  ledcDetach(pin);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void setup() {
  // 可选调试输出；即使 USB 串口没有显示，自动扫描仍会继续运行。
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32-C3 GPIO output test started");

  for (size_t index = 0; index < TEST_PIN_COUNT; ++index) {
    releasePin(TEST_PINS[index]);
  }
}

void loop() {
  for (size_t index = 0; index < TEST_PIN_COUNT; ++index) {
    const uint8_t pin = TEST_PINS[index];
    Serial.printf("TEST GPIO%d: 50 Hz, 50%% PWM\n", pin);

    // Arduino-ESP32 3.x：成功返回 true；失败时该 GPIO 不会有 PWM 输出。
    if (ledcAttach(pin, TEST_FREQUENCY_HZ, TEST_RESOLUTION_BITS)
        && ledcWrite(pin, TEST_DUTY)) {
      delay(PIN_TEST_DURATION_MS);
    } else {
      Serial.printf("ERROR: GPIO%d LEDC attach/write failed\n", pin);
      delay(PIN_TEST_DURATION_MS);
    }

    releasePin(pin);
  }

  Serial.println("GPIO test cycle complete; all test pins are LOW");
  delay(CYCLE_PAUSE_MS);
}
