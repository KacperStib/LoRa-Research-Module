#include <SPI.h>
#include <LoRa.h>
#include <HardwareSerial.h>

// SPI LoRa
#define MISO 2
#define MOSI 7
#define SCK 10
#define CS 3

// UART 
#define RX_PIN 20
#define TX_PIN 21

HardwareSerial SerialAUX(1);

void setup() {
  Serial.begin(9600);
  SerialAUX.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);  // UART COMM

  while (!Serial);

  Serial.println("LoRa Receiver");

  SPI.begin(SCK, MISO, MOSI, CS);
  
  LoRa.setPins(CS,-1 ,-1 );

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  } else{
    Serial.println("Lora init ok!");
    }
}

void loop() {
  // listen UART
  while (SerialAUX.available()) {
    char c = SerialAUX.read();
    Serial.write(c);   // pokaz na USB co przyszło z UART
  }

  // listen LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
