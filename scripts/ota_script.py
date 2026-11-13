import os
import socket
import struct
import sys

PORT = 3333
CHUNK_SIZE = 384
TIMEOUT = 1.0
MAX_RETRIES = 10

def print_progress_bar(sent, total, width=40):
    percent = sent / total if total > 0 else 1.0
    filled = int(width * percent)
    bar = "█" * filled + "-" * (width - filled)
    sys.stdout.write(f"\rSending OTA : |{bar}| {percent * 100:5.1f}% ({sent}/{total} bytes)")
    sys.stdout.flush()

def main():
    if len(sys.argv) < 3:
        print("Usage : python3 ota_script.py <DEVICE_IPV6> <FIRMWARE_BIN_PATH>")
        sys.exit(1)

    device_ipv6 = sys.argv[1]
    bin_path = sys.argv[2]

    if not os.path.isfile(bin_path):
        print(f"Error : File not found -> {bin_path}")
        sys.exit(1)

    total_size = os.path.getsize(bin_path)
    sent_bytes = 0
    seq = 0

    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    sock.settimeout(TIMEOUT)

    try:
        with open(bin_path, "rb") as f:
            while chunk := f.read(CHUNK_SIZE):
                packet = struct.pack("!I", seq) + chunk
                acked = False

                for attempt in range(MAX_RETRIES):
                    sock.sendto(packet, (device_ipv6, PORT))
                    try:
                        data, _ = sock.recvfrom(16)
                        if len(data) == 4 and struct.unpack("!I", data)[0] == seq:
                            acked = True
                            break
                    except socket.timeout:
                        continue

                if not acked:
                    print(f"\nFailed : no answer after {MAX_RETRIES} retries (packet {seq}).")
                    sys.exit(1)

                sent_bytes += len(chunk)
                seq += 1
                print_progress_bar(sent_bytes, total_size)

        sock.sendto(b"END", (device_ipv6, PORT))
        print("\nUpdate validated and written successfully !")

    finally:
        sock.close()

if __name__ == "__main__":
    main()