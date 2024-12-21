#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>

#define POWER_PIN 38
#define START_UP_PIN 39

// Service - Device information
#define SERVICE_UUID_DEVICE_INFORMATION        "180A"

// Characteristic - Model Number String - 0x2A24
#define CHARACTERISTIC_UUID_MODEL_NUMBER       "2A24"
// Characteristic - Serial Number String - 0x2A25
#define CHARACTERISTIC_UUID_SERIAL_NUMBER      "2A25"
// Characteristic - Firmware Revision String - 0x2A26
#define CHARACTERISTIC_UUID_FIRMWARE_REVISION  "2A26"
// Characteristic - Hardware Revision String - 0x2A27
#define CHARACTERISTIC_UUID_HARDWARE_REVISION  "2A27"
// Characteristic - Software Revision String - 0x2A28
#define CHARACTERISTIC_UUID_SOFTWARE_REVISION  "2A28"
// Characteristic - Peripheral Control Parameters String - 0x2A29
#define CHARACTERISTIC_UUID_PERIPHERAL_CONTROL_PARAMETERS  "2A29"

#define MODEL_NUMBER "艾塔智能灯泡_"
#define MANUFACTURER_DATA "550201AA"
#define SOFTWARE_REVISION "1.0.0"
#define SERIAL_NUMBER "E32BULB1000001"
#define FIRMWARE_REVISION "1.0.0"
#define HARDWARE_REVISION "1.0.0"
#define START_UP "5501AA"
#define STOP "5502AA"
#define CHANGE_COLOR "5503AA"
#define WIFI_CONFIG "5504AA"
#define MQTT_CONFIG "5505AA"

uint8_t startUpFlag = 0;

uint8_t macAddress[6];
String CHIP_ID;

// 定义一个枚举来表示不同的命令类型
enum CommandType
{
    COMMAND_START_UP,
    COMMAND_STOP,
    COMMAND_CHANGE_COLOR,
    COMMAND_WIFI_CONFIG,
    COMMAND_MQTT_CONFIG,
    COMMAND_UNKNOWN
};

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static NimBLEServer* pServer;
NimBLECharacteristic* pCharacteristic;

// 电源初始化
void power_init();
// 蓝牙初始化
void ble_init();
// wifi 初始化
void wifi_init();
// 开机
void start_up();
// 关机
void stop();
// 切换颜色
void change_color(uint8_t r, uint8_t g, uint8_t b);
// wifi 配置
void wifi_config();
// mqtt 配置
void mqtt_config();

// 函数根据输入的字符串返回对应的命令类型
CommandType matchCommand(const std::string& input)
{
    if (input == START_UP)
    {
        return COMMAND_START_UP;
    }
    else if (input == STOP)
    {
        return COMMAND_STOP;
    }
    else if (input == CHANGE_COLOR)
    {
        return COMMAND_CHANGE_COLOR;
    }
    else if (input == WIFI_CONFIG)
    {
        return COMMAND_WIFI_CONFIG;
    }
    else if (input == MQTT_CONFIG)
    {
        return COMMAND_MQTT_CONFIG;
    }
    else
    {
        return COMMAND_UNKNOWN;
    }
}

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer* pServer)
    {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };

    /** Alternative onConnect() method to extract details of the connection.
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc)
    {
        Serial.print("Client address: ");
        Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        /** We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments, try for 5x interval time for best results.
         */
        pServer->updateConnParams(desc->conn_handle, 24, 48, 0, 60);
    };

    void onDisconnect(NimBLEServer* pServer)
    {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc)
    {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };

    /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest()
    {
        Serial.println("Server Passkey Request");
        /**
         * This should return a random 6 digit number for security
         */
        uint32_t pwd = esp_random() % 1000000 + 1;
        return pwd;
    };

    bool onConfirmPIN(uint32_t pass_key)
    {
        Serial.print("The passkey YES/NO number: ");
        Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc)
    {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted)
        {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic* pCharacteristic)
    {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic* pCharacteristic)
    {
        //        Serial.print(pCharacteristic->getUUID().toString().c_str());
        std::string json = pCharacteristic->getValue();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (!error)
        {
            const std::string command = doc["command"];
            const uint8_t r = doc["r"];
            const uint8_t g = doc["g"];
            const uint8_t b = doc["b"];

            const CommandType commandType = matchCommand(command);
            switch (commandType)
            {
            case COMMAND_START_UP:
                Serial.println("start up");
                start_up();
                break;
            case COMMAND_STOP:
                Serial.println("stop");
                stop();
                break;
            case COMMAND_CHANGE_COLOR:
                Serial.println("change color");
                change_color(r, g, b);
                break;
            case COMMAND_WIFI_CONFIG:
                Serial.println("WIFI config");
                wifi_config();
                break;
            case COMMAND_MQTT_CONFIG:
                Serial.println("MQTT config");
                mqtt_config();
                break;
            default:
                Serial.println("unknown command");
                break;
            }
        }
    };

    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic* pCharacteristic)
    {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code)
    {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", ";
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue)
    {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if (subValue == 0)
        {
            str += " Unsubscribed to ";
        }
        else if (subValue == 1)
        {
            str += " Subscribed to notfications for ";
        }
        else if (subValue == 2)
        {
            str += " Subscribed to indications for ";
        }
        else if (subValue == 3)
        {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};


/** Define callback instances globally to use for multiple Charateristics */
static CharacteristicCallbacks chrCallbacks;

void setup()
{
    // write your initialization code here
    // 初始化 CHIP_ID
    WiFi.macAddress(macAddress);
    char hexString[3];
    sprintf(hexString, "%02X", macAddress[3]);
    CHIP_ID.concat(hexString);
    sprintf(hexString, "%02X", macAddress[4]);
    CHIP_ID.concat(hexString);
    sprintf(hexString, "%02X", macAddress[5]);
    CHIP_ID.concat(hexString);
    // 初始化调试串口
    Serial.begin(115200);
    // 初始化蓝牙
    ble_init();
    // 电源初始化
    power_init();
}

void loop()
{
    if (pServer->getConnectedCount())
    {
        NimBLEService* pService = pServer->getServiceByUUID(SERVICE_UUID_DEVICE_INFORMATION);
        if (pService)
        {
            NimBLECharacteristic* pCharacteristic = pService->getCharacteristic(
                CHARACTERISTIC_UUID_PERIPHERAL_CONTROL_PARAMETERS);
            if (pCharacteristic)
            {
                pCharacteristic->setValue(digitalRead(START_UP_PIN) == 1 ? START_UP : STOP);
                pCharacteristic->notify(true);
            }
        }
    }
    Serial.printf("startUpFlag: %d\n", digitalRead(START_UP_PIN));
    delay(2000);
}


void ble_init()
{
    String deviceName = MODEL_NUMBER;
    deviceName.concat(CHIP_ID);
    Serial.println(deviceName);

    /** sets device name */
    NimBLEDevice::init("");

    /** Optional: set the transmit power, default is 3db */
#ifdef ESP_PLATFORM
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
#else
    NimBLEDevice::setPower(9); /** +9db */
#endif

    /** Set the IO capabilities of the device, each option will trigger a different pairing method.
     *  BLE_HS_IO_DISPLAY_ONLY    - Passkey pairing
     *  BLE_HS_IO_DISPLAY_YESNO   - Numeric comparison pairing
     *  BLE_HS_IO_NO_INPUT_OUTPUT - DEFAULT setting - just works pairing
     */
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); // use passkey
    //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_YESNO); //use numeric comparison

    /** 2 different ways to set security - both calls achieve the same result.
     *  no bonding, no man in the middle protection, secure connections.
     *
     *  These are the default values, only shown here for demonstration.
     */
    //NimBLEDevice::setSecurityAuth(false, false, true);
    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID_DEVICE_INFORMATION);
    NimBLECharacteristic* pCharacteristic_Model_Number = pService->createCharacteristic(
        CHARACTERISTIC_UUID_MODEL_NUMBER,
        NIMBLE_PROPERTY::READ
    );

    pCharacteristic_Model_Number->setValue(MODEL_NUMBER);

    BLECharacteristic* pCharacteristic_Software_Revision = pService->createCharacteristic(
        CHARACTERISTIC_UUID_SOFTWARE_REVISION,
        NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Software_Revision->setValue(SOFTWARE_REVISION);

    BLECharacteristic* pCharacteristic_Serial_Number = pService->createCharacteristic(
        CHARACTERISTIC_UUID_SERIAL_NUMBER,
        NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Serial_Number->setValue(SERIAL_NUMBER);

    BLECharacteristic* pCharacteristic_Firmware_Revision = pService->createCharacteristic(
        CHARACTERISTIC_UUID_FIRMWARE_REVISION,
        NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Firmware_Revision->setValue(FIRMWARE_REVISION);

    BLECharacteristic* pCharacteristic_Hardware_Revision = pService->createCharacteristic(
        CHARACTERISTIC_UUID_HARDWARE_REVISION,
        NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Hardware_Revision->setValue(HARDWARE_REVISION);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_PERIPHERAL_CONTROL_PARAMETERS,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::NOTIFY
    );
    //    pCharacteristic_Hardware_Revision->setValue("AAA");
    pCharacteristic->setCallbacks(&chrCallbacks);

    /** Start the services when finished creating all Characteristics and Descriptors */
    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pService->getUUID());
    /** Add the services to the manufacturer data **/
    pAdvertising->setManufacturerData(MANUFACTURER_DATA);


    /** If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->setScanResponse(true);
    NimBLEAdvertisementData advertisementData;
    advertisementData.setName(deviceName.c_str());
    pAdvertising->setScanResponseData(advertisementData);
    pAdvertising->start();

    Serial.println("Advertising Started");
}


/*************************      电源初始化       ****************************/
void power_init()
{
    // 设置电源引脚
    pinMode(POWER_PIN, OUTPUT);
    neopixelWrite(POWER_PIN, 0, 0, 0);
    // 设置开机状态引脚
    pinMode(START_UP_PIN, OUTPUT);
}

/*************************      开机       ****************************/
void start_up()
{
    // 打开电源
    neopixelWrite(POWER_PIN, 255, 255, 255);
    digitalWrite(START_UP_PIN, HIGH);
}

/*************************      关机       ****************************/
void stop()
{
    // 关闭电源
    neopixelWrite(POWER_PIN, 0, 0, 0);
    digitalWrite(START_UP_PIN, LOW);
}

/*************************      切换颜色       ****************************/
void change_color(uint8_t r, uint8_t g, uint8_t b)
{
    // 切换颜色
    neopixelWrite(POWER_PIN, r, g, b);
}

/*************************      wifi 配置       ****************************/
void wifi_config()
{
    // 设置 wifi 参数
}

/*************************      mqtt 配置       ****************************/
void mqtt_config()
{
    // 设置 mqtt 参数
}
