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
  RPM_interval = (REDLINE - 1000)/22;
}
/*=========================================================================*/


/*=========================================================================
    APPLICATION MAIN LOOP
    -----------------------------------------------------------------------*/
void loop() {

  int RPM;
  if (obd.readPID(PID_RPM, RPM))                                            //Read RPM and store it in the variable "RPM"
  {
    int underShift = floor((SHIFT-1000)/RPM_interval)+1;                    //Determine the number of LEDs lit below the shift point
    int LEDs_lit = RPM>1000 ? floor((RPM-1000)/RPM_interval)+1 : 0;         //Determine the total number of LEDs lit
    if (LEDs_lit>=0)
    {
      strip.clear();
      for(uint8_t i = 0; i<LEDs_lit; i++)
      {  
        if(i<underShift)                                                    //Set LEDs below the shift point to Amber
        {
          strip.setPixelColor(i,strip.Color(255,191,0));                    //R=255, G=191, B=0 
        }
        else                                                                //Set LEDs abaove the shiftpoint to Red
        {
          strip.setPixelColor(i,strip.Color(255,0,0));                      //R=255, G=0, B=0
        }
      }
      strip.show();
    }
  }
}
/*=========================================================================*/
