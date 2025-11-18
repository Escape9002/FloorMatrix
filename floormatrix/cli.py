import asyncio
from bleak import BleakScanner, BleakClient
import argparse

from .modes.clock import send_time
from .modes.happy_messages import send_kindness
from .modes.rainbow import stream_rainbow
from .helper.clear import send_clear

# --- UUIDs and Target Name ---
SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
TARGET_NAME = "NeoMatrix"  # Change if needed

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
            await stream_rainbow(client, CHAR_UUID)
        elif args.mode == "time":
            # This mode uses the text-based command protocol for messages
            while True:
                await send_time(client, CHAR_UUID)
                await asyncio.sleep(20)
                await send_kindness(client, CHAR_UUID)    
                await asyncio.sleep(35)
        elif args.mode == "clear":
            await send_clear(client, CHAR_UUID)
            await asyncio.sleep(35)

        elif args.mode == "kind":
            await send_kindness(client, CHAR_UUID)

def main_entry():
    # Set up command-line argument parsing
    parser = argparse.ArgumentParser(description="Send data to a NeoMatrix display over BLE.")
    parser.add_argument(
        "mode", 
        choices=["rainbow", "time", "kind", "clear"], 
        help="The mode to run in: 'rainbow' for animation, 'time' to send the current time as a message."
    )
    args = parser.parse_args()

    while True:
        try:
            asyncio.run(main(args))
        except KeyboardInterrupt:
            print("\n👋 Exiting.\n Type stop to truly stop!")
        except Exception as e:
            print(f"An error occurred: {e.with_traceback}")
            print("To continue like nothing happened, press enter")

if __name__ == "__main__":
    main_entry()
    