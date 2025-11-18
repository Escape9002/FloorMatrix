from bleak import BleakClient
from datetime import datetime


async def send_time(client: BleakClient, CHAR_UUID : str):
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
    

    
    for i in range(5):
        await client.write_gatt_char(CHAR_UUID, "<DEL:0>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:1>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:2>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<FONT_BIG>".encode(), response=True)
    

    # Use "Write with Response" for commands to ensure they are processed.
    await client.write_gatt_char(CHAR_UUID, message.encode(), response=True)
    
    print("✅ Time message sent successfully.")
