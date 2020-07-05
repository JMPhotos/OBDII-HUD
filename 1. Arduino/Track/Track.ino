/*=========================================================================
    TACH MODE - This program is designed to illuminate a series of 23 NeoPixel
               LEDs based on engine RPM. While designed to work with the 2015
               and newer Mustangs, this could potentially work with any
               vehicle using a standard OBD-II port.
    HARDWARE: Arduino UNO R3
              Freematics OBD-UART V2.1
              23 LED NeoPixel Strand (144 LED/m)
    DEVELOPER: Jacob Mast
               jacobmast.net
    -----------------------------------------------------------------------*/
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <OBD2UART.h>
/*=========================================================================*/

/*=========================================================================
    APPLICATION SETTINGS
      REDLINE         Maximum Engine RPM
      SHIFT           Selected Shift Point RPM
      RPM_interval    Number of revs between illuminating two LEDs    
    -----------------------------------------------------------------------*/
int REDLINE = 6500;
int RPM_interval;
int SHIFT = 5500;
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

/* OBD-II */
  obd.begin();            // INITIALIZE OBDII module          (REQUIRED)
  while (!obd.init());    // Wait for OBD Plug configuration  (REQUIRED)

/* Read EEPROM and Configure HUD */
   RPM_interval = (SHIFT - (SHIFT/2))/10;
}
/*=========================================================================*/


/*=========================================================================
    APPLICATION MAIN LOOP
    -----------------------------------------------------------------------*/
void loop() {

  int RPM;
  if (obd.readPID(PID_RPM, RPM))                                            //Read RPM and store it in the variable "RPM"
  {
    strip.clear();
    if(RPM>=SHIFT/2+RPM_interval*9)                                         //Fill all LEDs with Red
    {
       strip.fill(strip.Color(255,0,0));
    }else if(RPM>=SHIFT/2+RPM_interval*8)
    {                                                                       //Fill all but the middle 3 LEDs with Red
      strip.fill(strip.Color(255,0,0),0,10);
      strip.fill(strip.Color(255,0,0),13);
    }else if(RPM>=SHIFT/2+RPM_interval*7)
    {                                                                       //Fill the outside 9 LEDs on both sides with Amber
      strip.fill(strip.Color(255,191,0),0,9);
      strip.fill(strip.Color(255,191,0),14);
    }else if(RPM>SHIFT/2+RPM_interval*6)
    {                                                                       //Fill the outside 8 LEDs on both sides with Amber
      strip.fill(strip.Color(255,191,0),0,8);
      strip.fill(strip.Color(255,191,0),15);
    }else if(RPM>SHIFT/2+RPM_interval*5)
    {                                                                       //Fill the outside 7 LEDs on both sides with Amber
      strip.fill(strip.Color(255,191,0),0,7);
      strip.fill(strip.Color(255,191,0),16);
    }else if(RPM>SHIFT/2+RPM_interval*4)
    {                                                                       //Fill the outside 6 LEDs on both sides with Green
      strip.fill(strip.Color(0,255,0),0,6);
      strip.fill(strip.Color(0,255,0),17);
    }else if(RPM>SHIFT/2+RPM_interval*3)
    {                                                                       //Fill the outside 5 LEDs on both sides with Green
      strip.fill(strip.Color(0,255,0),0,5);
      strip.fill(strip.Color(0,255,0),18);
    }else if(RPM>SHIFT/2+RPM_interval*2)
    {                                                                       //Fill the outside 4 LEDs on both sides with Green
      strip.fill(strip.Color(0,255,0),0,4);
      strip.fill(strip.Color(0,255,0),19);
    }else if(RPM>SHIFT/2+RPM_interval)
    {                                                                       //Fill the outside 3 LEDs on both sides with Green
      strip.fill(strip.Color(0,255,0),0,3);
      strip.fill(strip.Color(0,255,0),20);
    }else if(RPM>REDLINE/2)
    {                                                                       //Fill the outside 2 LEDs on both sides with Green
      strip.fill(strip.Color(0,255,0),0,2);
      strip.fill(strip.Color(0,255,0),21);
    }
    strip.show();
  }
}
/*=========================================================================*/
