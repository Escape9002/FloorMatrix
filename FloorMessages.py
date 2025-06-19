import asyncio
from bleak import BleakScanner, BleakClient
import random as rn
# --- UUIDs ---
SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
TARGET_NAME = "NeoMatrix"  # Change if needed
import struct


packet_format = "<Iffff"  # timestamp(uint32), q_i(float), q_j(float), q_k(float), q_r(float)

packet_size = struct.calcsize(packet_format)  # Should be 18 bytes
import time

last_packet_time = None

def handle_packet(sender: int, data: bytearray):
    global last_packet_time

    # Print raw data length and hex for debugging
    # print(f"Received {len(data)} bytes: {data.hex()}")

    # Ensure data length matches expected size
    if len(data) != struct.calcsize(packet_format):
        print(f"⚠️ Unexpected packet size: {len(data)} bytes")
        return

    # Unpack the data
    try:
        timestamp, q_i, q_j, q_k, q_r = struct.unpack(packet_format, data)
        print(f"Timestamp: {timestamp}, q_i: {q_i}, q_j: {q_j}, q_k: {q_k}, q_r: {q_r}")

        # Measure reception rate in Hz
        now = time.time()
        if last_packet_time is not None:
            dt = now - last_packet_time
            if dt > 0:
                rate_hz = 1.0 / dt
                # print(f"Reception rate: {rate_hz:.2f} packets/sec")
        last_packet_time = now

    except Exception as e:
        print(f"⚠️ Unpacking error: {e}")


# --- Scan and connect ---
async def main():
    print("🔍 Scanning for devices...")
    devices = await BleakScanner.discover(timeout=10)
    target = None

    for d in devices:
        print(f"Found: {d.name} [{d.address}]")
        if d.name and TARGET_NAME.lower() in d.name.lower():
            target = d
            break

    if not target:
        print("❌ Could not find HM-10 device.")
        return

    print(f"✅ Found HM-10 device: {target.name} [{target.address}]")

    async with BleakClient(target.address) as client:
        print(f"🔗 Connected to {target.address}")

        services = await client.get_services()
        print("📡 Available services:")
        for service in services:
            print(f"- {service.uuid}")
            for char in service.characteristics:
                print(f"  - Characteristic: {char.uuid}")

        print("sending color bits!")
        while(True):
            for x in range(0, 32):
                for y in range(0,8):
                    r,g,b = random_color()
                    msg : str = "<p:" + \
                        str(x) + "," + \
                        str(y) + "," + \
                        str(r) + ","+ \
                        str(g) + ","+ \
                        str(b) + ">" \


                    await client.write_gatt_char(CHAR_UUID, msg.encode())
 
def random_color():
    levels = range(32,256,32)
    return tuple(rn.choice(levels) for _ in range(3))
        

# --- Entry Point ---
if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n👋 Exiting.")
