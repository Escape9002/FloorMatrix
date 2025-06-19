#if defined(USE_NRF52_DRIVER)

#include "BLEnRF52.h"

// Debug macros are best placed here or in the header, outside the class
#define DEBUG 1
#if DEBUG
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif

/**
 * The Bluefruit library expects functions with this header:
 *      typedef void (*rx_callback_t) (uint16_t conn_hdl);
 * Since functions originating have a different header
 *      (BLEnRF::*) (uint16_t conn_hdl)
 * its required to mark the functions passed to bluefruit as
 * static. Since this may break function when there is more 
 * then one of these driver objects, we have to ensure that 
 * only one will ever exist.
 * 
 * This is ensured via the getInstance functions.
 * Since the static functions have no access to local variables
 * of the object, a trampolin pattern is implemented to send
 * the call "into" the object with local variables. 
 */

// --- Static Member Initialization ---
BLEnRF52* BLEnRF52::_instance = nullptr;

// Corrected UUIDs to be more descriptive of what they are
const char* BLEnRF52::VEIIO_SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* BLEnRF52::VEIIO_DATAPACKET_CHAR_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";


// --- Singleton Accessor ---
BLEnRF52& BLEnRF52::getInstance() {
    if (_instance == nullptr) {
        _instance = new BLEnRF52();
    }
    return *_instance;
}

// --- Constructor (CORRECT way to initialize members) ---
BLEnRF52::BLEnRF52() :
    bledis(),
    bleuart(),
    service(VEIIO_SERVICE_UUID),
    DataPacketChar(VEIIO_DATAPACKET_CHAR_UUID)
{
    // The constructor is now responsible for initializing the objects.
    // The `begin()` method is for starting them.
}

// --- Public Method Implementations ---

bool BLEnRF52::begin() {
    _instance = this; // Set the instance pointer for static callbacks

    // Setup basic BLE settings
    Bluefruit.autoConnLed(true);
    Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
    Bluefruit.begin();
    Bluefruit.setTxPower(4);
    Bluefruit.Periph.setConnInterval(6, 12); // 7.5 - 15 ms
    Bluefruit.setName("Feather_nRF52840_MultiChar");

    // Set the callbacks to our static "trampoline" functions
    Bluefruit.Periph.setConnectCallback(static_connect_callback);
    Bluefruit.Periph.setDisconnectCallback(static_disconnect_callback);

    // Setup services and characteristics in the correct order
    configDeviceInfo();
    setupBLEUart();
    setup_DataPacketChar(); // This will now also begin the service

    // Finally, start advertising
    startAdv();

    DEBUG_PRINTLN("BLEnRF52 Driver Started");
    return true;
}

bool BLEnRF52::connected() {
    return Bluefruit.connected();
}

// CORRECTED: Must take data and length
bool BLEnRF52::sendDataPacket(const void* data, uint16_t len) {
    if (connected()) {
        return DataPacketChar.notify(data, len);
    }
    return false;
}

bool BLEnRF52::sendWhenReady(const void* data, uint16_t len) {
    // For now, it's just a pass-through. A real implementation
    // would buffer the data if not connected.
    return sendDataPacket(data, len);
}

// CORRECTED: Checks our internal buffer, not the hardware buffer
bool BLEnRF52::available() {
    return received_msg.length() > 0;
}

// CORRECTED: Returns and clears our internal buffer
String BLEnRF52::get_received() {
    if (available()) {
        String msg = received_msg;
        received_msg = ""; // Clear buffer after reading
        return msg;
    }
    return "";
}

// --- Private Helper Implementations ---

void BLEnRF52::configDeviceInfo() {
    bledis.setManufacturer("Veiio");
    bledis.setModel("Body Hub");
    bledis.setFirmwareRev("0.0.1"); // Best to use semver
    bledis.begin();
}

void BLEnRF52::setupBLEUart() {
    bleuart.begin();
    // Use the static trampoline functions for callbacks
    bleuart.setRxCallback(static_bleuart_rx_callback);
    bleuart.setNotifyCallback(static_bleuart_notify_callback); // Corrected typo
}

void BLEnRF52::setup_DataPacketChar() {
    DataPacketChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
    DataPacketChar.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    // DataPacketChar.setFixedLen(sizeof(DataPacket)); // You can set this if your packets are always the same size
    DataPacketChar.begin();
    DataPacketChar.write32(0); // Set initial value

    // The service must be started *after* its characteristics are configured
    service.begin();
}

void BLEnRF52::startAdv() {
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addService(service);
    Bluefruit.Advertising.addService(bleuart); // Also advertise UART service
    Bluefruit.ScanResponse.addName();
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244);
    Bluefruit.Advertising.setFastTimeout(30);
    Bluefruit.Advertising.start(0);
}

// --- Static Trampoline Implementations ---

void BLEnRF52::static_connect_callback(uint16_t conn_handle) {
    if (_instance) _instance->onConnect(conn_handle);
}

void BLEnRF52::static_disconnect_callback(uint16_t conn_handle, uint8_t reason) {
    if (_instance) _instance->onDisconnect(conn_handle, reason);
}

void BLEnRF52::static_bleuart_rx_callback(uint16_t conn_hdl) {
    if (_instance) _instance->onBleUartRx(conn_hdl);
}

void BLEnRF52::static_bleuart_notify_callback(uint16_t conn_hdl, bool enabled) {
    if (_instance) _instance->onBleUartNotify(conn_hdl, enabled);
}

// --- Real Callback Handler Implementations ---

void BLEnRF52::onConnect(uint16_t conn_handle) {
    DEBUG_PRINTLN("Connected");
    BLEConnection* conn = Bluefruit.Connection(conn_handle);
    conn->requestPHY();
    conn->requestDataLengthUpdate();
    conn->requestMtuExchange(247);
    delay(100); // Small delay for requests to process
}

void BLEnRF52::onDisconnect(uint16_t conn_handle, uint8_t reason) {
    (void)conn_handle;
    DEBUG_PRINT("Disconnected, reason = 0x");
    // DEBUG_PRINTLN(reason, HEX);
}

// CORRECTED: Buffer data here instead of in get_received()
void BLEnRF52::onBleUartRx(uint16_t conn_hdl) {
    (void)conn_hdl;
    while (bleuart.available()) {
        received_msg += (char)bleuart.read();
    }
}

void BLEnRF52::onBleUartNotify(uint16_t conn_hdl, bool enabled) {
    (void)conn_hdl;
    if (enabled) {
        DEBUG_PRINTLN("BLE UART 'Notify' enabled by client.");
    } else {
        DEBUG_PRINTLN("BLE UART 'Notify' disabled by client.");
    }
}

#endif