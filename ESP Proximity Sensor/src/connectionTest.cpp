// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include <freertos/FreeRTOS.h>
// // potential freertos libras that can be used
// #include <freertos/task.h>
// #include <freertos/queue.h>




// // Unique IDs for the BLE Service and Characteristic
// // (You will need these exact IDs in your Android Kotlin code later!)
// #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// bool deviceConnected = false;

// // Callbacks to know when a phone connects or disconnects
// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//       deviceConnected = true;
//       Serial.println("Phone Connected!");
//     };

//     void onDisconnect(BLEServer* pServer) {
//       deviceConnected = false;
//       Serial.println("Phone Disconnected. Searching again...");
//       // Restart advertising so the phone can reconnect later
//       BLEDevice::startAdvertising(); 
//     }
// };

// void setup() {
//   Serial.begin(115200);
//   Serial.println("Starting BLE Server...");

//   // 1. Name your ESP32 (This is what your phone will see)
//   BLEDevice::init("Proximity_ESP32");

//   // 2. Create the BLE Server
//   BLEServer *pServer = BLEDevice::createServer();
//   pServer->setCallbacks(new MyServerCallbacks());

//   // 3. Create the BLE Service
//   BLEService *pService = pServer->createService(SERVICE_UUID);

//   // 4. Create a BLE Characteristic (Used to send/receive data)
//   BLECharacteristic *pCharacteristic = pService->createCharacteristic(
//                                          CHARACTERISTIC_UUID,
//                                          BLECharacteristic::PROPERTY_READ |
//                                          BLECharacteristic::PROPERTY_WRITE
//                                        );

//   pCharacteristic->setValue("ESP32 is ready!");

//   // 5. Start the service
//   pService->start();

//   // 6. Start broadcasting (advertising) so the phone can find it
//   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//   pAdvertising->addServiceUUID(SERVICE_UUID);
//   pAdvertising->setScanResponse(true);
//   pAdvertising->setMinPreferred(0x06);  
//   pAdvertising->setMinPreferred(0x12);
//   BLEDevice::startAdvertising();
  
//   Serial.println("ESP32 is now broadcasting! Waiting for a phone to connect...");
// }

// void loop() {
//   // We don't need to do anything in the loop right now!
//   // The BLE connection runs in the background.
//   delay(2000); 
// }