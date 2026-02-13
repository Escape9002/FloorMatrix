from bleak import BleakClient
from datetime import datetime
import asyncio

async def send_time(client: BleakClient, CHAR_UUID: str):
    print("🕒 Getting current time...")
    now = datetime.now()
    time_str = now.strftime("%H:%M")  # HH:MM

    # Clear old message slots first
    for idx in range(4):
        del_packet = f"<30>"
        await client.write_gatt_char(CHAR_UUID, del_packet.encode(), response=True)
        await asyncio.sleep(0.01)

    await client.write_gatt_char(CHAR_UUID, "<42>".encode(), response=True)
    # Send new time message
    message = f"<2{time_str}>"
    print(f"✉️ Sending message: {message}")
    await client.write_gatt_char(CHAR_UUID, message.encode(), response=True)
    await asyncio.sleep(0.01)

    
    print("✅ Time message sent successfully.")
