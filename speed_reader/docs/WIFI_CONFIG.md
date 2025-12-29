# Speed Reader - WiFi Configuration

WiFi credentials are now **loaded at runtime** from SPIFFS (flash filesystem) instead of being compiled into the sketch.

## How It Works

1. WiFi settings are stored in `/config.json` on the ESP32's flash
2. On startup, the sketch reads `/config.json` using SPIFFS
3. If the file doesn't exist, it falls back to default credentials
4. To change WiFi settings, update the config file and re-upload (no sketch recompile needed)

## Changing WiFi Credentials

### Method 1: Edit config.json directly (Recommended)

1. Edit `data/config.json`:
   ```json
   {
     "ssid": "YourNetworkName",
     "password": "YourPassword"
   }
   ```

2. Upload SPIFFS data:
   ```batch
   upload-spiffs.bat
   ```

3. The sketch will automatically use the new credentials on next restart

### Method 2: Use the Config Uploader Sketch

1. Open `spiffs_config_uploader.ino` in Arduino IDE
2. Edit the SSID and password in the sketch
3. Compile and upload to your ESP32
4. Check Serial Monitor to confirm upload
5. Switch back to `speed_reader.ino` and upload normally

## File Structure

```
speed_reader/
├── speed_reader.ino              # Main sketch (reads config at runtime)
├── data/
│   └── config.json              # WiFi config (stored in SPIFFS)
├── spiffs_config_uploader.ino   # Utility to update config
├── build.bat                     # Build & upload script
├── upload-spiffs.bat             # Upload only SPIFFS data
└── wifi_config.h                 # (Legacy - no longer used)
```

## Default Credentials

If `/config.json` is missing, the sketch uses:
- SSID: `BAYSAN`
- Password: `timetowork`

## Uploading SPIFFS Data

### Using Arduino IDE:
1. Tools → ESP32 Sketch Data Upload
2. Requires the `ESP32 Filesystem Uploader` plugin

### Using Command Line:
```batch
upload-spiffs.bat
```

## Troubleshooting

**WiFi not connecting after config change?**
- Check `Serial Monitor` (115200 baud) to see if config loaded
- Look for message: `Loading WiFi config from SPIFFS... OK`
- Verify SSID and password in `data/config.json`

**config.json file not found?**
- Run `upload-spiffs.bat` to upload the data directory
- Or use the `spiffs_config_uploader.ino` utility

**SPIFFS mount failed?**
- Partition scheme may need more space for SPIFFS
- In Arduino IDE: Tools → Partition Scheme → choose one with "SPIFFS" or increase size

