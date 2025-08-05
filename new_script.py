import asyncio
from bleak import BleakScanner, BleakClient
import colorsys
import argparse
from datetime import datetime

# --- UUIDs and Target Name ---
SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
TARGET_NAME = "NeoMatrix"  # Change if needed

def generate_rainbow_matrix(width, height):
    """Generates a rainbow pattern across the x-axis of a matrix."""
    matrix = []
    for y in range(height):
        row = []
        for x in range(width):
            hue = x / width
            r, g, b = colorsys.hsv_to_rgb(hue, 1.0, 1.0)
            rgb = (int(r * 255), int(g * 255), int(b * 255))
            row.append(rgb)
        matrix.append(row)
    return matrix

async def stream_rainbow(client: BleakClient):
    """
    Continuously streams a rainbow pattern to the matrix using the efficient
    binary chunking method.
    """
    print("🚀 Starting rainbow stream...")
    # The -3 is for BLE overhead.
    max_payload_size = client.mtu_size - 3
    print(f"✅ Connection successful. MTU size: {client.mtu_size}, payload size: {max_payload_size}")

    try:
        while True:
            rainbow_matrix = generate_rainbow_matrix(32, 8)
            for y in range(8):
                for x in range(32):
                    r, g, b = rainbow_matrix[y][x]
                    
                    msg: str = f"<p:{x},{y},{0},{r},{g},{b}>"
                    
                    await client.write_gatt_char(CHAR_UUID, msg.encode())
            
            await asyncio.sleep(0.016)  # ~60 FPS
    except Exception as e:
        print(f"❌ Disconnected or error during stream: {e}")

async def send_time(client: BleakClient):
    """
    Formats the current local time and sends it as a single message command.
    """
    print("🕒 Getting current time...")
    now = datetime.now()
    time_str = now.strftime("%H:%M")  # Format as HH:MM

    # Construct the message according to the protocol in your C++ code.
    # The command is "ADD:<text>", and the packet is wrapped in "<>".
    message = f"<ADD:{time_str}>"
    
    print(f"✉️  Sending message: {message}")
    

    await client.write_gatt_char(CHAR_UUID, "<CLEAR>".encode(), response=True)
    for i in range(5):
        await client.write_gatt_char(CHAR_UUID, "<DEL:0>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:1>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:2>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<FONT_BIG>".encode(), response=True)
    

    # Use "Write with Response" for commands to ensure they are processed.
    await client.write_gatt_char(CHAR_UUID, message.encode(), response=True)
    
    print("✅ Time message sent successfully.")

async def send_kindness(client: BleakClient):
    """
    Sends some kind words
    """
    print("🕒 Getting some sentences...")
 
    
    await client.write_gatt_char(CHAR_UUID, "<CLEAR>".encode(), response=True)
    for i in range(5):
        await client.write_gatt_char(CHAR_UUID, "<DEL:0>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:1>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:2>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<FONT_BIG>".encode(), response=True)
    
    # Construct the message according to the protocol in your C++ code.
    # The command is "ADD:<text>", and the packet is wrapped in "<>".
    words = ["UwU", "Time flies like my laptop", "^-^ CHAOS ^-^"]

    for sentence in words:
        message = f"<ADD:{sentence}>"
 
        print(f"✉️  Sending message: {message}")
        # Use "Write with Response" for commands to ensure they are processed.
        await client.write_gatt_char(CHAR_UUID, message.encode(), response=True)
    
    print("✅ Time message sent successfully.")

async def main(args):
    """Scans for the device and runs the selected mode."""
    print("🔍 Scanning for devices...")
    device = await BleakScanner.find_device_by_name(TARGET_NAME, timeout=10)

    if not device:
        print(f"❌ Could not find device named '{TARGET_NAME}'.")
        return

    print(f"✅ Found device: {device.name} [{device.address}]")

    async with BleakClient(device.address) as client:
        print(f"🔗 Connected to {device.address}")

        if args.mode == "rainbow":
            # This mode uses the efficient binary protocol for animations
            await stream_rainbow(client)
        elif args.mode == "time":
            # This mode uses the text-based command protocol for messages
            while True:
                await send_time(client)
                await asyncio.sleep(20)
                await send_kindness(client)    
                await asyncio.sleep(35)

        elif args.mode == "kind":
            await send_kindness(client)

if __name__ == "__main__":
    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(description="Send data to a NeoMatrix display over BLE.")
    parser.add_argument(
        "mode", 
        choices=["rainbow", "time", "kind"], 
        help="The mode to run in: 'rainbow' for animation, 'time' to send the current time as a message."
    )
    args = parser.parse_args()

    try:
        asyncio.run(main(args))
    except KeyboardInterrupt:
        print("\n👋 Exiting.")
    except Exception as e:
        print(f"An error occurred: {e}")