#include "defines.h"

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <TinyGPS.h>

#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <LoRa.h>

HardwareSerial GPSSerial(1);
SoftwareSerial AUXSerial(UART_RX, UART_TX);

// Modules instances
Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_INA219 ina219;
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPS gps;

// Variables
volatile float gps_lat = 0;
volatile float gps_lon = 0;
volatile uint8_t gps_sats = 0;
volatile bool gps_fix = false;

volatile float current_mA = 0, tx_mA = 0;

volatile bool radio_mode = 0;   // RX - 0, TX - 1
volatile int RSSI = 0;

// Tasks handles
TaskHandle_t taskGPSHandle;
TaskHandle_t taskINAHandle;
TaskHandle_t taskOLEDHandle;
TaskHandle_t taskSDHandle;
TaskHandle_t taskLoRaHandle;
TaskHandle_t taskAUXHandle;

void setup() {
  // Turn on LoRa power mosfet
  pinMode(23, OUTPUT);
  digitalWrite(23, 0);

  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // INA219
  if (! ina219.begin())
    Serial.println("Failed to find INA219 chip");
  
  ina219.setCalibration_32V_6A_50mOhm();
  
  // OLED 
  display.begin(OLED_ADDR, true);
  display.display();
  display.clearDisplay();

  // GPS
  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // SD card
  pinMode(LORA_CS, OUTPUT);
  digitalWrite(LORA_CS, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);//, SD_CS);

  if (!SD.begin(SD_CS)){
    Serial.println("Card Mount Failed");
    //return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    //return;
  } 
  else
    Serial.println(cardType);

  writeFile(SD, "/hello.txt", "Hello ");

  // LoRa
  delay(2000);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IO);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
  }
  //pinMode(LORA_CS, OUTPUT);
  //digitalWrite(LORA_CS, HIGH);

  // AUX UART
  AUXSerial.begin(9600);

  // Start Tasks
  xTaskCreatePinnedToCore(taskGPS,  "GPS",   4096, NULL, 1, &taskGPSHandle, 0);
  xTaskCreatePinnedToCore(taskINA,  "INA",   2048, NULL, 2, &taskINAHandle, 0);
  xTaskCreatePinnedToCore(taskOLED, "OLED",  4096, NULL, 3, &taskOLEDHandle, 0);
  xTaskCreatePinnedToCore(taskSD,   "SD",    4096, NULL, 4, &taskSDHandle, 0);
  xTaskCreatePinnedToCore(taskLoRa, "LORA",  4096, NULL, 5, &taskLoRaHandle, 0);
  xTaskCreatePinnedToCore(taskAUX,  "AUX",   2048, NULL, 6, &taskAUXHandle, 0);

}

// ===== TASK GPS =====
void taskGPS(void *pv) {
  float lat, lon;
  unsigned long age;

  for (;;) {
    while (GPSSerial.available()) {
      gps.encode(GPSSerial.read());
    }

    gps_sats = gps.satellites();
    gps.f_get_position(&lat, &lon, &age);

    print_int(gps_sats, TinyGPS::GPS_INVALID_SATELLITES, 5);
    print_float(lat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
    print_float(lon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);

    // Indicate GPS fix only if there are more than 3 sats
    if (lat != TinyGPS::GPS_INVALID_F_ANGLE &&
        lon != TinyGPS::GPS_INVALID_F_ANGLE &&
        gps_sats >= 3) {
      gps_lat = lat;
      gps_lon = lon;
      gps_fix = true;
    } 
    else {
      gps_fix = false;
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

// ===== TASK INA =====
void taskINA(void *pv) {
  for (;;) {
    current_mA = ina219.getCurrent_mA();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ===== TASK OLED =====
void taskOLED(void *pv) {
  for (;;) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    // Current
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Pomiar pradu: ");
    display.print(current_mA, 0);
    display.print(" mA");

    display.setCursor(0, 10);
    display.print("Pik pradu: ");
    display.print(tx_mA);

    // GPS
    display.setCursor(0, 25);
    if (gps_fix) {
      display.print("GPS FIX");
      display.setCursor(0, 35);
      display.print(gps_lat, 5);
      display.print(",");
      display.print(gps_lon, 5);
    } 
    else {
      display.print("NO GPS FIX");
    }

    // Radio
    display.setCursor(0, 50);
    display.print("Tryb LoRa: ");
    if (radio_mode){
      display.print("TX ---->"); 
    }
    else {
      display.print("RX ");
      display.print(RSSI);
      display.print(" dBm");
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ===== TASK SD =====
void taskSD(void *pv) {
  char buf[60];
  for (;;) {
    // Radio TX
    if (radio_mode){
      if (gps_fix)
        snprintf(buf, sizeof(buf), "%f, %f, %f\n", tx_mA, gps_lat, gps_lon);
      else
        snprintf(buf, sizeof(buf), "%f\n", tx_mA);
    }

    // Radio RX
    else{
      if (gps_fix)
        snprintf(buf, sizeof(buf), "%f, %f, %f\n", current_mA, gps_lat, gps_lon);
      else
        snprintf(buf, sizeof(buf), "%f, %d\n", current_mA, RSSI);
    }

    appendFile(SD, "/hello.txt", buf);
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

// ===== TASK LORA =====
void taskLoRa(void *pv) {
  for (;;) {

    // Transmitter
    if (radio_mode){
      LoRa.beginPacket();
      LoRa.print("hello ");
      LoRa.endPacket();
      tx_mA = ina219.getCurrent_mA();
      vTaskDelay(pdMS_TO_TICKS(10000));
    } 
    
    // Receiver
    else {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        while (LoRa.available())
          Serial.print((char)LoRa.read());
        RSSI = LoRa.packetRssi(); 
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}

// ===== TASK AUX =====
void taskAUX(void *pv) {
  static char buffer[10];
  uint8_t idx = 0;

  for (;;) {

    while (AUXSerial.available()) {

      char c = AUXSerial.read();

      // AUX serial only for radio mode selection
      if (c == '\n' || c == '\r') {
        buffer[idx] = '\0';   
        idx = 0;

        if (strcmp(buffer, "RX") == 0) {
          Serial.println("TRYB RX");
          AUXSerial.println("TRYB RX");
          radio_mode = false;
        }
        else if (strcmp(buffer, "TX") == 0) {
          Serial.println("TRYB TX");
          AUXSerial.println("TRYB TX");
          radio_mode = true;
        }
        else {
          Serial.println("Nieznana komenda");
          AUXSerial.println("Nieznana komenda");
        }

      } else {
        if (idx < sizeof(buffer) - 1) {
          buffer[idx++] = c;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void loop() {}

// GPS helpers
static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (GPSSerial.available())
      gps.encode(GPSSerial.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec) {
  if (val == invalid) {
    while (len-- > 1) Serial.print('*');
    Serial.print(' ');
  } else {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . i -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i) Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len) {
  char sz[32];
  if (val == invalid) strcpy(sz, "*******");
  else sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i) sz[i] = ' ';
  if (len > 0) sz[len - 1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

// SD helpers
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}