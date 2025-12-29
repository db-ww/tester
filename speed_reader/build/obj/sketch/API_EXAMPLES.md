#line 1 "D:\\woodway\\apps\\Arduino\\Tester\\speed_reader\\API_EXAMPLES.md"
# Speed Reader API Examples

> **Authentication**: All requests must include the `X-API-Key` header (default: `hello`).
> **Authorization**: Management requests (`/config`) additionally require the `password` parameter in the request body (default: `admin`).
> **Encrypted Storage**: Sensitive credentials (`wifi_password`, `device_password`, `api_key`) are stored using XOR encryption on the device's persistent storage (SPIFFS). They are only decrypted into memory during the boot sequence.

## POST /config
Updates device configuration. Requires `X-API-Key` header AND `password` in body.
Accepts either `application/x-www-form-urlencoded` or `application/json` (raw body).

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| `password` | String | **Required**. Device password to authorize changes |
| `ssid` | String | New WiFi SSID |
| `wifi_password` | String | New WiFi Password |
| `name` | String | Device Name (e.g. "SpeedReader-01") |
| `speed_offset` | Float | Add/subtract mph (e.g. `0.5` or `-0.2`) |
| `speed_scale` | Float | Multiplier for speed (e.g. `1.05` = +5%) |
| `distance_offset` | Float | Add/subtract miles to distance readout |
| `angle_offset` | Float | Add/subtract degrees to angle readout |
| `accel_offset` | Float | Raw accelerometer offset |
| `accel_scale` | Float | Raw accelerometer scale |
| `api_key` | String | Change the API key |
| `device_password` | String | Change the device password |
| `register_url` | String | URL for automatic registration on startup |
| `station` | String | Station identifier for registration |

## Automatic Registration
On every boot, after a successful WiFi connection, the device sends an HTTP POST request to the configured `register_url`.

**Payload Format:**
```json
{
  "name": "SensorNode-01",
  "ip": "192.168.1.100",
  "station": "Station-01"
}
```

### Example CURL Requests

**1. Basic Configuration - JSON payload**
```bash
curl -X POST http://192.168.1.100/config \
     -H "X-API-Key: hello" \
     -H "Content-Type: application/json" \
     -d '{
           "password": "admin",
           "speed_scale": 1.02,
           "speed_offset": 0.0
         }'
```

**2. Zero Out Angle - Form payload**
```bash
curl -X POST http://192.168.1.100/config \
     -H "X-API-Key: hello" \
     -d "password=admin" \
     -d "angle_offset=-2.5"
```

**3. Change WiFi Settings - JSON payload**
```bash
curl -X POST http://192.168.1.100/config \
     -H "X-API-Key: hello" \
     -H "Content-Type: application/json" \
     -d '{
           "password": "admin",
           "ssid": "NewNetwork",
           "wifi_password": "NewPassword123"
         }'
```

**4. Change API Key**
```bash
curl -X POST http://192.168.1.100/config \
     -H "X-API-Key: hello" \
     -d "password=admin" \
     -d "api_key=newSecretKey"
```

## POST /start
Starts a new session. Resets all run measurements (rotations, distance, max speed, min/max angle).

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| `job` | String | **Required**. Job ID or Name (max 31 chars) |

### Example (JSON)
```bash
curl -X POST http://192.168.1.100/start \
     -H "X-API-Key: hello" \
     -H "Content-Type: application/json" \
     -d '{"job": "Run_001"}'
```

## GET /readings
Returns current sensor data. Requires authentication.

### Example
```bash
curl http://192.168.1.100/readings -H "X-API-Key: hello"
```

**Response:**
```json
{
  "rotations": 120,
  "distance_miles": 0.1500,
  "speed_mph": 4.50,
  "max_speed": 6.20,
  "angle": 1.5,
  "max_angle": 2.1,
  "min_angle": 0.0,
  "job": "Run_001"
}
```
