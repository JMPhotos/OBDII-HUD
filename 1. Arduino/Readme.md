# ARDUINO

These files contribute the code to making the Arduino microcontroller work. There are 4 different subfolders here, each one contains a different variant of the project, depending on your individual needs.

# FOLDER DESCRIPTIONS
**1. _DRAG_** - Hard-coded verion of the _Drag Mode_ Shift Light. When the Engine RPM reaches 5500 RPM, the entire LED strip will light up red.

**2. _HUD_** - Full-featured version of the HUD. Uses the Adafruit BLE shield to communicate with the iOS S550 HUD application for configuration. 

**3. _TACH_** - Hard-coded verion of the _Tach Mode_ Shift Light. The LEDs will illuminate sequentially from 1000 to 6500 RPM. While the RPM is lower than the shift point, LEDs will be amber colored. Above the shift point, LEDs will be red colored.

**4. _TRACK_** - Hard-coded verion of the _Track Mode_ Shift Light. LEDs will light up sequentially from the outside edges of the HUD towards the middle, starting at 1/2 of the shift point, until reaching the shift point. LEDs will change colors from green, to amber, to red.

**NOTE:** In addition to these files, you will also need to download the OBD2UART library and save it to your Arduino libraries folder. 
https://github.com/stanleyhuangyc/ArduinoOBD/tree/master/libraries/OBD2UART
