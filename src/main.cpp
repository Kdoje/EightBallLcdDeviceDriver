#include <Arduino.h>
#include <SPI.h> // For SPI communication
#include <Wire.h>
#include <DFRobot_BMI160.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

DFRobot_BMI160 imu;

static const int8_t I2C_ADDR = 0x68; // Use 0x69 if SA0 pulled high
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite bgSprite = TFT_eSprite(&tft);   // Background sprite
TFT_eSprite textSprite = TFT_eSprite(&tft); // Text sprite
TFT_eSprite shakeSprite = TFT_eSprite(&tft); // Text sprite

volatile int run = 0;

char buff[32];

const int cw = tft.width()/2;
const int ch = tft.height()/2;
const int s = min(cw/4,ch/4);

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

bool deviceConnected = false;

// UUIDs for the service and characteristic
#define bleServerName "BME280_ESP32"

#define SERVICE_UUID        "8c380000-10bd-4fdb-ba21-1922d6cf860d"
#define CHARACTERISTIC_UUID "8c380002-10bd-4fdb-ba21-1922d6cf860d"


void renderFrame(const String &text)
{
  // Clear only the text sprite
  textSprite.fillSprite(TFT_BLUE);  // same color as circle background

  // Set text color and background
  textSprite.setTextColor(TFT_WHITE, TFT_BLUE);

  // Draw the provided text instead of hardcoded "Ask!"
  textSprite.drawString(text, 0, 10);  // position relative to sprite

  // Push the text sprite at the circle position (centered)
  textSprite.pushSprite(30, 60);  // top-left corner of circle is ~24,60
}
 
void renderShake(const String &text) {
    // Clear only the text sprite
  textSprite.fillSprite(TFT_BLACK);  // same color as circle background

  // Set text color and background
  textSprite.setTextColor(TFT_WHITE, TFT_BLACK);

  // Draw the provided text instead of hardcoded "Ask!"
  textSprite.drawString(text, 0, 10);  // position relative to sprite

  // Push the text sprite at the circle position (centered)
  textSprite.pushSprite(30, 90);  // top-left corner of circle is ~24,60
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
    pServer->startAdvertising(); // Restart advertising
  }
};

// Callback to handle writes to the characteristic
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Data received: ");
      Serial.println(value.c_str());
      renderFrame(value.c_str());
    }
  }
};

void setup()
{
  Serial.begin(115200);
  Serial.println("starting");
  // SPI.begin(18, -1, 23, -1); // SCK=18, MISO not used, MOSI=23
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_CYAN);

  bgSprite.createSprite(128, 128);
  bgSprite.fillSprite(TFT_WHITE);

  // Draw static elements: title and circle
  bgSprite.setTextColor(TFT_BLACK, TFT_WHITE);
  bgSprite.setTextSize(1);
  bgSprite.drawString("Magic 8 Ball", 10, 10);

  bgSprite.fillCircle(64, 80, 40, TFT_BLUE);

  // Push the background once to the screen
  bgSprite.pushSprite(0, 0);

  // Create text sprite (only covers the circle)
  // Keep it slightly larger than text for padding
  textSprite.createSprite(90, 40);  // width=80, height=40
  textSprite.setTextSize(2);  
  shakeSprite.createSprite(90, 40); 
  shakeSprite.setTextSize(2);
  // TODO Check if this gets the accel.
  // Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22
  // Optional: soft reset
  if (imu.softReset() != BMI160_OK)
  {
    Serial.println("BMI160 reset failed!");
    while (true)
    {
      delay(1000);
    }
  }

  // Initialize I2C with the address
  if (imu.I2cInit(I2C_ADDR) != BMI160_OK)
  {
    Serial.printf("BMI160 I2C init failed at addr 0x%02X\n", I2C_ADDR);
    while (true)
    {
      delay(1000);
    }
  }
  Serial.println("creating server");
  // Create BLE Server
    // Create the BLE Device
  BLEDevice::init(bleServerName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("server created");

  // Create BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->setValue("Hello BLE");
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // Helps with iOS connections
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE server is running and advertising...");
  renderFrame("Waiting");

}

void loop() {
  int i = 0;
  int rslt;
  int16_t accelGyro[6] = {0};

  // get both accel and gyro data from bmi160
  // parameter accelGyro is the pointer to store the data
  rslt = imu.getAccelGyroData(accelGyro);
  if (rslt == 0)
  {
    for (i = 0; i < 6; i++)
    {
      if (i < 3)
      {
        // the first three are gyro data
        //  Serial.print(accelGyro[i]*3.14/180.0);Serial.print("\t");
      }
      else
      {
        // the following three data are accel data
        // Serial.print(accelGyro[i] / 16384.0);
        // Serial.print("\t");
      }
    }
    float magnitude_g = sqrt(accelGyro[3] / 16384.0 * accelGyro[3] / 16384.0 + accelGyro[4] / 16384.0 * accelGyro[4] / 16384.0 + accelGyro[5] / 16384.0 * accelGyro[5] / 16384.0);
    renderShake(String(magnitude_g));
  }
  else
  {
    Serial.println("err");
  }
  delay(200);
}
