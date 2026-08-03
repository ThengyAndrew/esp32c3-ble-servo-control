# ESP32-C3 BLE 舵机控制（第三版）

## 无 Python 的电脑前端使用

网页前端可发布到 GitHub Pages，通过 HTTPS 在任意安装 Chrome 或 Edge 的电脑上使用，无需安装 Python。完整发布步骤见 [DEPLOYMENT.md](DEPLOYMENT.md)。

## 功能

ESP32-C3 通过 BLE 与电脑完成挑战握手后，接收 `ANGLE:0–180` 指令控制普通位置舵机。电脑前端使用 −10° 和 +10° 两个按键调整目标角度；设备上电时舵机设为 0°。

## 接线

| 舵机线 | 连接位置 |
| --- | --- |
| 信号线（橙色/黄色） | Seeed XIAO ESP32-C3 的 **D3**（芯片 GPIO5；不是 D5） |
| 电源正极（红色） | 独立 5 V 电源正极 |
| 地线（棕色/黑色） | 独立 5 V 电源负极，并与 ESP32-C3 GND 共地 |

不要使用 ESP32-C3 的 3.3 V 引脚给舵机供电。

## 上传

在 Arduino IDE 安装 Espressif ESP32 开发板支持（此工程使用 Arduino-ESP32 3.x 的 LEDC API）；选择实际 ESP32-C3 板型与端口后，上传 `esp32c3_ble_servo_control.ino`。本版本使用 ESP32 内置 LEDC PWM，不需要安装 `ESP32Servo` 库。

舵机 PWM 为 50 Hz、14 位分辨率，脉宽范围为 500–2400 µs。ESP32-C3 的 LEDC 分辨率上限为 14 位，不能改为 16 位；否则 PWM 初始化失败，舵机没有控制信号。若舵机在端点抖动或顶死，请收窄角度或脉宽范围。

## 电脑前端

前端位于 `web_frontend/index.html`。在该目录启动本地服务：

```powershell
python -m http.server 8000
```

用 Chrome 或 Edge 打开 <http://localhost:8000>，选择 `ESP32C3-Servo-Control`。握手完成后，使用两个角度按钮控制舵机。

## 协议

握手：`HELLO → CHALLENGE:<值> → CONFIRM:<值> → READY`。

握手成功后，电脑向 RX 特征写入 `ANGLE:<0–180>`；设备通过 TX 通知回传 `ANGLE:<当前角度>`。服务和特征 UUID 与第二版相同。
