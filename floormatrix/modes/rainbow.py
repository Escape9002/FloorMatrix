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

            batch_buffer = bytearray([ord('<'),ord('1')])

            for x in range(32):
                for y in range(8):
                    r, g, b = rainbow_matrix[y][x]

                    color = rgb_to_565(r, g, b)
                    high_byte = (color >> 8) & 0xFF
                    low_byte = color & 0xFF

                    index = xy_to_index(x,y)

                    pixel_cmd = bytearray([
                        
                         
                        clamp(index), 
                        clamp(high_byte), 
                        clamp(low_byte)
                    ])
                    batch_buffer.extend(pixel_cmd)


                    if len(batch_buffer) >= 17:
                        batch_buffer.extend([ord('>')])
                        await client.write_gatt_char(CHAR_UUID, batch_buffer, response=False)
                        batch_buffer = bytearray([ord('<'),ord('1')]) # Clear buffer
                        
                        # Tiny sleep prevents flooding the BLE stack
                        # With packing, we can sleep less often
                        await asyncio.sleep(0.001) 
                    

            
            await asyncio.sleep(0.016)  # ~60 FPS
    except Exception as e:
        print(f"❌ Disconnected or error during stream: {e}")

def xy_to_index(x, y, width=32):
    # row-major order: top-left is 0
    return y * width + x

def rgb_to_565(r, g, b):
    """Convert 8-bit R,G,B to 16-bit RGB565."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def clamp(value):
    """Prevents value from being '<' (60) or '>' (62)"""
    if value == 60: return 61
    if value == 62: return 63
    return value