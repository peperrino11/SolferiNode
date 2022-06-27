# SolferiNode
Including Arduino and Python scripts for the SolferiNode
--------------------------------------------------------
TRANSMITTER:

The transmitter code is written in Arduino (C/C++). This code captures an image using a VC0706 Adafruit motion sensing camera, sends the image data in the form of bytes through an Xbee S2C module and simultaneously saves the image locally in an SD card in the form of a JPEG.

![image](https://user-images.githubusercontent.com/87677161/175926988-22bcb213-c99c-4e1f-9e83-86d34e5793ae.png)

RECEIVER:

The receiver code is written in Python as it will be run on a Raspberry Pi 4. This code receives bytes through an Xbee S2C, converts them to a JPEG,  saves them locally and also uploads them to a dropbox

![image](https://user-images.githubusercontent.com/87677161/175927296-e879b648-fd00-423c-aa63-854adb30d9eb.png)
