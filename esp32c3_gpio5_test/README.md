# ESP32-C3 GPIO5 单独测试

烧录 `esp32c3_gpio5_test.ino` 前，请断开舵机信号线；示波器地夹接 ESP32 `GND`，探头接 Seeed XIAO ESP32-C3 的 **D3**。该板的 D3 才是芯片 `GPIO5`；板上 **D5 是 GPIO7**。

程序上电后持续输出标准舵机 PWM：每 20 ms 一次 3.3 V、1.5 ms 宽高脉冲（50 Hz），不会插入稳定高电平或低电平阶段。

示波器建议：DC 耦合、`1 V/div`、`5 ms/div`、上升沿触发、约 `1.5 V` 触发电平。

若持续看不到该 PWM，请先确认探头地与 ESP32 GND 共地、测点确为 GPIO5；其后再考虑 GPIO5 损坏或 LEDC 环境异常。
