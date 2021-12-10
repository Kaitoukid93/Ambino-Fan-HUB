#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "src/userUsbCdc/USBCDC.h"
#include <WS2812.h>


#define NUM_LEDS_PER_FAN 32 /// max led number supported
#define COLOR_PER_LEDS 3 /// 3 chanel per led
#define NUM_BYTES_PER_FAN (NUM_LEDS_PER_FAN*COLOR_PER_LEDS)  /// number of bytes 
#define MAGICSIZE  sizeof(magic)
#define HICHECK    (MAGICSIZE)
#define LOCHECK    (MAGICSIZE + 1)
#define SUM   (MAGICSIZE + 2)
#define HEADER1   (MAGICSIZE + 3) // Which output to display
#define HEADER2   (MAGICSIZE + 4)// preserved for future purpose 
#define HEADER3   (MAGICSIZE + 5)// preserved for future purpose 
#define DEBUG_LED 34
enum processModes_t {Header, Data} Stage = Header;
enum app {Ambino, Adalight} App = Ambino;
const uint8_t magic[] = {'a', 'b', 'n'};
const uint8_t serial[] = {'d', 'i', 'd'};
const uint8_t firmware[] = {'f', 'v', 'r'};
const uint8_t magic2[] = {
  'A', 'd', 'a'
};
__xdata uint8_t ledData[NUM_BYTES_PER_FAN];//maximum leds per fan support
uint8_t currentOutput;
__xdata uint8_t * ledsRaw = (uint8_t *)ledData;
uint16_t outPos;  // current byte index in the LED array
uint32_t bytesRemaining;  // count of bytes yet received, set by checksum
char serialChar;
unsigned long t, lastByteTime, lastAckTime;  // millisecond timestamps
uint16_t
SerialTimeout  = 0; // set to 0 if you want led stay what it was at the las second after disconnect from application


int ledState = 0;
void headerMode();
void dataMode();
void timeouts();
#ifdef DEBUG_LED
#define ON  1
#define OFF 0

#define D_LED(x) do {digitalWrite(DEBUG_LED, x);} while(0)
#else
#define D_LED(x)
#endif


void setup()
{
  USBInit();
  pinMode(15, OUTPUT); //Possible to use other pins.
  pinMode(14, OUTPUT); //Possible to use other pins.
  pinMode(32, OUTPUT); //Possible to use other pins.
  pinMode(16, OUTPUT); //Possible to use other pins.
  pinMode(17, OUTPUT); //Possible to use other pins.
  pinMode(31, OUTPUT); //Possible to use other pins.
  pinMode(30, OUTPUT); //Possible to use other pins.
  pinMode(11, OUTPUT); //Possible to use other pins.
  pinMode(10, OUTPUT); //Possible to use other pins.
  pinMode(33, OUTPUT); //Possible to use other pins.
#ifdef DEBUG_LED
  pinMode(DEBUG_LED, OUTPUT);
  digitalWrite(DEBUG_LED, LOW);
#endif
  lastByteTime = lastAckTime = millis(); // Set initial counters
  USBSerial_print_s("Abn\n");
  USBSerial_flush();
  delay(100);
  //  USBSerial_print_s("Ada\n"); // Send ACK string to host
  USBSerial_flush();
}

void loop()
{
  t = millis(); // Save current time

  // If there is new serial dataUSBSerial_available()
  if (USBSerial_available()) {
    serialChar = USBSerial_read();
    lastByteTime = lastAckTime = t; // Reset timeout counters

    switch (Stage) {
      case Header:
        headerMode();
        break;
      case Data:
        dataMode();
        break;
    }

  }
  else {
    // No new data
    timeouts();
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
      if (headPos == 2)
      {
        App = Ambino;
      }
      headPos++;
    }
    else if (serialChar == magic2[headPos]) {

      if (headPos == 2)
      {
        App = Adalight;
      }
      headPos++;
    }
    else if (serialChar == serial[headPos]) //Application asking for Serial Number of ther device
    {
      if (headPos == 2)
      {
        USBSerial_print_s("ID:");
        USBSerial_print_ub(*(__code uint8_t*)(0x3FFC), HEX);
        // USBSerial_print_c(' ');
        USBSerial_print_ub(*(__code uint8_t*)(0x3FFD), HEX);
        // USBSerial_print_c(' ');
        USBSerial_print_ub(*(__code uint8_t*)(0x3FFE), HEX);
        // USBSerial_print_c(' ');
        USBSerial_print_ub(*(__code uint8_t*)(0x3FFF), HEX);
        //  USBSerial_print_c(' ');
        USBSerial_print_ub(*(__code uint8_t*)(0x3FFA), HEX);
        // USBSerial_print_c(' ');
        USBSerial_println();
      }
      headPos++;
    }

    else if (serialChar == firmware[headPos]) {
      if (headPos == 2)
      {
        USBSerial_print_s("FW: 1.0.1");
        USBSerial_println();
      }
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
        switch (App) {
          case Ambino:
            if (sum == (hi ^ lo ^ 0x55)) // sum matched
            {
              //turn on signal LED , data is valid
              D_LED(ON);
              // how many bytes left to read, add 1
              bytesRemaining = 3L * (256L * (long)hi + (long)lo + 1L);
              // reset data byte position
              outPos = 0;
              // clear all LEDs
              memset(ledData, 0, NUM_BYTES_PER_FAN );
              // Set current output
              currentOutput = output;
              // Proceed to data mode
              Stage = Data;

            }

            headPos = 0; // Reset header position regardless of checksum result

            break;
          case Adalight:
            // Serial0_println_s("Ada");
            if (sum == (hi ^ lo ^ 0x55)) { //sum matched
              // Checksum looks valid. Get 16-bit LED count, add 1
              // (# LEDs is always > 0) and multiply by 3 for R,G,B.
              D_LED(ON);
              bytesRemaining = 3L * (256L * (long)hi + (long)lo + 1L);
              outPos = 0;
              memset(ledData, 0, NUM_BYTES_PER_FAN );
              Stage = Data; // Proceed to latch wait mode
            }
            headPos = 0; // Reset header position regardless of checksum result
            break;
        }
        break;
    }
  }
}

void dataMode() {

  if (outPos < sizeof(ledData)) {
    ledsRaw[outPos++] = serialChar; // Issue next byte
  }

  bytesRemaining--;
  if (bytesRemaining == 0) {
    // End of data -- Show LED:
    Stage = Header; // Begin next header search
    switch (currentOutput)
    {
      case 0:
        neopixel_show_P1_5(ledData, outPos - 3); // FastLED.show();
        break;
      case 1:
        neopixel_show_P1_4(ledData, outPos - 3); // FastLED.show();
        break;
      case 2:
        neopixel_show_P3_2(ledData, outPos - 3); // FastLED.show();
        break;
      case 3:
        neopixel_show_P1_6(ledData, outPos - 3); // FastLED.show();
        break;
      case 4:
        neopixel_show_P1_7(ledData, outPos - 3); // FastLED.show();
        break;
      case 5:
        neopixel_show_P3_1(ledData, outPos - 3); // FastLED.show();
        break;
      case 6:
        neopixel_show_P3_0(ledData, outPos - 3); // FastLED.show();
        break;
      case 7:
        neopixel_show_P1_1(ledData, outPos - 3); // FastLED.show();
        break;
      case 8:
        neopixel_show_P1_0(ledData, outPos - 3); // FastLED.show();
        break;
      case 9:
        neopixel_show_P3_3(ledData, outPos - 3); // FastLED.show();
        break;
    }
    // turn off signal LEDs, Frame finished
    D_LED(OFF);
    // Flush serial to clear ring buffer
    USBSerial_flush();

  }
}


void timeouts() {
  // No data received. If this persists, send an ACK packet
  // to host once every second to alert it to our presence.

  if ((t - lastAckTime) >= 1000) {
    switch (App) {
      case Ambino:
        USBSerial_println_s("Abn\n"); // Send ACK string to host
        USBSerial_flush();
        if (ledState == 0) {
          ledState = 1;
          D_LED(ON);
        } else {
          ledState = 0;
          D_LED(OFF);
        }
        break;
      case Adalight:
        USBSerial_print_s("Ada\n"); // Send ACK string to host
        USBSerial_flush();
        if (ledState == 0) {
          ledState = 1;
          D_LED(ON);
        } else {
          ledState = 0;
          D_LED(OFF);
        }
        break;
    }

    lastAckTime = t; // Reset counter

    // If no data received for an extended time, turn off all LEDs.
    if (SerialTimeout != 0 && (t - lastByteTime) >= (uint32_t) SerialTimeout * 1000) {
      //      for (uint8_t i = 0; i < NUM_LEDS; i++) {
      //        //        set_pixel_for_GRB_LED(ledData, i, 0, 0, 0); //Choose the color order depending on the LED you use.
      //      }
      //      neopixel_show_P3_2(ledData, NUM_BYTES); //Possible to use other pins.
      Stage = Header;
      lastByteTime = t; // Reset counter
    }
  }
}
