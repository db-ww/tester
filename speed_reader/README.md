# Speed Reader ESP32

A high-performance, secure, and professional-grade speed monitoring system built on the ESP32 platform. This project features high-precision hall-sensor rotation tracking, accelerometer-based angle monitoring, and a robust two-tier authenticated API.

## 🚀 Key Features

- **High Precision Tracking**: Accurate speed and distance measurements with configurable offsets and scales.
- **Secure by Design**:
  - **Two-Tier Authentication**: Distinct keys for daily usage (`X-API-Key`) and administrative management (`password`).
  - **Encrypted Storage**: Sensitive credentials (WiFi, API keys) are XOR-encrypted on the device's persistent storage (SPIFFS).
- **Modern API**: RESTful JSON API with both HTTP and HTTPS support.
- **Auto-Registration**: Automatic device registration to a central server on boot.
- **Rich Feedback**: Real-time monitoring via I2C LCD and Serial interface.

## 📂 Project Structure

- `speed_reader.ino`: Main Arduino sketch.
- `libraries/SpeedReaderCore`: Primary logic, HTTP handlers, and hardware abstraction.
- `data/`: SPIFFS data containing `config.json` and SSL certificates.
- `docs/`: Comprehensive technical documentation.

## 🛠 Setup & Documentation

Detailed guides are available in the [docs](./docs) folder:

- [API Reference](./docs/API_EXAMPLES.md): Learn how to interact with the device.
- [HTTPS & Certificates](./docs/HTTPS_CERTIFICATE_GUIDE.md): Secure your communication.
- [WiFi Configuration](./docs/WIFI_CONFIG.md): Network setup and troubleshooting.
- [Build Optimization](./docs/BUILD_OPTIMIZATION.md): Performance tuning for ESP32.

## 🚦 Quick Start

1. **Hardware**: Connect your Hall Effect sensor to `D4` and Accel to I2C.
2. **Configuration**: Edit `data/config.json` with your WiFi and API settings.
3. **Build**: Use the provided `build-and-upload.ps1` or `build.bat` scripts.
4. **Register**: The device will automatically ping your `register_url` once connected!

---
