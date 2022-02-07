#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Ticker.h>

#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#include <EasyButton.h>

// Arduino pin where the buttons are connected to
#define BUTTON_ONE_PIN 14
#define BUTTON_TWO_PIN 27
#define BUTTON_THREE_PIN 26
#define BUTTON_FOUR_PIN 25

#define SERVICE_UUID           "4FAFC201-1FB5-459E-8FCC-C5C9C331914B" // UART service UUID
#define CHARACTERISTIC_UUID    "BEB5483E-36E1-4688-B7F5-EA07361B26A8" //Use this characteristic for all JG

// Button1
EasyButton button1(BUTTON_ONE_PIN);
// Button2
EasyButton button2(BUTTON_TWO_PIN);
// Button3
EasyButton button3(BUTTON_THREE_PIN);
// Button4
EasyButton button4(BUTTON_FOUR_PIN);
/** Initialize DHT sensor 1 */
DHTesp dhtSensor1;
TaskHandle_t tempTaskHandle = NULL;

void SendData(String DataBLE);

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
String filterString;
int LED1 = 5;
int LED2 = 18;
int LED3 = 19;
int LED4 = 21;
int dhtPin1 = 23;
bool tasksEnabled = false;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Flags for temperature readings finished */
bool gotNewTemperature = false;
/** Data from sensor 1 */
TempAndHumidity sensor1Data;
/** Data from sensor 2 */

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
 // Serial.println("tempTask loop started");
  while (1) // tempTask loop
  {
    if (tasksEnabled && !gotNewTemperature) { // Read temperature only if old data was processed already
      // Reading temperature for humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
      sensor1Data = dhtSensor1.getTempAndHumidity();  // Read values from sensor 1
      
      gotNewTemperature = true;
    }
    vTaskSuspend(NULL);
  }
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker tempTicker
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

/* Flag if main loop is running */

// Callback function to be called when button1 is pressed
void onButton1Pressed()
{
  digitalWrite(LED1,!digitalRead(LED1)); 
  if(deviceConnected){
    if(digitalRead(LED1)){
       SendData("FThouseON");
     }else{
      SendData("FThouseOFF");
      }
   
  }
}

// Callback function to be called when button2 is pressed
void onButton2Pressed()
{
  digitalWrite(LED2,!digitalRead(LED2)); 
  if(deviceConnected){
    if(digitalRead(LED2)){
       SendData("KitchenON");
     }else{
      SendData("KitchenOFF");
      }
   
  }
}

// Callback function to be called when button3 is pressed
void onButton3Pressed()
{
  digitalWrite(LED3,!digitalRead(LED3)); 
  if(deviceConnected){
    if(digitalRead(LED3)){
       SendData("GarageON");
     }else{
      SendData("GarageOFF");
      }
   
  }
}

// Callback function to be called when button4 is pressed
void onButton4Pressed()
{
  digitalWrite(LED4,!digitalRead(LED4)); 
  if(deviceConnected){
    if(digitalRead(LED4)){
       SendData("BathroomON");
     }else{
      SendData("BathroomOFF");
      }
   
  }
}





class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Client Connected");  
    };
 
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Client Disconnected");
    }
};
 
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
//      filterString = rxValue.c_str(); // Convert to standard c string format
      //digitalWrite(RELAY_PINS[rxValue[0]-48], rxValue[1]-48); 
//      String rxBLE = rxValue.c_str();
      Serial.println(rxValue.c_str());
      String rxBLE = rxValue.c_str();
       if(rxBLE == "FThouseON"){
          digitalWrite(LED1, 0); 
       }else if(rxBLE == "KitchenON"){    
          digitalWrite(LED2, 0); 
      }else if(rxBLE == "GarageON"){    
          digitalWrite(LED3, 0); 
      }else if(rxBLE == "BathroomON"){    
          digitalWrite(LED4, 0); 
      }else if(rxBLE == "FThouseOFF"){
         digitalWrite(LED1, 1); 
      }else if(rxBLE == "KitchenOFF"){
         digitalWrite(LED2, 1); 
      }else if(rxBLE == "GarageOFF"){
         digitalWrite(LED3, 1); 
      }else if(rxBLE == "BathroomOFF"){
         digitalWrite(LED4, 1); 
      }
    }
};

void SendData(String DataBLE) {
  pCharacteristic->setValue(DataBLE.c_str());
  pCharacteristic->notify();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  pinMode(LED1, OUTPUT);      // set the LED pin mode
  pinMode(LED2, OUTPUT);      // set the LED pin mode
  pinMode(LED3, OUTPUT);      // set the LED pin mode
  pinMode(LED4, OUTPUT);      // set the LED pin mode
  /*-----Setup Buletooth----- */
  // Create the BLE Device
  BLEDevice::init("BLE_Relay");
 
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
 
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
                    
  // Create a BLE Characteristic                                              
  pCharacteristic = pService->createCharacteristic(                           
                      CHARACTERISTIC_UUID,                                    
                      BLECharacteristic::PROPERTY_READ   |                    
                      BLECharacteristic::PROPERTY_WRITE  |                    
                      BLECharacteristic::PROPERTY_NOTIFY |                    
                      BLECharacteristic::PROPERTY_INDICATE                      
                    ); 
                                                         
  pCharacteristic->addDescriptor(new BLE2902()); 
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();
  /*-----End Setup Buletooth-----*/  
  button1.begin();
  // Initialize the button2
  button2.begin();
  button3.begin();
  button4.begin();
  // Add the callback function to be called when the button1 is pressed.
  button1.onPressed(onButton1Pressed);
  // Add the callback function to be called when the button2 is pressed.
  button2.onPressed(onButton2Pressed);
   // Add the callback function to be called when the button2 is pressed.
  button3.onPressed(onButton3Pressed);
   // Add the callback function to be called when the button2 is pressed.
  button4.onPressed(onButton4Pressed);


  
//temperature
  dhtSensor1.setup(dhtPin1, DHTesp::DHT22);
  // Start task to get temperature
  xTaskCreatePinnedToCore(
      tempTask,                      /* Function to implement the task */
      "tempTask ",                    /* Name of the task */
      4000,                          /* Stack size in words */
      NULL,                          /* Task input parameter */
      5,                              /* Priority of the task */
      &tempTaskHandle,                /* Task handle. */
      1);                            /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("[ERROR] Failed to start task for temperature update");
  } else {
    // Start update of environment data every 30 seconds
    tempTicker.attach(10, triggerGetTemp);
  }

  // Signal end of setup() to tasks
  tasksEnabled = true;
}
 
void loop() {
  button1.read();
  button2.read();
  button3.read();
  button4.read();
  if (gotNewTemperature) {
    //Serial.println("Sensor 1 data:");
    //Serial.println("Temp: " + String(sensor1Data.temperature,2) + "'C Humidity: " + String(sensor1Data.humidity,1) + "%");
    String th = String(sensor1Data.temperature,2) + ":" + String(sensor1Data.humidity,2);
    SendData(String(sensor1Data.temperature,2));
    SendData(String(sensor1Data.humidity,2));
    if(deviceConnected){
      SendData(th);
    }
    
    gotNewTemperature = false;
  }
  
  
}
