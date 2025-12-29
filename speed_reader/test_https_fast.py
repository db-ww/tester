
import requests
import time
import json
import sys

def stream_readings(ip, interval_ms=10):
    url = f"https://{ip}/readings"
    cert_path = "data/cert.pem"
    interval_s = interval_ms / 1000.0
    
    print(f"Starting High-Speed Stream from {url}...")
    print(f"Polling interval: {interval_ms}ms")
    print("Pre-warming SSL connection (Keep-Alive)...")
    
    # Use a session to reuse the SSL connection (Keep-Alive)
    # This is CRITICAL for high-speed requests on ESP32
    session = requests.Session()
    session.verify = cert_path
    
    count = 0
    start_time = time.time()
    
    try:
        # First request to establish the session
        session.get(url, timeout=5)
        
        while True:
            loop_start = time.time()
            try:
                response = session.get(url, timeout=1.0)
                if response.status_code == 200:
                    data = response.json()
                    # Print on one line to keep the terminal clean
                    sys.stdout.write(f"\r[{count}] Speed: {data.get('speed_mph', 0.0):.2f} mph | Dist: {data.get('distance_miles', 0.0):.4f} mi | Angle: {data.get('angle', 0.0):.1f}°    ")
                    sys.stdout.flush()
                    count += 1
                else:
                    print(f"\nError: Status {response.status_code}")
            except requests.exceptions.RequestException as e:
                print(f"\nRequest failed: {e}")
                time.sleep(1) # Wait before retry
            
            # Precise sleep to maintain 10ms interval
            elapsed = time.time() - loop_start
            sleep_time = interval_s - elapsed
            if sleep_time > 0:
                time.sleep(sleep_time)
                
    except KeyboardInterrupt:
        total_time = time.time() - start_time
        print(f"\n\nStream stopped.")
        print(f"Total requests: {count}")
        print(f"Average frequency: {count/total_time:.2f} Hz")
    except Exception as e:
        print(f"\nUnexpected error: {e}")

if __name__ == "__main__":
    # The IP provided in your request
    DEVICE_IP = "10.2.1.79" 
    stream_readings(DEVICE_IP, interval_ms=1000)
