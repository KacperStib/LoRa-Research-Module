#include "defines.h"

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <TinyGPS.h>

#include <HardwareSerial.h>

HardwareSerial GPSSerial(1);

// Modules instances
Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_INA219 ina219;
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPS gps;

float current_mA = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // Neopixel onboard
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(100, 0, 100));
  pixels.show();

  // INA219
  if (! ina219.begin())
    Serial.println("Failed to find INA219 chip");

  // OLED 
  display.begin(OLED_ADDR, true);
  display.display();
  display.clearDisplay();

  // GPS
  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
}

void loop() {

  // INA219
  current_mA = ina219.getCurrent_mA();
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");

  // Read from GPS
  float flat, flon; 
  unsigned long age;
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);

  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);

  // Show on OLED
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Pomiar pradu:");
  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // 'inverted' text
  display.println(current_mA);
  display.display();

  smartdelay(2000);
}

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