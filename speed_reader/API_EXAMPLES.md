# Speed Reader API Examples

## POST /config
Updates device configuration. Requires admin password.
Accepts either `application/x-www-form-urlencoded` or `application/json` (raw body).

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| `password` | String | **Required**. Admin password (default: `hello`) |
| `ssid` | String | New WiFi SSID |
| `wifi_password` | String | New WiFi Password |
| `name` | String | Device Name (e.g. "SpeedReader-01") |
| `speed_offset` | Float | Add/subtract mph (e.g. `0.5` or `-0.2`) |
| `speed_scale` | Float | Multiplier for speed (e.g. `1.05` = +5%) |
| `distance_offset` | Float | Add/subtract miles to distance readout |
| `angle_offset` | Float | Add/subtract degrees to angle readout |
| `accel_offset` | Float | Raw accelerometer offset |
| `accel_scale` | Float | Raw accelerometer scale |
| `new_password` | String | Change the admin password |

### Example CURL Requests

**1. Basic Configuration (Speed Calibration) - JSON payload**
```bash
curl -X POST http://192.168.1.100/config \
     -H "Content-Type: application/json" \
     -d '{
           "password": "hello",
           "speed_scale": 1.02,
           "speed_offset": 0.0
         }'
```

**2. Zero Out Angle - Form payload**
If the device is reading 2.5 degrees when flat, set offset to -2.5.
```bash
curl -X POST http://192.168.1.100/config \
     -d "password=hello" \
     -d "angle_offset=-2.5"
```

**3. Change WiFi Settings - JSON payload**
```bash
curl -X POST http://192.168.1.100/config \
     -H "Content-Type: application/json" \
     -d '{
           "password": "hello",
           "ssid": "NewNetwork",
           "wifi_password": "NewPassword123"
         }'
```

**4. Change Admin Password**
```bash
curl -X POST http://192.168.1.100/config \
     -d "password=hello" \
     -d "new_password=secret123"
```

## POST /start
Starts a new session. Resets all run measurements (rotations, distance, max speed, min/max angle).
Accepts either `application/x-www-form-urlencoded` or `application/json`.

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| `job` | String | **Required**. Job ID or Name (max 31 chars) |

### Example (JSON)
```bash
curl -X POST http://192.168.1.100/start \
     -H "Content-Type: application/json" \
     -d '{"job": "Run_001"}'
```

### Example (Form)
```bash
curl -X POST http://192.168.1.100/start \
     -d "job=Run_001"
```

## GET /readings
Returns current sensor data.

### Example
```bash
curl http://192.168.1.100/readings
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
