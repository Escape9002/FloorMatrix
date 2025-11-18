from bleak import BleakClient
import colorsys
import asyncio
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

async def stream_rainbow(client: BleakClient, CHAR_UUID : str):
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
