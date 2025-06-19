import asyncio
from bleak import BleakScanner, BleakClient
import random as rn
# --- UUIDs ---
SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
TARGET_NAME = "NeoMatrix"  # Change if needed

import colorsys

def generate_rainbow_matrix(width, height):
    """
    Generates a rainbow pattern across the x-axis of a matrix.

    Args:
        width (int): Number of columns in the matrix (x-axis).
        height (int): Number of rows in the matrix (y-axis).

    Returns:
        List[List[Tuple[int, int, int]]]: A 2D list of RGB tuples.
    """
    matrix = []

    for y in range(height):
        row = []
        for x in range(width):
            hue = x / width  # Hue varies from 0 to 1 along x-axis
            r, g, b = colorsys.hsv_to_rgb(hue, 1.0, 1.0)  # Full saturation and value
            rgb = (int(r * 255), int(g * 255), int(b * 255))
            row.append(rgb)
        matrix.append(row)

    return matrix


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

            rainbow_matrix = generate_rainbow_matrix(32,8)
            for y in range(0,8):
                for x in range(0,32):
                    r,g,b = rainbow_matrix[y][x]
                    
                    msg : str = "<p:" + \
                        str(x) + "," + \
                        str(y) + "," + \
                        str(r) + ","+ \
                        str(g) + ","+ \
                        str(b) + ">" \


            # for x in range(0, 32):
            #     for y in range(0,8):
            #         r,g,b = random_color()
            #         msg : str = "<p:" + \
            #             str(x) + "," + \
            #             str(y) + "," + \
            #             str(r) + ","+ \
            #             str(g) + ","+ \
            #             str(b) + ">" \


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
