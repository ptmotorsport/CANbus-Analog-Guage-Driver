// PT Motorsport AU - CAN shift light project - (c) 2023 Peter Nowell
// Released under the GPLv3 license
// Arduino Neopixel project to illuminate LEDs based on wheel slip percentage from rear to front indicating to the driver 
// the front wheels are locking up.

#include <Adafruit_NeoPixel.h> // Library for NeoPixel Communication
#include <SPI.h>               // Library for using SPI Communication 
#include <mcp2515.h>           // Library for using CAN Communication

// How many NeoPixels are attached to the Arduino? If you add more LEDs, add the colours below too
#define NUMPIXELS 8 

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 4 

int ON_LEDS = 0; //Number of LEDS to illuminate
int CLT = 0;     //Coolant Temp from CAN
int OILP = 0;    //Oil Pressure from CAN
int OILPG = 6;   //Pin that Oil pressure gauge sensor wire is connected to (PWM!)
int OILPPWM = 254; //PWM Pulse width to send to gauge (zero position)
int CLTG = 5;    //Pin that Coolant Temp gauge sensor wire is connected to (PWM!)
int CLTPWM = 32; //PWM Pulse width to send to gauge (zero position)

struct can_frame canMsg1;
struct can_frame canMsg2;


MCP2515 mcp2515(10); // SPI CS Pin 10 - MCP2515 CS Pin

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
{
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.show(); // Initialize all pixels to 'off'
}

SPI.begin();

Serial.begin(9600); //comment out to disable serial

  mcp2515.reset();      
  mcp2515.setBitrate(CAN_1000KBPS,MCP_8MHZ); // Sets CAN at speed and clock freq | Default = (CAN_500KBPS,MCP_8MHZ)
  mcp2515.setNormalMode();                   // Sets CAN at normal mode

  pixels.clear();
  pixels.fill(pixels.Color(10, 0, 0)); // Faint red
  pixels.show();



  pinMode(OILPG, OUTPUT);
  pinMode(CLTG, OUTPUT);
}

void loop() {
 // pixels.clear();
       analogWrite(CLTG, CLTPWM);  
       analogWrite(OILPG, OILPPWM); 
       
// GET THE CLT FROM CAN
 if (mcp2515.readMessage(&canMsg2) == MCP2515::ERROR_OK) {
    if(canMsg2.can_id == 0x3E0) { // CAN ID
    
      int y1 = canMsg2.data[0]; //byte 0
      int y2 = canMsg2.data[1]; //byte 1

      CLT = (y2 + (y1 << 8)); //put them together
      CLT = 0.1 * CLT - 273;
      CLTPWM = map(CLT,40,120,32,194);
       Serial.println("---------");
       Serial.println("CLT");   
       Serial.println(CLT);

    }
 }
 if (CLT > 105) {
  pixels.fill(pixels.Color(0, 0, 50)); // blue
  pixels.show();
 }
  else{

 // GET OIL PRESSURE FROM CAN
if (mcp2515.readMessage(&canMsg1) == MCP2515::ERROR_OK) {

    //CAN Message 1, dettermine the CAN ID and Bytes of the message
    if(canMsg1.can_id == 0x361) { // CAN ID
    
      int x1 = canMsg1.data[2]; //byte 2
      int x2 = canMsg1.data[3]; //byte 3


      OILP = (x2 + (x1 << 8)); //put them together
      OILP = (((0.1 * OILP) - 101.3)*0.145);
      OILPPWM = map(OILP,0,140,255,194);          
      // Print to serial (uncomment for debugging)
       Serial.println("---------");
       Serial.println("OIL PRESSURE");   
       Serial.println(OILP); 
                   



  
    }
   }
}
}
