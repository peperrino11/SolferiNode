# SolferiNode
Including Arduino and Python scripts for the SolferiNode
--------------------------------------------------------
TRANSMITTER:

The transmitter code is written in Arduino (C/C++). This code captures an image using a VC0706 Adafruit motion sensing camera, sends the image data in the form of bytes through an Xbee S2C module and simultaneously saves the image locally in an SD card in the form of a JPEG.

RECEIVER:

The receiver code is written in Python as it will be run on a Raspberry Pi 4. This code receives bytes through an Xbee S2C, converts them to a JPEG,  saves them locally and also uploads them to a dropbox
