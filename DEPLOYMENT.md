# HTTPS 前端发布（GitHub Pages）

本项目的网页前端位于 `web_frontend/`。它不需要 Python、Node.js 或任何后端；GitHub Pages 会以 HTTPS 提供该目录中的静态文件，浏览器再通过 Web Bluetooth 直接连接附近的 ESP32-C3。

## 首次发布

1. 在 GitHub 创建一个新的**公开**仓库，例如 `esp32c3-ble-servo-control`。
2. 在本项目目录打开 PowerShell，依次执行：

   ```powershell
   git init -b main
   git add .
   git commit -m "Publish BLE servo controller"
   git remote add origin https://github.com/<你的用户名>/<仓库名>.git
   git push -u origin main
   ```

3. 在 GitHub 仓库中打开 **Settings → Pages**，将 **Build and deployment / Source** 设为 **GitHub Actions**。
4. 打开仓库的 **Actions** 页面，等待 `Deploy website to GitHub Pages` 工作流完成。页面地址会显示在该工作流的 deploy 步骤中，通常为：

   ```text
   https://<你的用户名>.github.io/<仓库名>/
   ```

## 使用要求

- 使用最新版 Chrome 或 Microsoft Edge；不支持 Safari、Firefox。
- Windows 必须有可用的蓝牙适配器与驱动，ESP32-C3 必须已上电且在蓝牙范围内。
- 访问上面的 **HTTPS** 地址，点击“连接设备”，然后在浏览器弹窗中选择 `ESP32C3-Servo-Control`。
- 浏览器的设备选择与授权是安全机制，不能自动跳过。

## 更新网页

修改 `web_frontend/` 内的文件后提交并推送到 `main` 分支：

```powershell
git add web_frontend
git commit -m "Update web interface"
git push
```

GitHub Actions 会自动重新部署。通常等待一两分钟后，刷新网页即可获得新版本。

## 安全边界

GitHub Pages 只托管网页代码，不会转发蓝牙数据。舵机指令仅在用户电脑与附近 ESP32-C3 之间通过 BLE 传输。公开页面不意味着互联网上的用户能远程控制你的舵机；他们仍必须处在蓝牙范围内并主动在浏览器中授权设备。
