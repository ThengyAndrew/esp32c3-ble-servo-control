// ESP32-C3 GPIO5 单脚连续 PWM 测试
// 无需串口、无需蓝牙。示波器地夹接 GND，探头接 GPIO5。

constexpr uint8_t TEST_PIN = 5;
constexpr uint32_t PWM_FREQUENCY_HZ = 50;
constexpr uint8_t PWM_RESOLUTION_BITS = 14;
constexpr uint32_t PWM_MAX_DUTY = (1UL << PWM_RESOLUTION_BITS) - 1;
// 1.5 ms / 20 ms = 7.5%，为舵机中位脉冲。
constexpr uint32_t PWM_MID_DUTY = (1500UL * PWM_MAX_DUTY) / 20000UL;

void setup() {
  // 标准舵机中位 PWM：50 Hz、1.5 ms 高脉冲。
  ledcAttach(TEST_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcWrite(TEST_PIN, PWM_MID_DUTY);
}

void loop() {
  // PWM 已由 LEDC 外设持续输出；主循环不改变 GPIO5 的电平或占空比。
}
