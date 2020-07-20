// Tommy Test by TTGO_ESP32_LORA SX1278
// Upload by ESP32_Dev_Module
// For traves Russia
// Akarawat Panwilai
// ต้องรับและส่งด้วย HEX เท่านั้น

#include <SPI.h>
#include <LoRa.h>
#include "mbedtls/aes.h"

// WIFI_LoRa_32 ports

// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)

#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6 //433E6 //915E6 //868E6

String masKey = "=*lo#";
char * key = "uiwyasuisdifysdu"; // Encryp/Decrypt Key 16 digit

void decrypt(unsigned char * chipherText, char * key, unsigned char * outputBuffer) {

  mbedtls_aes_context aes;
  
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)chipherText, outputBuffer);
  mbedtls_aes_free( &aes );
}

void setup() {
  Serial.begin(115200);
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);

  Serial.println("LoRa Receiver MASTER Version");

  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

unsigned char cipherTextOutput[16];//จะต้องกำหนด ขนาดตวามความยาวทั้ง encrypt-decrypt
unsigned char decipheredTextOutput[16];//จะต้องกำหนด ขนาดตวามความยาวทั้ง encrypt-decrypt

/*
  Length = 19
  Pure Debug:=*lo#N01M01999
*/
void loop() {
  delay(10);
  int packetSize = LoRa.parsePacket();
  
  if (packetSize == 19) {
    Serial.println(packetSize);
    String txt = "";
    int i = 0;
    //ห้ามแทรกอะไรลงไปใน loop while
    while (LoRa.available()) {
      cipherTextOutput[i] = (char)LoRa.read();
      //-->Serial.print(String(cipherTextOutput[i]));
      i++;
    }
    /*
    for (int i = 0; i < 16; i++) {
      char str[3];
      sprintf(str, "%02x", (int)cipherTextOutput[i]);
      Serial.print(str);
    }
    */

    decrypt(cipherTextOutput, key, decipheredTextOutput);
    for (int i = 0; i < 16; i++) {
      //-->Serial.print((char)decipheredTextOutput[i]);
      txt += String((char)decipheredTextOutput[i]);
    }
    //# Debug #//
    //-->Serial.println("Pure Debug:" + txt); //ทำการตัดเอา string 3 ตัวหลอก ออก ก่อน

    //=*lo#N01O01999
    if (txt.length() == 14) { // && txt.substring(0, 5) == masKey
      Serial.print(txt);
      Serial.print(" RSSI ");
      Serial.println(LoRa.packetRssi());
      Serial.flush();

    }
  }
}
