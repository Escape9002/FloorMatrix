from bleak import BleakClient


async def send_kindness(client: BleakClient, CHAR_UUID : str):
    """
    Sends some kind words
    """
    print("🕒 Getting some sentences...")
 
    
    
    for i in range(5):
        await client.write_gatt_char(CHAR_UUID, "<DEL:0>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:1>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<DEL:2>".encode(), response=True)
        await client.write_gatt_char(CHAR_UUID, "<FONT_BIG>".encode(), response=True)
    
    # Construct the message according to the protocol in your C++ code.
    # The command is "ADD:<text>", and the packet is wrapped in "<>".
    words = ["UwU", "=<O.o>=", "^-^ CHAOS  ^-^"]

    for sentence in words:
        message = f"<ADD:{sentence}>"
 
        print(f"✉️  Sending message: {message}")
        # Use "Write with Response" for commands to ensure they are processed.
        await client.write_gatt_char(CHAR_UUID, message.encode(), response=True)
    
    print("✅ Time message sent successfully.")
