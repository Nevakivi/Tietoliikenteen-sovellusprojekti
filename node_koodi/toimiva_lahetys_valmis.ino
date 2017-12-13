
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <RH_RF95.h>
Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1);
/* for feather32u4 */
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;

int i=0;

void lahetys(){
    uint16_t jpglen = cam.frameLength();
    int32_t time = millis();
    byte wCount = 0; // For counting # of writes
    byte vahvistus=5;
    char radiopacket[1] = "6";
    cam.takePicture();
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
 
    pinMode(8, OUTPUT);
    // Read all the data up to # bytes!
    rf95.send((uint8_t *)radiopacket,1);
    if (rf95.waitAvailableTimeout(1000))
      { 
    // Should be a reply message for us now   
      if (rf95.recv(buf, &len))
        {
        Serial.print("Got reply: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);    
        if(buf[0]==57)
        {
          while (jpglen > 0) 
            {
            // read 32 bytes at a time;
            uint8_t *buffer;
            uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
            
            buffer = cam.readPicture(bytesToRead);
            rf95.send((uint8_t *)buffer,bytesToRead);
            rf95.waitPacketSent();
            if(++wCount >= 64) 
              { // Every 2K, give a little feedback so it doesn't appear locked up
                Serial.print('.');
                wCount = 0;
              }
            jpglen -= bytesToRead;
            }
        }
        else
          Serial.println("väärä tarkiste");

        Serial.println("Waiting for packet to complete..."); delay(10);
        time = millis() - time;
        Serial.println("done!");
        Serial.print(time); Serial.println(" ms elapsed");
        delay(100);
        cam.reset();
        }
      delay(1000);
      }
    else 
      Serial.println("ime sitä"); 
}

void vastaanotto()
{
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
    {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
  }  
}

void setup() 
{ 
  while (!Serial);
  Serial.begin(9600);

  Serial.println("asd");
  if (!cap.begin(0x5A)) 
  {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);


  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) 
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  delay(100);
  Serial.println("VC0706 Camera snapshot test");
  
  cam.begin(); 
 
  delay(100);
  cam.setImageSize(VC0706_160x120);          // small
  delay(100);
  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");
 

}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{   
  currtouched = cap.touched();
  
  if ((currtouched & _BV(1)) && !(lasttouched & _BV(1)) ) 
    {
      Serial.print(i); Serial.println("lähetys");
      
      lahetys();
      vastaanotto();
    }
}


