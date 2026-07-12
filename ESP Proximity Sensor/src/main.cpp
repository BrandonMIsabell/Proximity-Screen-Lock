#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// --- FREERTOS TASK HANDLES ---
TaskHandle_t TaskBLEScanHandle = NULL;
TaskHandle_t TaskProximityHandle = NULL;
TaskHandle_t TaskButtonHandle = NULL;
TaskHandle_t TaskRGBEngineHandle = NULL;

// --- INTER-TASK COMMUNICATION ---
// Define a Queue Handle here to safely send RSSI values from Core 0 to Core 1!
// QueueHandle_t rssiQueue;

// --- HARDWARE CONFIGURATION ---
const int BUTTON_1_PIN = 4; 
const int BUTTON_2_PIN = 5;
const int RGB_RED_PIN  = 12;
const int RGB_GRN_PIN  = 13;
const int RGB_BLU_PIN  = 14;

// ==========================================
//               CORE 0 TASKS
// ==========================================

// Task 1: BLE Mesh Scanning
void TaskBLEScan(void *pvParameters) {
    // Write your BLE setup logic here (Runs once upon task initialization)
    
    for (;;) {
        // 1. Scan for nearby BLE devices
        // 2. Filter advertising packets for your specific app identifier
        // 3. Extract the raw RSSI (signal strength)
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Yield to prevent watchdog starvation
    }
}

// Task 2: RSSI Proximity Calculus
void TaskProximity(void *pvParameters) {
    for (;;) {
        // 1. Collect raw RSSI values over a short window
        // 2. Run a digital filter (like a moving average) to smooth out signal spikes
        // 3. Determine if the device crossed the lock or unlock threshold
        // 4. Send the result to Core 1's RGB Engine via a queue or global state
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


// ==========================================
//               CORE 1 TASKS
// ==========================================

// Task 3: Button Debouncing & Software ISR Handling
void TaskButton(void *pvParameters) {
    // Initialize your GPIO input pins here
    
    for (;;) {
        // 1. Monitor state changes on your Momentary Tactile Buttons
        // 2. Perform software debouncing (ensure press is stable for ~50ms)
        // 3. Trigger state transitions (e.g., manual pairing mode, tracking override)
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task 4: RGB State Engine
void TaskRGBEngine(void *pvParameters) {
    // Initialize your PWM channels / GPIO output pins here for the Common Cathode LED
    
    for (;;) {
        // 1. Check current connection or proximity state
        // 2. Drive PWM signals to blend colors (Red = Locked, Cyan/Green = Unlocked, Blue = Pairing)
        // 3. Handle smooth blinking or breathing visual effects
        
        vTaskDelay(pdMS_TO_TICKS(33)); // ~$30\text{Hz}$ refresh rate for smooth animations
    }
}


// ==========================================
//               SYSTEM SETUP
// ==========================================
void setup() {
    Serial.begin(115200);

    // Initialize your Inter-Task Communication queues here
    // rssiQueue = xQueueCreate(10, sizeof(int));

    // CREATE CORE 0 TASKS (The Protocol Core)
    xTaskCreatePinnedToCore(
        TaskBLEScan,          // Function name
        "BLE_Scan",           // Text name for debugging
        4096,                 // Stack size in words (BLE requires a larger stack)
        NULL,                 // Parameter to pass
        2,                    // Priority
        &TaskBLEScanHandle,   // Task handle
        0                     // Pinned to Core 0
    );

    xTaskCreatePinnedToCore(
        TaskProximity,
        "Proximity_Calc",
        2048,
        NULL,
        1,
        &TaskProximityHandle,
        0                     // Pinned to Core 0
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
    // Leave this empty! In a FreeRTOS architecture, execution happens entirely inside your tasks.
    // Putting code here wastes Core 1 processing cycles on an implicitly lower priority setup loop.
    vTaskDelete(NULL); 
}
