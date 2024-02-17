#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <esp_random.h>
#include "control.hpp"

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

#define MODEL_NUMBER "E32CAR"
#define MANUFACTURER_DATA "E32CAR"
#define SOFTWARE_REVISION "1.2.0"
#define SERIAL_NUMBER "E32CAR1000001"
#define FIRMWARE_REVISION "1.2.0"
#define HARDWARE_REVISION "1.2.0"

#define RGB_LED 38

// motor pin
#define IN1_PIN 5
#define IN2_PIN 4
#define IN3_PIN 6
#define IN4_PIN 7
#define IN5_PIN 11
#define IN6_PIN 12
#define IN7_PIN 14
#define IN8_PIN 13

#define POINT_X 100
#define POINT_Y 100

uint8_t  lastPointX = 100;
uint8_t  lastPointY = 100;

Motor leftFrontMotor = Motor();
Motor rightFrontMotor = Motor();
Motor leftRearMotor = Motor();
Motor rightRearMotor = Motor();

Control control = Control();


static NimBLEServer *pServer;

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *pServer) {
        Serial.println("Client connected");
        Serial.println("Multi-connect support: start advertising");
        NimBLEDevice::startAdvertising();
    };

    /** Alternative onConnect() method to extract details of the connection.
     *  See: src/ble_gap.h for the details of the ble_gap_conn_desc struct.
     */
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) {
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

    void onDisconnect(NimBLEServer *pServer) {
        Serial.println("Client disconnected - start advertising");
        NimBLEDevice::startAdvertising();
    };

    void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc) {
        Serial.printf("MTU updated: %u for connection ID: %u\n", MTU, desc->conn_handle);
    };

/********************* Security handled here **********************
****** Note: these are the same return values as defaults ********/
    uint32_t onPassKeyRequest() {
        Serial.println("Server Passkey Request");
        /**
         * This should return a random 6 digit number for security
         */
        uint32_t pwd = esp_random() % 1000000 + 1;
        return pwd;
    };

    bool onConfirmPIN(uint32_t pass_key) {
        Serial.print("The passkey YES/NO number: ");
        Serial.println(pass_key);
        /** Return false if passkeys don't match. */
        return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc *desc) {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!desc->sec_state.encrypted) {
            NimBLEDevice::getServer()->disconnect(desc->conn_handle);
            Serial.println("Encrypt connection failed - disconnecting client");
            return;
        }
        Serial.println("Starting BLE work!");
    };
};

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic *pCharacteristic) {
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(": onRead(), value: ");
        Serial.println(pCharacteristic->getValue().c_str());
    };

    void onWrite(NimBLECharacteristic *pCharacteristic) {
//        Serial.print(pCharacteristic->getUUID().toString().c_str());
        std::string json = pCharacteristic->getValue();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (!error) {
            uint8_t pointX = doc["pointX"];
            uint8_t pointY = doc["pointY"];
            // 运行车辆
            if (pointX != lastPointX) {
                lastPointX = pointX;
                Serial.print("pointX: ");
                Serial.println(pointX);
            }
            if (pointY != lastPointY) {
                lastPointY = pointY;
                Serial.print("pointY: ");
                Serial.println(pointY);
            }
            control.run(pointX, pointY);
            uint8_t padA = doc["padA"];
            uint8_t padB = doc["padB"];
            uint8_t padC = doc["padC"];
            uint8_t padD = doc["padD"];
            if (padA == 1) {
                neopixelWrite(RGB_LED, 0, 255, 255);
            }
            if (padB == 1) {
                neopixelWrite(RGB_LED, 0, 0, 0);
            }
        }
    };

    /** Called before notification or indication is sent,
     *  the value can be changed here before sending if desired.
     */
    void onNotify(NimBLECharacteristic *pCharacteristic) {
        Serial.println("Sending notification to clients");
    };


    /** The status returned in status is defined in NimBLECharacteristic.h.
     *  The value returned in code is the NimBLE host return code.
     */
    void onStatus(NimBLECharacteristic *pCharacteristic, Status status, int code) {
        String str = ("Notification/Indication status code: ");
        str += status;
        str += ", return code: ";
        str += code;
        str += ", ";
        str += NimBLEUtils::returnCodeToString(code);
        Serial.println(str);
    };

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue) {
        String str = "Client ID: ";
        str += desc->conn_handle;
        str += " Address: ";
        str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
        if (subValue == 0) {
            str += " Unsubscribed to ";
        } else if (subValue == 1) {
            str += " Subscribed to notfications for ";
        } else if (subValue == 2) {
            str += " Subscribed to indications for ";
        } else if (subValue == 3) {
            str += " Subscribed to notifications and indications for ";
        }
        str += std::string(pCharacteristic->getUUID()).c_str();

        Serial.println(str);
    };
};


/** Define callback instances globally to use for multiple Charateristics */
static CharacteristicCallbacks chrCallbacks;

void initCar() {
    // write your initialization code here
    leftFrontMotor.attachMotorInit(IN1_PIN, IN2_PIN, 0 , 0);
    rightFrontMotor.attachMotorInit(IN3_PIN, IN4_PIN, 0, 1);
    leftRearMotor.attachMotorInit(IN5_PIN, IN6_PIN, 1, 0);
    rightRearMotor.attachMotorInit(IN7_PIN, IN8_PIN, 1, 1);
    // 初始化控制器
    control.motorInit(leftFrontMotor, rightFrontMotor, leftRearMotor, rightRearMotor);
    control.pointInit(POINT_X, POINT_Y);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting NimBLE Server");
    // 初始化
    initCar();

    pinMode(RGB_LED, OUTPUT);

    /** sets device name */
    NimBLEDevice::init(MODEL_NUMBER);

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

    NimBLEService *pService = pServer->createService(SERVICE_UUID_DEVICE_INFORMATION);
    NimBLECharacteristic *pCharacteristic_Model_Number = pService->createCharacteristic(
            CHARACTERISTIC_UUID_MODEL_NUMBER,
            NIMBLE_PROPERTY::READ
    );

    pCharacteristic_Model_Number->setValue(MODEL_NUMBER);

    BLECharacteristic *pCharacteristic_Software_Revision = pService->createCharacteristic(
            CHARACTERISTIC_UUID_SOFTWARE_REVISION,
            NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Software_Revision->setValue(SOFTWARE_REVISION);

    BLECharacteristic *pCharacteristic_Serial_Number = pService->createCharacteristic(
            CHARACTERISTIC_UUID_SERIAL_NUMBER,
            NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Serial_Number->setValue(SERIAL_NUMBER);

    BLECharacteristic *pCharacteristic_Firmware_Revision = pService->createCharacteristic(
            CHARACTERISTIC_UUID_FIRMWARE_REVISION,
            NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Firmware_Revision->setValue(FIRMWARE_REVISION);

    BLECharacteristic *pCharacteristic_Hardware_Revision = pService->createCharacteristic(
            CHARACTERISTIC_UUID_HARDWARE_REVISION,
            NIMBLE_PROPERTY::READ
    );
    pCharacteristic_Hardware_Revision->setValue(HARDWARE_REVISION);

    BLECharacteristic *pCharacteristic_Peripheral_Control_Parameters = pService->createCharacteristic(
            CHARACTERISTIC_UUID_PERIPHERAL_CONTROL_PARAMETERS,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::NOTIFY
    );
//    pCharacteristic_Hardware_Revision->setValue("AAA");
    pCharacteristic_Peripheral_Control_Parameters->setCallbacks(&chrCallbacks);

    /** Start the services when finished creating all Characteristics and Descriptors */
    pService->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    /** Add the services to the advertisment data **/
    pAdvertising->addServiceUUID(pService->getUUID());
    /** Add the services to the manufacturer data **/
    pAdvertising->setManufacturerData(MANUFACTURER_DATA);


    /** If your device is battery powered you may consider setting scan response
     *  to false as it will extend battery life at the expense of less data sent.
     */
    pAdvertising->setScanResponse(false);
    pAdvertising->start();

    Serial.println("Advertising Started");
}


void loop() {
    /** Do your thing here, this just spams notifications to all connected clients */

    delay(2000);
}