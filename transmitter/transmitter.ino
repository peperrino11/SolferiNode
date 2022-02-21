//including all libraries
#include <XBee.h> //xbee library
#include <Adafruit_VC0706.h> //camera library
#include <SPI.h> //SPI for SD card
#include <SD.h> //SD card library
#include <LiquidCrystal.h> //LCD library
//-----------------------------------------------------------------------------------
//things for XBEE
XBee xbee = XBee();
XBeeAddress64 Broadcast = XBeeAddress64(0x0013A200, 0x41E5947F); //address of receiving Xbee... make sure to have exact address and not broadcast for optimal speed
#define LED 13  // LED will tell us when data is being sent
//----------------------------------------------------------------------------------
//things for VC0706
#if defined(__AVR__) || defined(ESP8266)
#include <SoftwareSerial.h>
SoftwareSerial cameraconnection(64, 12); //TX=64 and RX=12
#else
//#define cameraconnection Serial1
#endif
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);
//---------------------------------------------------------------------------------
//things for SD Card
#define chipSelect 53
File imgFile;
char filename[14];
//---------------------------------------------------------------------------------
//things for LCD
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void setup() {
  //opening up serial port
  Serial.begin(9600);
  //--------------------------------------------------------------------------------
  //setting up SD
#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if (chipSelect != 53) pinMode(53, OUTPUT); // SS on Mega
#else
  if (chipSelect != 10) pinMode(10, OUTPUT); // SS on Uno, etc.
#endif
#endif
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
  }
  //----------------------------------------------------------------------------------
  //setting up CAM, trying to locate the camera
  if (cam.begin(38400)) { //baud rate determined by manufacturer specs
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("-----------------");
    Serial.print(reply);
    Serial.println("-----------------");
  }
  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120
  // Remember that bigger pictures take longer to transmit!
  //cam.setImageSize(VC0706_640x480);        // biggest
  cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small
  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");
  //  Motion detection system can alert you when the camera 'sees' motion!
  cam.setMotionDetect(true);           // turn it on
  //cam.setMotionDetect(false);        // turn it off   (default)
  // You can also verify whether motion detection is active!
  Serial.print("Motion detection is ");
  if (cam.getMotionDetect())
    Serial.println("ON");
  else
    Serial.println("OFF");
  //---------------------------------------------------------------------------------
  //setting up XBEE
  Serial3.begin(9600); //and the software serial port
  xbee.setSerial(Serial3); //xbee in software serial3
  Serial.println("XBEE Initialization all done!");
  pinMode(LED, OUTPUT); // Declare the LED as an output
  //---------------------------------------------------------------------------------
  //setting up LCD
  lcd.begin(16, 2);
  // Print a message to the LCD.
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void loop() {

  digitalWrite(LED,LOW); //turn LED off when data is not being sent
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SolferiNode MK1");
  lcd.setCursor(0,1);
  lcd.print("Waiting...");

 if (cam.motionDetected()) { 
   Serial.println("Motion!");   
   cam.setMotionDetect(false);
   lcd.clear();
   delay(1000);
   
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
    //lcd.print(empty);
    //lcd.setCursor(0,0);
    //lcd.print("Picture Taken");

  //semaforo 1... not currently being used in the receiving end
  ZBTxRequest opened = ZBTxRequest(Broadcast, "OPEN", sizeof("OPEN")-1);
  xbee.send(opened);

  //creating file names with increasing index (up to 1000... will have to make more xD)
  strcpy(filename, "IMAGE000.JPG");
  for (int i = 0; i < 1000; i++) {
    filename[5] = '0' + i/100;
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;

    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  File imgFile = SD.open(filename, FILE_WRITE);

  uint16_t jpglength = cam.frameLength();
  lcd.setCursor(0,0);
  //lcd.print(empty);
  lcd.print(jpglength);
  lcd.setCursor(11,0);
  lcd.print("bytes");
  uint16_t jpglen = cam.frameLength();
  Serial.print(jpglen);
  Serial.println(" byte image");
 
  Serial.print("Writing image to "); Serial.print(filename);
  
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min((int)64, jpglen); // 64 is the max that will work with this setup
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    ZBTxRequest zbtx = ZBTxRequest(Broadcast, buffer, bytesToRead);
    xbee.send(zbtx);
    delay(10);
    jpglen -= bytesToRead;
    lcd.setCursor(0,1);
    lcd.print(jpglen);
    lcd.setCursor(6,1);
    lcd.print("bytes left");
    digitalWrite(LED,HIGH);
  }
  //semaforo2
  ZBTxRequest closed = ZBTxRequest(Broadcast, "CLOSED", sizeof("CLOSE")-1);
  xbee.send(closed);
  Serial.println("...Done!");
  digitalWrite(LED,LOW);  
  cam.resumeVideo();
  cam.setMotionDetect(true);
 }
}
