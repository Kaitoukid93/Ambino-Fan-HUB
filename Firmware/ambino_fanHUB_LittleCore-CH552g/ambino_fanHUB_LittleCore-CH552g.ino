//#ifndef USER_USB_RAM
//#error "This example needs to be compiled with a USER USB setting"
//#endif
//
//#include "src/userUsbCdc/USBCDC.h"
const uint8_t magic[] = {'a', 'b', 'n'};
__xdata uint8_t speedData[10];//maximum 10 fans support
enum processModes_t {Header, DataSpeed} Stage = Header;
#define MAGICSIZE  sizeof(magic)
#define HICHECK    (MAGICSIZE)
#define LOCHECK    (MAGICSIZE + 1)
#define SUM   (MAGICSIZE + 2)
#define HEADER1   (MAGICSIZE + 3) // Which output to display
#define HEADER2   (MAGICSIZE + 4)// preserved for future purpose 
#define HEADER3   (MAGICSIZE + 5)// preserved for future purpose 
uint16_t outPos;  // current byte index in the Speed array
uint8_t writePtr = 0;
char serialChar;
unsigned long t;
int pwmPin = 34;
void setup() {
  //
//  USBInit();
  Serial0_begin(9600);
  Serial0_println_s("waiting for speed");

}

void loop() {
  t = millis(); // Save current time

  // If there is new serial dataUSBSerial_available()
  if (Serial0_available()) {
    serialChar = Serial0_read();
    switch (Stage) {
      case Header:
        headerMode();
        break;
      case DataSpeed:
        dataSpeedMode();
        break;
    }

  }


}

void headerMode()
{
  static uint8_t
  headPos,
  hi, lo, sum, output;
  if (headPos < MAGICSIZE) {
    // Check if magic word matches
    if (serialChar == magic[headPos]) {

      headPos++;
    }

    else {
      headPos = 0;
    }
  }
  else {

    ///read in to string////
    ///first 6 byte param///
    switch (headPos) {
      case HICHECK:
        hi = serialChar; // hi byte LED number
        headPos++;
        break;
      case LOCHECK:
        lo = serialChar; // low byte LED number
        headPos++;
        break;
      case SUM:
        sum = serialChar; // checksum
        headPos++;
        break;
      case HEADER1:
        output = serialChar; // Which output this data is for
        headPos++;
        break;
      case HEADER2:
        // tbd
        headPos++;
        break;
      case HEADER3:
        // tbd
        Stage = DataSpeed;
        outPos = 0;
        break;
    }
  }
}
void dataSpeedMode()
{

  if (outPos < sizeof(speedData)) {
    speedData[outPos] = serialChar; // Issue next byte
    outPos++;
  }
  else
  {
     //apply speed to all fan
      analogWrite(pwmPin, speedData[0]);
    Serial0_print_s("FanSpeed Applied : ");
    Serial0_println(speedData[0]);
    Stage = Header; // Begin next header search
  
  }
}
