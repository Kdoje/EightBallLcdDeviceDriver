#include <Arduino.h>
// #include <Adafruit_GFX.h>    // Core graphics library
// #include <Adafruit_ST7735.h> // ST7735 driver
#include <SPI.h> // For SPI communication
#include <Wire.h>
#include <DFRobot_BMI160.h>
#include <TFT_eSPI.h>

// #define TFT_CS 5
// #define TFT_RST 4
// #define TFT_DC 2

// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

DFRobot_BMI160 imu;

static const int8_t I2C_ADDR = 0x68; // Use 0x69 if SA0 pulled high
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite bgSprite = TFT_eSprite(&tft);   // Background sprite
TFT_eSprite textSprite = TFT_eSprite(&tft); // Text sprite
volatile int run = 0;

char buff[32];

const int cw = tft.width()/2;
const int ch = tft.height()/2;
const int s = min(cw/4,ch/4);

void renderFrame()
{
  // Clear only the text sprite
  textSprite.fillSprite(TFT_BLUE);  // same color as circle background

  textSprite.setTextColor(TFT_WHITE, TFT_BLUE);
  textSprite.drawString("Ask!", 0, 10);  // position relative to sprite

  // Push the text sprite at the circle position (centered)
  textSprite.pushSprite(30, 60);  // top-left corner of circle is ~24,60
}

void renderFrame2()
{
  // Clear only the text sprite
  textSprite.fillSprite(TFT_BLUE);  // same color as circle background

  textSprite.setTextColor(TFT_WHITE, TFT_BLUE);
  textSprite.drawString("Now!", 0, 10);  // position relative to sprite

  // Push the text sprite at the circle position (centered)
  textSprite.pushSprite(30, 60);  // top-left corner of circle is ~24,60
}


void setup()
{
  Serial.begin(115200);
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
  textSprite.createSprite(50, 40);  // width=80, height=40
  textSprite.setTextSize(2);  

  // TODO Check if this gets the accel.
  // Wire.begin(21, 22);  // SDA = GPIO21, SCL = GPIO22
  // Optional: soft reset
  if (imu.softReset() != BMI160_OK)
  {
    //  Serial.println("BMI160 reset failed!");
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
  // renderFrame();
  //   tft.init();
  // tft.fillScreen(TFT_BLACK);  
  // tft.setRotation(0);

  // tft.setTextFont(1);
  // tft.setTextSize(2);
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // tft.setTextDatum(CC_DATUM);
  // tft.drawString("Makerguides", ch, cw+s);

  // tft.fillCircle(ch, cw/2+s/2, s/2, TFT_RED);
  // tft.fillRect(1.5*ch-s, cw/2, s, s, TFT_GREEN);
  // tft.fillTriangle(ch/2, cw/2, ch/2+s, cw/2, ch/2, cw/2+s, TFT_BLUE);
}

void loop()
{  
  renderFrame();
  delay(300);
  renderFrame2();
  delay(300);
}

void old_loop() {
  Serial.println("hi");
  //  int16_t accelData[3] = {0, 0, 0};
  // // Fetch only accelerometer data
  // if (imu.getAccelData(accelData) == BMI160_OK) {
  //   // Raw data are 16‑bit signed values representing acceleration
  //   // According to datasheet for ±2g range: sensitivity = 16384 LSB/g :contentReference[oaicite:4]{index=4}
  //   const float SENSITIVITY_2G = 16384.0f;
  //   float ax_g = accelData[0] / SENSITIVITY_2G;
  //   float ay_g = accelData[1] / SENSITIVITY_2G;
  //   float az_g = accelData[2] / SENSITIVITY_2G;

  //   // Compute overall (vector) acceleration in g
  //   float magnitude_g = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);

  //   Serial.printf("Accel (g): X=%.3f, Y=%.3f, Z=%.3f, |a|=%.3f g\n",
  //                 ax_g, ay_g, az_g, magnitude_g);
  //   Serial.println(run++);

  //   //tft.printf("Current accel is %.3f", magnitude_g);

  // } else {
  //   Serial.println("Failed to read accelerometer data.");
  // }
  // delay(100);
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

    // tft.setCursor(0, 20);
    // tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    // sprintf(buff, "accel: %4.3f", magnitude_g);
    // tft.print(buff);

    // Serial.println();
  }
  else
  {
    Serial.println("err");
  }
  renderFrame();
  delay(200);
}

// void setup() {
//   Serial.begin(115200);
//   Wire.begin(21, 22);
// }

// void loop() {
//   Serial.println("Scanning...");
//   for (int i = 1; i < 127; i++) {
//     Wire.beginTransmission(0x68);
//     if (Wire.endTransmission() == 0) {
//       Serial.print("Found device at 0x");
//       Serial.println(i, HEX);
//     }
//   }
//   delay(2000);
// }