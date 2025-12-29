
import os
import datetime
import ipaddress

# This script generates a simple self-signed certificate and private key in DER format
# for use with the ESP32 HTTPS server. 
# It requires the 'cryptography' library. If not installed, run: pip install cryptography

try:
    from cryptography import x509
    from cryptography.x509.oid import NameOID
    from cryptography.hazmat.primitives import hashes
    from cryptography.hazmat.primitives import serialization
    from cryptography.hazmat.primitives.asymmetric import ec
except ImportError:
    print("Error: 'cryptography' library not found.")
    print("Please install it by running: pip install cryptography")
    exit(1)

def generate_self_signed_cert():
    print("Generating Elliptic Curve Private Key (secp256r1)...")
    key = ec.generate_private_key(
        ec.SECP256R1()
    )

    print("Generating Self-Signed Certificate...")
    subject = issuer = x509.Name([
        x509.NameAttribute(NameOID.COUNTRY_NAME, u"US"),
        x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, u"Wisconsin"),
        x509.NameAttribute(NameOID.LOCALITY_NAME, u"Woodway"),
        x509.NameAttribute(NameOID.ORGANIZATION_NAME, u"SpeedReader"),
        x509.NameAttribute(NameOID.COMMON_NAME, u"speedreader.local"),
    ])
    
    cert = x509.CertificateBuilder().subject_name(
        subject
    ).issuer_name(
        issuer
    ).public_key(
        key.public_key()
    ).serial_number(
        x509.random_serial_number()
    ).not_valid_before(
        datetime.datetime.now(datetime.UTC)
    ).not_valid_after(
        # Our certificate will be valid for 10 years
        datetime.datetime.now(datetime.UTC) + datetime.timedelta(days=3650)
    ).add_extension(
        x509.SubjectAlternativeName([
            x509.DNSName(u"localhost"), 
            x509.DNSName(u"speedreader.local"),
            x509.IPAddress(ipaddress.IPv4Address(u"10.2.1.79"))
        ]),
        critical=False,
    ).sign(key, hashes.SHA256())

    # Ensure data directory exists
    os.makedirs("data", exist_ok=True)

    print("Saving to data/key.der...")
    with open("data/key.der", "wb") as f:
        f.write(key.private_bytes(
            encoding=serialization.Encoding.DER,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption(),
        ))

    print("Saving to data/cert.der...")
    with open("data/cert.der", "wb") as f:
        f.write(cert.public_bytes(serialization.Encoding.DER))

    print("Saving to data/cert.pem...")
    with open("data/cert.pem", "wb") as f:
        f.write(cert.public_bytes(serialization.Encoding.PEM))

    print("\nSuccess! Files generated in the 'data' folder.")
    print("Now run: build.bat -UploadData")

if __name__ == "__main__":
    generate_self_signed_cert()
