from bleak import BleakClient


async def send_clear(client: BleakClient, CHAR_UUID: str):
    print("sending clear")
    message = "<0>"
    await client.write_gatt_char(CHAR_UUID, message.encode(), response=True)
    
