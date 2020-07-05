/*=========================================================================
    S550 HUD - This program is designed to illuminate a series of 18 NeoPixel
               LEDs based on engine RPM. While designed to work with the 2015
               and newer Mustangs, this could potentially work with any
               vehicle using a standard OBD-II port.

    HARDWARE: Arduino UNO R3
              Adafruit BLE Shield
              Freematics OBD-UART V2.1
              18 LED NeoPixel Strand (144 LED/m)

    DEVELOPER: Jacob Mast
               jacobmast.net
    -----------------------------------------------------------------------*/
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "BluefruitConfig.h"
#include <EEPROM.h>
#include <OBD2UART.h>
#include <SPI.h>
/*=========================================================================*/

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

      HUD_Type        Selected HUD Mode: 49-Tach, 50-Track, 51-Shift, 52-Custom
      REDLINE         Maximum Engine RPM
      SHIFT           Selected Shift Point RPM
      RPM             Global Variable to hold RPM
      RPM_interval    Number of revs between illuminating two LEDs
      testMode        Bool
      
    -----------------------------------------------------------------------*/
int HUD_Type;
int REDLINE;
int RPM_interval;
int SHIFT;
bool TESTMODE = false;
int testRPM;
bool ascent = true;

/*=========================================================================*/

/*=========================================================================
    Bluetooth LE Config
      Hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user
      selected CS/IRQ/RST                    
      
      MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
      MODE_LED_BEHAVIOUR        LED activity, valid options are
                                    "DISABLE" or "MODE" or "BLEUART" or
                                    "HWUART"  or "SPI"  or "MANUAL"                             
    -----------------------------------------------------------------------*/
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "SPI"
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
/*=========================================================================*/

/*=========================================================================
    OBD-II Config                                                          
    -----------------------------------------------------------------------*/
COBD obd;
/*=========================================================================*/

/*=========================================================================
    NEOPIXEL Config

      LED_COUNT         Number of LEDs in the NeoPixel strand
      LED_PIN           NeoPixel Control Pin
    
      Argument 1        Number of pixels in NeoPixel strip
      Argument 2        Arduino pin number (most are valid)
      Argument 3        Pixel type flags, add together as needed:
        NEO_KHZ800      800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
        NEO_KHZ400      400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
        NEO_GRB         Pixels are wired for GRB bitstream (most NeoPixel products)
        NEO_RGB         Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
        NEO_RGBW        Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
    -----------------------------------------------------------------------*/
#define LED_COUNT 23
#define LED_PIN    6
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
/*=========================================================================*/


/*=========================================================================
    APPLICATION SETUP
    -----------------------------------------------------------------------*/
void setup() {

/* NeoPixel */
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP

/* Bluetooth */
  if (!ble.begin() )       // INITIALIZE BLE module            (REQUIRED)
  {
    
  }

  ble.echo(false);         // DISABLE command echo from Bluefruit
  
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    ble.sendCommandCheckOK("AT+GAPDEVNAME=S550 HUD");         //Configure Bluetooth Display Name
    //ble.setMode(BLUEFRUIT_MODE_DATA);
  }

/* OBD-II */
  obd.begin();            // INITIALIZE OBDII module          (REQUIRED)
  while (!obd.init());    // Wait for OBD Plug configuration  (REQUIRED)

  Serial.print("ATM1");


/* Read EEPROM and Configure HUD */
  Configure_HUD();
  testRPM = 650;
}


/*=========================================================================*/


/*=========================================================================
    APPLICATION MAIN LOOP
    -----------------------------------------------------------------------*/
void loop() {

  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    
    if (TESTMODE) {
        TestHUD();     
    }else{    
      //RESET all LEDs to OFF
      int RPM;
      if (obd.readPID(PID_RPM, RPM))                //Read RPM and store it in the variable "RPM"
      {
        switch (HUD_Type) {
          case 49:
            HUD_1(RPM);
         break;
          case 50:
            HUD_2(RPM);
          break;
          case 51:
            HUD_3(RPM);
          break;
        }
      }
    }
  }else{
    ReadData(ble.buffer);
  }
  //ble.waitForOK();
}
/*=========================================================================*/

void TestHUD()
{
  Configure_HUD();

  if (testRPM>649)
  {
     if (!ascent)
     {
       testRPM = testRPM - 75;
     }else
     {
       testRPM = testRPM +50;
       if(testRPM >=REDLINE)
       {
        ascent = false;
       }
     }
  }else{
    TESTMODE = false;
    testRPM = 650;
  }
  
 switch (HUD_Type) {
    case 49:
      HUD_1(testRPM);
    break;
    case 50:
      HUD_2(testRPM);
    break;
    case 51:
      HUD_3(testRPM);
    break;
  }
}

void Configure_HUD()
{
  HUD_Type = EEPROM.read(0);
  REDLINE = word(EEPROM.read(1), EEPROM.read(2));
  
  switch (HUD_Type) {
    case 49:    
      RPM_interval = (REDLINE - 1000)/22;
      SHIFT = word(EEPROM.read(10), EEPROM.read(11));
      break;
    case 50:     
      RPM_interval = (SHIFT - (SHIFT/2))/10;
      SHIFT = word(EEPROM.read(20), EEPROM.read(21));
      break;
    case 51:
      SHIFT = word(EEPROM.read(40), EEPROM.read(41));
      break;
  }
}

void ReadData(String str)
{
    // Some data was found, its in the buffer
  int start = str.indexOf("HUD=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 4))){
      byte HUD_type = str.charAt(start + 4);
      EEPROM.update(0,HUD_type);
    }
  }
  
  start = str.indexOf("REDLINE=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 8))){
      int redline = str.substring(start + 8).toInt();
      EEPROM.update(1, highByte(redline));
      EEPROM.update(2, lowByte(redline));
    }
  }
  start = str.indexOf("H1S=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 4))){
      int shiftpoint = str.substring(start + 4,start + 8).toInt();      
      EEPROM.update(10, highByte(shiftpoint));
      EEPROM.update(11, lowByte(shiftpoint));
    }
  }
  start = str.indexOf("H1C1=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 5))){
      int r = str.substring(start + 5,start + 8).toInt();
      int g = str.substring(start + 8,start + 11).toInt();
      int b = str.substring(start + 11,start + 14).toInt();
      EEPROM.update(12,r);
      EEPROM.update(13,g);
      EEPROM.update(14,b);
    }
  }
  start = str.indexOf("H1C2=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 5))){
      int r = str.substring(start + 5,start + 8).toInt();
      int g = str.substring(start + 8,start + 11).toInt();
      int b = str.substring(start + 11,start + 14).toInt();
      EEPROM.update(15,r);
      EEPROM.update(16,g);
      EEPROM.update(17,b);
    }
  }
  start = str.indexOf("H2S=");
  if(start >= 0)
  { 
    if(isDigit(str.charAt(start + 4))){
      int shiftpoint = str.substring(start + 4,start + 8).toInt();  
      EEPROM.update(20, highByte(shiftpoint));
      EEPROM.update(21, lowByte(shiftpoint));
    }
  }
  start = str.indexOf("H2C1=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 5))){
      int r = str.substring(start + 5,start + 8).toInt();
      int g = str.substring(start + 8,start + 11).toInt();
      int b = str.substring(start + 11,start + 14).toInt();
      EEPROM.update(22,r);
      EEPROM.update(23,g);
      EEPROM.update(24,b);
    }
  }
  start = str.indexOf("H2C2=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 5))){
      int r = str.substring(start + 5,start + 8).toInt();
      int g = str.substring(start + 8,start + 11).toInt();
      int b = str.substring(start + 11,start + 14).toInt();
      EEPROM.update(25,r);
      EEPROM.update(26,g);
      EEPROM.update(27,b);
    }
  }
  start = str.indexOf("H2C3=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 5))){
      int r = str.substring(start + 5,start + 8).toInt();
      int g = str.substring(start + 8,start + 11).toInt();
      int b = str.substring(start + 11,start + 14).toInt();
      EEPROM.update(28,r);
      EEPROM.update(29,g);
      EEPROM.update(30,b);
    }
  }  
  start = str.indexOf("H3S=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 4))){
      int shiftpoint = str.substring(start + 4,start + 8).toInt();      
      EEPROM.update(40, highByte(shiftpoint));
      EEPROM.update(41, lowByte(shiftpoint));
    }
  }
  start = str.indexOf("HUD3_CLR1=");
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 5))){
      int r = str.substring(start + 5,start + 8).toInt();
      int g = str.substring(start + 8,start + 11).toInt();
      int b = str.substring(start + 11,start + 14).toInt();
      EEPROM.update(42,r);
      EEPROM.update(43,g);
      EEPROM.update(44,b);
    }
  }
  start = str.indexOf("STARTTEST");
  if(start >= 0)
  {
    TESTMODE=true;
    ascent=true;
  }
  start = str.indexOf("CONNECTED");
  if(start >= 0)
  { 
    ble.print("AT+BLEUARTTX=");
    ble.print(word(EEPROM.read(10),EEPROM.read(11)));
    ble.print("-");
    ble.print(EEPROM.read(12));
    ble.print("-");
    ble.print(EEPROM.read(13));
    ble.print("-");
    ble.print(EEPROM.read(14));
    ble.print("-");
    ble.print(EEPROM.read(15));
    ble.print("-");
    ble.print(EEPROM.read(16));
    ble.print("-");
    ble.print(EEPROM.read(17));
    ble.println("-");
    ble.print("AT+BLEUARTTX=");
    ble.print(word(EEPROM.read(20),EEPROM.read(21)));
    ble.print("-");
    ble.print(EEPROM.read(22));
    ble.print("-");
    ble.print(EEPROM.read(23));
    ble.print("-");
    ble.print(EEPROM.read(24));
    ble.print("-");
    ble.print(EEPROM.read(25));
    ble.print("-");
    ble.print(EEPROM.read(26));
    ble.print("-");
    ble.print(EEPROM.read(27));
    ble.print("-");
    ble.print(EEPROM.read(28));
    ble.print("-");
    ble.print(EEPROM.read(29));
    ble.print("-");
    ble.print(EEPROM.read(30));
    ble.println("-");
    ble.print("AT+BLEUARTTX=");
    ble.print(word(EEPROM.read(40),EEPROM.read(41)));
    ble.print("-");
    ble.print(EEPROM.read(42));
    ble.print("-");
    ble.print(EEPROM.read(43));
    ble.print("-");
    ble.println(EEPROM.read(44));
  }
}

void HUD_1(int RPM)
{
  int underShift = floor((SHIFT-1000)/RPM_interval)+1;
  int LEDs_lit = RPM>1000 ? floor((RPM-1000)/RPM_interval)+1 : 0;
  if (LEDs_lit>=0)
  {
    strip.clear();
    for(uint8_t i = 0; i<LEDs_lit; i++)
    {  
      if(i<underShift)
      {
        strip.setPixelColor(i,strip.Color(EEPROM.read(12),EEPROM.read(13),EEPROM.read(14)));
      }
      else
      {
        strip.setPixelColor(i,strip.Color(EEPROM.read(15),EEPROM.read(16),EEPROM.read(17)));
      }
    }
    strip.show();
  }
}

void HUD_2(int RPM)
{   
    
    SHIFT = word(EEPROM.read(20), EEPROM.read(21));
    strip.clear();
    if(RPM>=SHIFT/2+RPM_interval*10)
    {
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)));
    }else if(RPM>=SHIFT/2+RPM_interval*9)
    {
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)),0,11);
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)),12);
    }else if(RPM>=SHIFT/2+RPM_interval*8)
    {
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)),0,10);
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)),13);
    }else if(RPM>=SHIFT/2+RPM_interval*7)
    {
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),0,9);
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),14);
    }else if(RPM>SHIFT/2+RPM_interval*6)
    {
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),0,8);
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),15);
    }else if(RPM>SHIFT/2+RPM_interval*5)
    {
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),0,7);
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),16);
    }else if(RPM>SHIFT/2+RPM_interval*4)
    {
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,6);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),17);
    }else if(RPM>SHIFT/2+RPM_interval*3)
    {
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,5);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),18);
    }else if(RPM>SHIFT/2+RPM_interval*2)
    { 
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,4);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),19);
    }else if(RPM>SHIFT/2+RPM_interval)
    {
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,3);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),20);
    }else if(RPM>REDLINE/2)
    {
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,2);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),21);
    }
    strip.show();
}

void HUD_3(int RPM)
{
    if (RPM>=SHIFT)
    {
      strip.clear();
      strip.fill(strip.Color(EEPROM.read(42),EEPROM.read(43),EEPROM.read(44)));
      strip.show();
    }else{
      strip.clear(); 
      strip.show();
    }
}
