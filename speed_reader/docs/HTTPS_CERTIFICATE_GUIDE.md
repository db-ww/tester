# HTTPS Certificate Management Guide

## Overview

The Speed Reader device supports both **HTTP** and **HTTPS** modes, configurable via `config.json`. Certificates are automatically generated and stored in SPIFFS for reuse.

## Configuration

### Enabling/Disabling HTTPS

Edit `data/config.json`:

```json
{
  "ssid": "YourWiFi",
  "password": "YourPassword",
  "name": "SensorNode-01",
  "use_https": true,    // Set to false for HTTP mode
  ...
}
```

- **`true`**: HTTPS mode on port 443 (secure, default)
- **`false`**: HTTP mode on port 80 (non-secure, faster)

## Certificate Storage

Certificates are stored in SPIFFS as:
- `/cert.der` - Public certificate (DER format)
- `/key.der` - Private key (DER format)

### First Boot Behavior

**HTTPS Mode Enabled:**
1. Check if `/cert.der` and `/key.der` exist
2. If **NOT found**: Generate new self-signed certificate (~30 seconds)
3. If **found**: Load certificates from SPIFFS (instant)

**HTTP Mode:**
- No certificates needed or generated
- Server starts immediately

## How Certificates Are Generated

The device uses the esp32_idf5_https_server library's built-in certificate generation:

```cpp
createSelfSignedCert(
  *cert, 
  KEYSIZE_2048,                           // 2048-bit RSA
  "CN=speedreader.local,O=Woodway",       // Certificate subject
  "20240101000000",                        // Valid from (YYYYMMDDHHMMSS)
  "20340101000000"                         // Valid until (10 years)
)
```

## Pre-Generating Certificates

### Option 1: Let Device Generate (Recommended)

**Advantages:**
- Fully automatic
- No external tools needed
- Certificates stored in SPIFFS automatically

**Disadvantages:**
- First boot takes ~30 seconds
- Must be done on the device

**How to:**
1. Set `"use_https": true` in config.json
2. Upload SPIFFS data (run `upload-spiffs.bat`)
3. Boot device - it will generate and save certificates
4. Future boots load certificates instantly

### Option 2: Using OpenSSL (Advanced)

If you want to pre-generate certificates on your PC and upload them:

#### Generate Certificate

```bash
# Generate private key
openssl genrsa -out key.pem 2048

# Generate self-signed certificate (10 year validity)
openssl req -new -x509 -key key.pem -out cert.pem -days 3650 \
  -subj "/CN=speedreader.local/O=Woodway"

# Convert to DER format (binary)
openssl x509 -in cert.pem -outform DER -out cert.der
openssl rsa -in key.pem -outform DER -out key.der
```

#### Upload to SPIFFS

1. Place `cert.der` and `key.der` in the `data/` folder
2. Run upload script: `upload-spiffs.bat`
3. Device will use these certificates instead of generating new ones

## Switching Between HTTP and HTTPS

### To Switch to HTTP Mode:

1. Edit `data/config.json`:
   ```json
   "use_https": false
   ```
2. Upload SPIFFS: `upload-spiffs.bat`
3. Reboot device
4. Access at: `http://[device-ip]` (port 80)

### To Switch to HTTPS Mode:

1. Edit `data/config.json`:
   ```json
   "use_https": true
   ```
2. Upload SPIFFS: `upload-spiffs.bat`
3. Reboot device
4. Access at: `https://[device-ip]` (port 443)

## Browser Certificate Warnings

Self-signed certificates will trigger browser security warnings:

**"Your connection is not private" / "NET::ERR_CERT_AUTHORITY_INVALID"**

This is **NORMAL** for self-signed certificates. To proceed:

- **Chrome/Edge**: Click **Advanced** → **Proceed to [IP] (unsafe)**
- **Firefox**: Click **Advanced** → **Accept the Risk and Continue**
- **Safari**: Click **Show Details** → **visit this website**

### Why This Happens

Self-signed certificates are not verified by a trusted Certificate Authority (CA). They still provide encryption but cannot verify server identity.

### Production Options

For trusted certificates (no browser warnings):

1. **Option A**: Use a domain name + Let's Encrypt (requires public DNS)
2. **Option B**: Create your own CA and install it on client devices
3. **Option C**: Use HTTP mode for local/trusted networks

## Troubleshooting

### Certificate Generation Fails

**Symptoms:** Device boots but HTTPS server doesn't start
**Solutions:**
1. Check Serial Monitor for error code: `Cert Gen Failed: 0xXX`
2. Ensure SPIFFS has enough space (check with `SPIFFS.totalBytes()`)
3. Try factory reset: Delete `/cert.der` and `/key.der`, reboot

### Certificates Not Loading

**Symptoms:** Device generates new cert every boot
**Solutions:**
1. Verify SPIFFS is mounted: Check Serial Monitor for "Loading Config from SPIFFS..."
2. Check files exist: Device should log "Loading cert from SPIFFS..."
3. Verify file sizes: Both files should be > 0 bytes

### Server Won't Start

**Symptoms:** `serverStarted = false`
**Solutions:**
1. Check if port is blocked (443 for HTTPS, 80 for HTTP)
2. Verify `ENABLE_HTTP` is set to `1` in `config.h`
3. Check RAM: Large certs may cause OOM on constrained devices

## API Access Examples

### HTTP Mode (port 80)
```bash
curl http://192.168.1.100/readings
curl -X POST http://192.168.1.100/start?job=test123
```

### HTTPS Mode (port 443)
```bash
curl -k https://192.168.1.100/readings
curl -k -X POST https://192.168.1.100/start?job=test123
```

**Note:** `-k` flag bypasses certificate verification (needed for self-signed certs)

## Security Considerations

### HTTP Mode
- ✅ Fast, simple
- ❌ **NO ENCRYPTION**: Data sent in plain text
- ⚠️ Only use on trusted/isolated networks

### HTTPS Mode  
- ✅ **ENCRYPTED**: Protects data in transit
- ✅ Prevents passive eavesdropping
- ⚠️ Self-signed = No identity verification
- ⚠️ First boot takes ~30s to generate cert

## File Locations

```
speed_reader/
├── data/
│   ├── config.json          # Contains "use_https" setting
│   ├── cert.der            # Optional: Pre-generated certificate
│   └── key.der             # Optional: Pre-generated private key
└── SPIFFS (on device):
    ├── /config.json        # Uploaded from data/
    ├── /cert.der          # Auto-generated or uploaded
    └── /key.der           # Auto-generated or uploaded
```

## Quick Reference

| Task | Command/Action |
|------|----------------|
| Enable HTTPS | Set `"use_https": true` in config.json |
| Enable HTTP | Set `"use_https": false` in config.json |
| Upload config | Run `upload-spiffs.bat` |
| Force cert regeneration | Delete `/cert.der` and `/key.der` from SPIFFS, reboot |
| View cert info | Check Serial Monitor during boot |
| Test connectivity | HTTP: `curl http://IP/readings` <br> HTTPS: `curl -k https://IP/readings` |
