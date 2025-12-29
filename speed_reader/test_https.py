
import requests
import urllib3
import json

def get_readings(ip):
    url = f"https://{ip}/readings"
    cert_path = "data/cert.pem"
    print(f"Connecting to {url} using {cert_path}...")
    
    try:
        # Use our local certificate for verification
        response = requests.get(url, verify=cert_path, timeout=10)
        
        if response.status_code == 200:
            data = response.json()
            print("\n--- Speed Reader Readings ---")
            print(json.dumps(data, indent=4))
        else:
            print(f"Error: Server returned status code {response.status_code}")
            print(response.text)
            
    except requests.exceptions.ConnectionError:
        print("Error: Could not connect to the device. Is the IP correct and are you on the same network?")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    # The IP provided in your request
    DEVICE_IP = "10.2.1.79" 
    get_readings(DEVICE_IP)
