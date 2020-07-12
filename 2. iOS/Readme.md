# iOS

These files contribute the code to creating an iOS application that:

- Connects to and reads data from the Arduino
- Configures the Arduino to display a HUD in 1 of 3 configurations:
  - Tachometer Mode - LEDs light sequentially from 1000 to the Redline value.
  - Track Mode - LEDs light from outside to inside, from half of the shift value to the shift value.
  - Drag Mode - Entire LEDs bar illuminates if RPM is over the shift value.

------

## BLE CONFIGURATION STRINGS

The Arduino BLE module uses Bluetooth Low Energy to communicate with this custom mobile application. However, as this application was written for iOS only, and is not  released outside of source code, its use is not for everyone.

As an alternative, Adafruit has produced an application capable of communicating over the same UART channels. The iOS app is a [free download from Apple's App Store](https://itunes.apple.com/us/app/adafruit-bluefruit-le-connect/id830125974?mt=8). It requires iOS 11.3 or later and works on iPhones, iPads, and iPod Touches. The Android app is a free download from the [Google Play Store](https://play.google.com/store/apps/details?id=com.adafruit.bluefruit.le.connect). It requires Android 4.4 or later.

Instead of a graphical interface, configuration changes will need to be entered as commands.

#### Configuring Tach Mode

```
HUD=1H1S=5500H1C1=255191000H1C2=255000000
```

- H1S - Sets the shift point 

- H1C1 - Sets the color of the LEDs below the shift point. 9 digit RGB format

- H1C2 - Sets the color of the LEDs above the shift point. 9 digit RGB format

  

#### Configuring Track Mode

```
HUD=2H2S=5500H2C1=255191000H2C2=255000000H2C3=000255000
```

- H2S - Sets the shift point 

- H2C1 - Sets the color of the first level LEDs. 9 digit RGB format

- H2C2 - Sets the color of the second level LEDs. 9 digit RGB format

- H2C3 - Sets the color of the third level LEDs. 9 digit RGB format

  

#### Configuring Drag Mode

```
HUD=3H3S=5500H3C1=255191000
```

- H3S - Sets the shift point 

- H3C1 - Sets the color of the LEDs. 9 digit RGB format

  


#### Adjusting the Redline

```
REDLINE=6500
```

- REDLINE - Sets the maximum RPM for the HUD. 

  **NOTE:** Needs to be set once for the Tach mode to operate correctly.

  

#### Entering TEST Mode

```
STARTTEST
```

- STARTTEST - Enters Test Mode. 

  **NOTE:** Use after a configuration change to commit changes to the Arduino and perform a display test of the HUD

------

**NOTE:** All colors are in Red Green Blue format and take values between 0 and 255. You can use an online color picker to determine the rgb values for any color you want. https://rgbcolorcode.com/
