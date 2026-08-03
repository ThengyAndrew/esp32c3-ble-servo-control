// ESP32-C3 GPIO5 单脚电平与 PWM 测试
// 无需串口、无需蓝牙。示波器地夹接 GND，探头接 GPIO5。

constexpr uint8_t TEST_PIN = 5;
constexpr uint32_t PWM_FREQUENCY_HZ = 50;
constexpr uint8_t PWM_RESOLUTION_BITS = 14;
constexpr uint32_t PWM_MAX_DUTY = (1UL << PWM_RESOLUTION_BITS) - 1;
// 1.5 ms / 20 ms = 7.5%，为舵机中位脉冲。
constexpr uint32_t PWM_MID_DUTY = (1500UL * PWM_MAX_DUTY) / 20000UL;

void forceLevel(uint8_t level) {
  ledcDetach(TEST_PIN);
  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, level);
}

void setup() {
  forceLevel(LOW);
}

void loop() {
  // 阶段 1：6 秒标准舵机 PWM，50 Hz、1.5 ms 高脉冲。
  if (ledcAttach(TEST_PIN, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS)) {
    ledcWrite(TEST_PIN, PWM_MID_DUTY);
  }
  delay(6000);

  // 阶段 2：2 秒强制低电平。若此阶段仍为高电平，说明测点、接线或 GPIO5 异常。
  forceLevel(LOW);
  delay(2000);

  // 阶段 3：2 秒强制高电平，用于确认示波器和测点正确。
  forceLevel(HIGH);
  delay(2000);
}
