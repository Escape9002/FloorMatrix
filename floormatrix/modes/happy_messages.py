from bleak import BleakClient
import asyncio
async def send_kindness(client: BleakClient, CHAR_UUID: str):
    print("🕒 Clearing old messages...")

    # Clear old messages with DEL prep packets
    for idx in range(4):
        del_packet = f"<30>"
        await client.write_gatt_char(CHAR_UUID, del_packet.encode('utf-8'), response=True)
        await asyncio.sleep(0.03)  # slightly longer delay

    # set the font to big (4 is font-choice)
    # 1 is small
    # 2 is big
    await client.write_gatt_char(CHAR_UUID, "<42>".encode(), response=True)

    # Send new messages
    words = ["UwU", "=<O.o>=", "^-^ CHAOS  ^-^"]
    for sentence in words:
        message = f"<2{sentence}>"
        print(f"✉️  Sending message: {message}")
        await client.write_gatt_char(CHAR_UUID, message.encode('utf-8'), response=True)
        await asyncio.sleep(0.03)
    
    print("✅ Kindness messages sent successfully.")
