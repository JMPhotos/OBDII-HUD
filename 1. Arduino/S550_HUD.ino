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
      TESTMODE        Boolean used to loop through LED display once
      testRPM         Placeholder used for TestMode simulated Engine RPMs
      ascent          Boolean to control ascending or decending RPMs during TestMode
      
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
      //Could do some error checking here...I'm ignoring it.
  }

  ble.echo(false);         // DISABLE command echo from Bluefruit
  
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    ble.sendCommandCheckOK("AT+GAPDEVNAME=S550 HUD");         //Configure Bluetooth Display Name
  }

/* OBD-II */
  obd.begin();            // INITIALIZE OBDII module          (REQUIRED)
  while (!obd.init());    // Wait for OBD Plug configuration  (REQUIRED)

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
  
  // If we have data from the BLE module, read it, otherwise operate the HUD.
  // By default, when there's no BLE data, the readline function above returns "OK"
  // NOTE that the strcmp function operates counterintuitively, i.e: if the left
  // and right sides are the same, it returns 0. 
  if (strcmp(ble.buffer, "OK") == 0) {
    if (TESTMODE) {
        TestHUD();                                  //Test the LED pattern
    }else{    
      int RPM;
      if (obd.readPID(PID_RPM, RPM))                //Read RPM and store it in the variable "RPM"
      {
        switch (HUD_Type) {                         
          case 49:                                  //Tach Mode
            HUD_1(RPM);
         break;
          case 50:                                  //Track Mode
            HUD_2(RPM);
          break;
          case 51:                                  //Drag Mode
            HUD_3(RPM);
          break;
        }
      }
    }
  }else{
    ReadData(ble.buffer);                            //Parse the incoming data string
  }
}
/*=========================================================================*/

void TestHUD()
{
  Configure_HUD();                                      //Reconfigure the HUD to the new values

  if (testRPM>649)                                      //Cycle through the RPM range, from 650-6500, at 50 RPM intervals
  {
     if (!ascent)                                       
     {
       testRPM = testRPM - 50;                          //Decrease the RPM
     }else
     {
       testRPM = testRPM + 50;                          //Increase the RPM
       if(testRPM >=REDLINE)                            //Until we reach the Redline
       {
        ascent = false;
       }
     }
  }else{
    TESTMODE = false;                                   //Stop and reset once if the testRPM counter goes below 650
    testRPM = 650;
  }
  
 switch (HUD_Type) {                                    //Operate the HUD using the test RPMvalue
    case 49:                                            //Tach Mode
      HUD_1(testRPM);
    break;
    case 50:                                            //Track Mode
      HUD_2(testRPM);
    break;
    case 51:
      HUD_3(testRPM);                                   //Drag Mode
    break;
  }
}

void Configure_HUD()
{
  HUD_Type = EEPROM.read(0);                            //Set the HUD type
  REDLINE = word(EEPROM.read(1), EEPROM.read(2));       //Set the Redline
  
  switch (HUD_Type) {
    case 49:                                            //Tachometer Mode
      RPM_interval = (REDLINE - 1000)/22;               //Set the interval between sequential LEDs
      SHIFT = word(EEPROM.read(10), EEPROM.read(11));   //Set the mode's shift point
      break;
    case 50:                                            //Track Mode
      RPM_interval = (SHIFT - (SHIFT/2))/9;            //Set the interval between sequential LEDs
      SHIFT = word(EEPROM.read(20), EEPROM.read(21));   //Set the mode's shift point
      break;
    case 51:                                            //Drag Mode
      SHIFT = word(EEPROM.read(40), EEPROM.read(41));   //Set the mode's shift point
      break;
  }
}

void ReadData(String str)
{
  int start = str.indexOf("HUD=");                      //Find the HUD Type
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 4))){                 
      byte HUD_type = str.charAt(start + 4);
      EEPROM.update(0,HUD_type);
    }
  }
  start = str.indexOf("REDLINE=");                      //Find the REDLINE (not configured in iOS app)
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 8))){
      int redline = str.substring(start + 8).toInt();
      EEPROM.update(1, highByte(redline));
      EEPROM.update(2, lowByte(redline));
    }
  }
  start = str.indexOf("H1S=");                          //Find the HUD1 Shift Point
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 4))){
      int shiftpoint = str.substring(start + 4,start + 8).toInt();      
      EEPROM.update(10, highByte(shiftpoint));
      EEPROM.update(11, lowByte(shiftpoint));
    }
  }
  start = str.indexOf("H1C1=");                         //Find the 1st HUD1 Color
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
  start = str.indexOf("H1C2=");                         //Find the 2nd HUD1 Color
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
  start = str.indexOf("H2S=");                          //Find the HUD2 Shift Point
  if(start >= 0)
  { 
    if(isDigit(str.charAt(start + 4))){
      int shiftpoint = str.substring(start + 4,start + 8).toInt();  
      EEPROM.update(20, highByte(shiftpoint));
      EEPROM.update(21, lowByte(shiftpoint));
    }
  }
  start = str.indexOf("H2C1=");                         //Find the 1st HUD2 Color
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
  start = str.indexOf("H2C2=");                         //Find the 2nd HUD2 Color
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
  start = str.indexOf("H2C3=");                         //Find the 3rd HUD2 Color
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
  start = str.indexOf("H3S=");                          //Find the HUD3 Shift Point
  if(start >= 0)
  {
    if(isDigit(str.charAt(start + 4))){
      int shiftpoint = str.substring(start + 4,start + 8).toInt();      
      EEPROM.update(40, highByte(shiftpoint));
      EEPROM.update(41, lowByte(shiftpoint));
    }
  }
  start = str.indexOf("HUD3_CLR1=");                    //Find the HUD3 Color
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
  start = str.indexOf("STARTTEST");                     //Find the command to enter Test Mode
  if(start >= 0)
  {
    TESTMODE=true;
    ascent=true;
  } 
  start = str.indexOf("CONNECTED");                     //Find the command to send Configuration Data over BLE
  if(start >= 0)
  {                         //Color 1 - Red
    ble.print("AT+BLEUARTTX=");                         //Send the HUD 1 Config
    ble.print(word(EEPROM.read(10),EEPROM.read(11)));   //Shift Point
    ble.print("-");
    ble.print(EEPROM.read(12));                         //Color 1 - Red
    ble.print("-");
    ble.print(EEPROM.read(13));                         //Color 1 - Green
    ble.print("-");
    ble.print(EEPROM.read(14));                         //Color 1 - Blue
    ble.print("-");
    ble.print(EEPROM.read(15));                         //Color 2 - Red
    ble.print("-");
    ble.print(EEPROM.read(16));                         //Color 2 - Green
    ble.print("-");
    ble.print(EEPROM.read(17));                         //Color 2 - Blue
    ble.println("-");
    ble.print("AT+BLEUARTTX=");                         //Send the HUD 2 Data
    ble.print(word(EEPROM.read(20),EEPROM.read(21)));   //Shift Point
    ble.print("-");
    ble.print(EEPROM.read(22));                         //Color 1 - Red
    ble.print("-");
    ble.print(EEPROM.read(23));                         //Color 1 - Green
    ble.print("-");
    ble.print(EEPROM.read(24));                         //Color 1 - Blue
    ble.print("-");
    ble.print(EEPROM.read(25));                         //Color 2 - Red
    ble.print("-");
    ble.print(EEPROM.read(26));                         //Color 2 - Green
    ble.print("-");
    ble.print(EEPROM.read(27));                         //Color 2 - Blue
    ble.print("-");
    ble.print(EEPROM.read(28));                         //Color 3 - Red
    ble.print("-");
    ble.print(EEPROM.read(29));                         //Color 3 - Green
    ble.print("-");
    ble.print(EEPROM.read(30));                         //Color 3 - Blue
    ble.println("-");
    ble.print("AT+BLEUARTTX=");                         //Send the HUD 3 Data
    ble.print(word(EEPROM.read(40),EEPROM.read(41)));   //Shift Point
    ble.print("-");
    ble.print(EEPROM.read(42));                         //Color 1 - Red
    ble.print("-");
    ble.print(EEPROM.read(43));                         //Color 1 - Green
    ble.print("-");
    ble.println(EEPROM.read(44));                       //Color 1 - Blue
  }
}

void HUD_1(int RPM)
{
  int underShift = floor((SHIFT-1000)/RPM_interval)+1;                  //Find the LEDs to be lit below the Shift point
  int LEDs_lit = RPM>1000 ? floor((RPM-1000)/RPM_interval)+1 : 0;       //Find the total number of LEDs to be lit
  if (LEDs_lit>=0)
  {
    strip.clear();
    for(uint8_t i = 0; i<LEDs_lit; i++)
    {  
      if(i<underShift)      //If this is an LED before the ShiftPoint, set the color to Color 1
      {
        strip.setPixelColor(i,strip.Color(EEPROM.read(12),EEPROM.read(13),EEPROM.read(14)));
      }
      else                  //This is an LED after the shift point, so set the color to Color 2
      {
        strip.setPixelColor(i,strip.Color(EEPROM.read(15),EEPROM.read(16),EEPROM.read(17)));
      }
    }
    strip.show();
  }
}

void HUD_2(int RPM)
{   
    strip.clear();
    if(RPM>=SHIFT/2+RPM_interval*9)             //Fill all LEDs with Color 3
    {
       strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)));
    }else if(RPM>=SHIFT/2+RPM_interval*8)
    {                                           //Fill all but the middle 3 LEDs with Color 3
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)),0,10);
      strip.fill(strip.Color(EEPROM.read(28),EEPROM.read(29),EEPROM.read(30)),13);
    }else if(RPM>=SHIFT/2+RPM_interval*7)
    {                                           //Fill the outside 9 LEDs on both sides with Color 2
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),0,9);
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),14);
    }else if(RPM>SHIFT/2+RPM_interval*6)
    {                                           //Fill the outside 8 LEDs on both sides with Color 2
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),0,8);
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),15);
    }else if(RPM>SHIFT/2+RPM_interval*5)
    {                                           //Fill the outside 7 LEDs on both sides with Color 2
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),0,7);
      strip.fill(strip.Color(EEPROM.read(25),EEPROM.read(26),EEPROM.read(27)),16);
    }else if(RPM>SHIFT/2+RPM_interval*4)
    {                                           //Fill the outside 6 LEDs on both sides with Color 1
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,6);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),17);
    }else if(RPM>SHIFT/2+RPM_interval*3)
    {                                           //Fill the outside 5 LEDs on both sides with Color 1
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,5);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),18);
    }else if(RPM>SHIFT/2+RPM_interval*2)
    {                                           //Fill the outside 4 LEDs on both sides with Color 1
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,4);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),19);
    }else if(RPM>SHIFT/2+RPM_interval)
    {                                           //Fill the outside 3 LEDs on both sides with Color 1
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,3);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),20);
    }else if(RPM>REDLINE/2)
    {                                           //Fill the outside 2 LEDs on both sides with Color 1
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),0,2);
      strip.fill(strip.Color(EEPROM.read(22),EEPROM.read(23),EEPROM.read(24)),21);
    }
    strip.show();
}

void HUD_3(int RPM)                         
{
    if (RPM>=SHIFT)                                      //If the RPM is at/above the Shift Point
    {                                                    //Set the entire strip to the stored color
      strip.clear();                                    
      strip.fill(strip.Color(EEPROM.read(42),EEPROM.read(43),EEPROM.read(44)));
      strip.show();
    }else{                                               //Otherwise, clear the strip
      strip.clear();        
      strip.show();
    }
}
