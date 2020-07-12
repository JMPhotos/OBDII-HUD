# OBDII-HUD
This project was created to provide a Shelby GT350-styled shift light to the standard S550 Mustang. Unlike the Shelby HUD, which operates over a CANBUS signal generated by the instrument cluster, this HUD operates by an Arduino that reads OBDII data and illuminates its own LED strip. As a result, this HUD is capable of multiple configurable colors, compared to the Shelby's single color.

[![YouTube Video](https://bn1301files.storage.live.com/y4mc-Qmy1Wy7CH_nki1sQPXlUrQyFIpGN45elIMjiqWiqnhzH5xPaIOnbj_rqs-YyVlDRxONY2hskADcRBpKHx3kI2mJzZXOBgym8kHvxZfXqWbLxaQvcQSg7p6pnP7OKIzwGyiiOqPuidRYqy9FZapIOiJ00SW1W0aa_LU_Zmlrpm6DtRr4yn2uLOn0IaKoCAjh9IjLacihOpi1r86y6wVUA/screenshot.png?psid=1&width=1748&height=986)](https://www.youtube.com/watch?v=3OxBmfJmqig)

------

### Parts Needed

- Arduino UNO (or similar)
- Adafruit BLE Shield
- Freematics OBDII UART v2.1
- Neopixel strip 144/m
- Defroster Grille - HR3Z-63044E82-AA
- HUD Module - FR3Z-19G468-A



### Folder Descriptions

1. Arduino - Contains the source code for the Arduino microcontroller. Includes hard-coded versions that don't require the iOS app 
2. iOS - ~~Contains the source code for the iOS app.~~ **TO BE ADDED** Since I don't want to spend the $100 per year to publish the application on Apple's App Store, this app will only be distributed via source code. You'll need a Mac computer with XCode in order to compile it and run it on your device. 