// Tommy Test by TTGO_ESP32_LORA SX1278
// Upload by ESP32_Dev_Module
// For traves Russia
// Akarawat Panwilai
// ต้องรับและส่งด้วย HEX เท่านั้น

// Format: =*lo#n01RAND(5digit)D01
// V102 add mask key masKey = "=*lo#"
// V103 Can be recieve I/O on ESP32 board
// V103-1 Fix 300 Sec for send output status 1 uint
// V104 Repeat sender remain code 2 times for makesure data sended complete
// V105 Add Encrypt
#include <SPI.h>
#include <LoRa.h>
#include<Arduino.h>
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
#define BAND    433E6 //433E6  //915E6 -- 这里的模式选择中，检查一下是否可在中国实用915这个频段

int counter = 0;

String LoRaUID = "N01"; // the LoRaUID Need to Relate with Collection on mongoDB

String masKey = "=*lo#";  // For LoRa Rssi Add ToKen is "=*lo#"
char * key = "uiwyasuisdifysdu"; // Encryp/Decrypt Key

String strRTC = "999";
String strTmp = "";

const long interval = 300;  /* set delay loop second */
unsigned long prevms = 0;

//# I/O 13, 12, 2, 25, 17, 23, 16
//I/O 13=Breakdowns, 12=Org. Prob., 2=Planned Stops, 25=Changeover, 17=Quality pb, 23=Microstops, 16=Output
const int Sw_Dow = 13;
const int Sw_Org = 12;
const int Sw_Pln = 2;
const int Sw_Cha = 25;
const int Sw_Qua = 17;
const int Sw_Mic = 23;
const int Sw_Out = 16;
int IOState = 0;

void encrypt(char * plainText, char * key, unsigned char * outputBuffer) {

  mbedtls_aes_context aes;

  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_enc( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb( &aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)plainText, outputBuffer);
  mbedtls_aes_free( &aes );
}
void decrypt(unsigned char * chipherText, char * key, unsigned char * outputBuffer) {

  mbedtls_aes_context aes;

  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)chipherText, outputBuffer);
  mbedtls_aes_free( &aes );
}

void setup() {

  Serial.begin(115200);
  while (!Serial);

  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa V105 Initial OK!");
  pinMode(Sw_Dow, INPUT);
  pinMode(Sw_Org, INPUT);
  pinMode(Sw_Pln, INPUT);
  pinMode(Sw_Cha, INPUT);
  pinMode(Sw_Qua, INPUT);
  pinMode(Sw_Mic, INPUT);
  pinMode(Sw_Out, INPUT);

  prevms = millis();

}


String inData;

// Encryption
unsigned char cipherTextOutput[16];
unsigned char decipheredTextOutput[16];
void loop() {

  // Debug Before Receive from Rx Tx
  if (digitalRead(Sw_Dow)) { // Down
    inData = "D01";
  }
  if (digitalRead(Sw_Org)) { // Organiz Problem
    inData = "G01";
  }
  if (digitalRead(Sw_Pln)) { // Plan Stop
    inData = "P01";
  }
  if (digitalRead(Sw_Cha)) { // Change
    inData = "C01";
  }
  if (digitalRead(Sw_Qua)) { // Quality
    inData = "Q01";
  }
  if (digitalRead(Sw_Mic)) { // Micro Stop
    inData = "M01";
  }
  if (digitalRead(Sw_Out)) { // Output
    inData = "O01";
  }

  //--> Send output 1 unit every 5 Min
  unsigned long curms = millis();
  if (inData == "" and curms - prevms >= interval * 1000)
  {
    inData = "O01";
    prevms = curms;
  }

  if (inData != "") {
    String strSend = "";
    int irand = random(10000, 99999);
    int iins = random(100, 999);
    for (int rpt = 0; rpt < 2; rpt++) { // ส่ง 2 ครั้ง เพื่อ Makesure ว่าข้อมูลถึง โดยไปกรองที่ Gateway
      strSend = masKey + LoRaUID + inData + strRTC;
      // Encrypt
      char *plainText = const_cast<char*>(strSend.c_str());
      encrypt(plainText, key, cipherTextOutput);

      LoRa.beginPacket();
      LoRa.print(reinterpret_cast<char*>(cipherTextOutput)); //สำหรับ LoRa ต้องส่งเป็น char เท่านั้น
      LoRa.endPacket();

      delay(2000);
    }
    inData = ""; // Clear recieved buffer
  }

}
