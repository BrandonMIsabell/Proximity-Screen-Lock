#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// --- FREERTOS TASK HANDLES ---
TaskHandle_t TaskBLEServerHandle = NULL;
TaskHandle_t TaskButtonHandle = NULL;
TaskHandle_t TaskRGBEngineHandle = NULL;

// --- INTER-TASK COMMUNICATION ---
volatile bool deviceConnected = false;
volatile bool proximityLockingActive = false;
volatile bool isLocked = true;
volatile int statusDisplayTimer = 0;

BLEServer* pGlobalServer = NULL;

// --- HARDWARE CONFIGURATION ---
#define BUTTON_1_PIN  32 // Bluetooth connection search (reset)
#define BUTTON_2_PIN  33 // System status indicator with LED
#define RGB_RED_PIN   14
#define RGB_GRN_PIN   26
#define RGB_BLU_PIN   27


// --- BLUETOOTH CONECTIONS ---
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) {
          deviceConnected = true;
          Serial.println("Phone Connected!");
        };
    
        void onDisconnect(BLEServer* pServer) {
          deviceConnected = false;
          Serial.println("Phone Disconnected. Searching again...");
          // Restart advertising so the phone can reconnect later
          BLEDevice::startAdvertising(); 
        }
    };

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristic){
        std::string rxValue = pCharacteristic-> getValue();
        if (rxValue.length() > 0) {
            int receivedCommand = rxValue[0];
            
            if (receivedCommand == 0) {
                isLocked = true; 
                Serial.println("Command Received: LOCK");
            } else if (receivedCommand == 1) {
                isLocked = false; 
                Serial.println("Command Received: UNLOCK");
            } else if (receivedCommand == 2) {
                proximityLockingActive = !proximityLockingActive; 
        
                if (proximityLockingActive) {
                    Serial.println("System ARMED: Proximity Locking Active");
                } else {
                    Serial.println("System DISARMED: Manual Mode Only");
                }
            }
        }
    }
};

// ==========================================
//               CORE 0 TASKS
// ==========================================

// Task 1: BLE Mesh Scanning
void TaskBLEServer(void *pvParameters) {
    Serial.println("Starting BLE Server...");

    // Name of ESP32 (This is what the phone will see)
    BLEDevice::init("Proximity_ESP32"); 

    // Creates the BLE Server
   pGlobalServer = BLEDevice::createServer();
    pGlobalServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pGlobalServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic (Used to send/receive data)
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

    pCharacteristic->setValue("ESP32 is ready!");
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Start the service
    pService->start();

    // Start broadcasting (advertising) so the phone can find it
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
  
    Serial.println("ESP32 is now broadcasting! Waiting for a phone to connect...");
    
    for (;;) {
        // The BLE connection runs in the background        
        vTaskDelay(pdMS_TO_TICKS(100)); // Yield to prevent watchdog starvation
    }
}


// ==========================================
//               CORE 1 TASKS
// ==========================================

// Task 2: Button Debouncing & Software ISR Handling
void TaskButton(void *pvParameters) {
    // Initialize your GPIO input pins here
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);
    for (;;) {
        // 1. Monitor state changes on  Momentary Tactile Buttons
        if(digitalRead(BUTTON_1_PIN) == LOW){ // Reset or start bluetooth connection search
           if(!deviceConnected){
                Serial.println("Restarting Connection Search");
                BLEDevice::startAdvertising();
            }
            else{
                Serial.println("Force Phone Disconnect");
                pGlobalServer->disconnect(pGlobalServer->getConnId());
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        if(digitalRead(BUTTON_2_PIN) == LOW){ // Display System state with LED (Make TaskButton higher priority once device is connected)
            statusDisplayTimer = 2000;
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task 3: RGB State Engine
void TaskRGBEngine(void *pvParameters) {
    // Initialize your PWM channels / GPIO output pins here for the Common Cathode LED
    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_GRN_PIN, OUTPUT);
    pinMode(RGB_BLU_PIN, OUTPUT);

    digitalWrite(RGB_RED_PIN, LOW);
    digitalWrite(RGB_GRN_PIN, LOW);
    digitalWrite(RGB_BLU_PIN, LOW);
    for (;;) {
        digitalWrite(RGB_RED_PIN, LOW);
        digitalWrite(RGB_GRN_PIN, LOW);
        digitalWrite(RGB_BLU_PIN, LOW);
        if(!deviceConnected){
            if ((millis() / 500) % 2 == 0) {
                digitalWrite(RGB_BLU_PIN, HIGH);
            }
    
        }
        else if (statusDisplayTimer > 0){
            if(isLocked){
                digitalWrite(RGB_RED_PIN, HIGH);
            }
            else if(!isLocked && !proximityLockingActive){
                digitalWrite(RGB_BLU_PIN, HIGH);
                digitalWrite(RGB_RED_PIN, HIGH);
            }
            else if (proximityLockingActive){
                digitalWrite(RGB_GRN_PIN, HIGH);
            }
            statusDisplayTimer -= 33;
        }
        
        vTaskDelay(pdMS_TO_TICKS(33)); 
    }
}


// ==========================================
//               SYSTEM SETUP
// ==========================================
void setup() {
    Serial.begin(115200);

    // Initialize Future Inter-Task Communication queues here
    // rssiQueue = xQueueCreate(10, sizeof(int));

    // CREATE CORE 0 TASK (The Protocol Core)
    xTaskCreatePinnedToCore(
        TaskBLEServer,          // Function name
        "BLE_Server",           // Text name for debugging
        4096,                   // Stack size in words (BLE requires a larger stack)
        NULL,                   // Parameter to pass
        2,                      // Priority
        &TaskBLEServerHandle,   // Task handle
        0                       // Pinned to Core 0
    );
    
    // CREATE CORE 1 TASKS (The Application Core)
    xTaskCreatePinnedToCore(
        TaskButton,
        "Button_Debounce",
        2048,
        NULL,
        2,
        &TaskButtonHandle,
        1                     // Pinned to Core 1
    );

    xTaskCreatePinnedToCore(
        TaskRGBEngine,
        "RGB_Engine",
        2048,
        NULL,
        1,
        &TaskRGBEngineHandle,
        1                     // Pinned to Core 1
    );
}

void loop() {
    vTaskDelete(NULL); 
}
