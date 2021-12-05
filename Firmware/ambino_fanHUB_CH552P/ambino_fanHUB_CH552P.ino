#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#endif

#include "src/userUsbCdc/USBCDC.h"
#include <WS2812.h>


#define NUM_LEDS_PER_FAN 16  /// max led number supported
#define COLOR_PER_LEDS 3 /// 3 chanel per led
#define NUM_BYTES_PER_FAN (NUM_LEDS_PER_FAN*COLOR_PER_LEDS)  /// number of bytes 
#define MAGICSIZE  sizeof(magic)
#define HICHECK    (MAGICSIZE)
#define LOCHECK    (MAGICSIZE + 1)
#define MODE   (MAGICSIZE + 2)
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
uint16_t outPos;  //current byte index in the LEDs array
uint8_t outPos1; // current byte index in the LED1 array
uint8_t outPos2;  // current byte index in the LED2 array
uint8_t outPos3;  // current byte index in the LED3 array
uint8_t outPos4;  // current byte index in the LED4 array
uint8_t outPos5;  // current byte index in the LED5 array
uint8_t outPos6;  // current byte index in the LED6 array
uint8_t outPos7;  // current byte index in the LED7 array
uint8_t outPos8;  // current byte index in the LED8 array
uint8_t outPos9;  // current byte index in the LED9 array
uint8_t outPos10;  // current byte index in the LED10 array
//__xdata uint8_t * ledsRaw = (uint8_t *)ledData;
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

  // Serial0_begin(9600);
  ////test sequence///
  //  for (uint8_t i = 0; i < NUM_LEDS_PER_FAN; i++) {
  //    set_pixel_for_GRB_LED(ledData_Fan1, i, 100, 0, 0); //Choose the color order depending on the LED you use.
  //    neopixel_show_P1_7(ledData_Fan1, NUM_BYTES_PER_FAN); //Possible to use other pins.
  //    delay(5);
  //  }
  //  for (uint8_t i = 0; i < NUM_LEDS_PER_FAN; i++) {
  //    set_pixel_for_GRB_LED(ledData_Fan1, i, 0, 100, 0);
  //    neopixel_show_P1_7(ledData_Fan1, NUM_BYTES_PER_FAN);
  //    delay(5);
  //  }
  //  for (uint8_t i = 0; i < NUM_LEDS_PER_FAN; i++) {
  //    set_pixel_for_GRB_LED(ledData_Fan1, i, 0, 0, 100);
  //    neopixel_show_P1_7(ledData_Fan1, NUM_BYTES_PER_FAN);
  //    delay(5);
  //  }
  //  for (uint8_t i = 0; i < NUM_LEDS_PER_FAN; i++) {
  //    set_pixel_for_GRB_LED(ledData_Fan1, i, 0, 0, 0);
  //    neopixel_show_P1_7(ledData_Fan1, NUM_BYTES_PER_FAN);
  //    delay(5);
  //  }

  ///end///
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
  hi, lo, mod;
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
    ///first 3 byte param///
    switch (headPos) {
      case HICHECK:
        hi = serialChar;
        headPos++;
        break;
      case LOCHECK:
        lo = serialChar;
        headPos++;
        break;
      case MODE:
        mod = serialChar;
        switch (App) {
          case Ambino:
            if (mod == (hi ^ lo ^ 0x55))
            {
              D_LED(ON);
              bytesRemaining = 3L * (256L * (long)hi + (long)lo + 1L);
              // Serial0_println_u(bytesRemaining);
              outPos = 0;
              //              outPos1 = 0;
              //              outPos2 = 0;
              //              outPos3 = 0;
              //              outPos4 = 0;
              //              outPos5 = 0;
              //              outPos6 = 0;
              //              outPos7 = 0;
              //              outPos8 = 0;
              //              outPos9 = 0;
              //              outPos10 = 0;
              memset(ledData, 0, NUM_BYTES_PER_FAN );
              //              memset(ledData_Fan2, 0, sizeof(ledData_Fan2));
              //              memset(ledData_Fan3, 0, sizeof(ledData_Fan3));
              //              memset(ledData_Fan4, 0, sizeof(ledData_Fan4));
              //              memset(ledData_Fan5, 0, sizeof(ledData_Fan5));
              //              memset(ledData_Fan6, 0, sizeof(ledData_Fan6));
              //              memset(ledData_Fan7, 0, sizeof(ledData_Fan7));
              //              memset(ledData_Fan8, 0, sizeof(ledData_Fan8));
              //              memset(ledData_Fan9, 0, sizeof(ledData_Fan9));
              //              memset(ledData_Fan10, 0, sizeof(ledData_Fan10));
              Stage = Data; // Proceed to latch wait mode
            }

            //            else if(mod==0) ///check status mod
            //            {
            //              USBSerial_write('a');
            //            }
            headPos = 0; // Reset header position regardless of checksum result

            break;
          case Adalight:
            // Serial0_println_s("Ada");
            if (mod == (hi ^ lo ^ 0x55)) { ///normal ligting mode
              // Checksum looks valid. Get 16-bit LED count, add 1
              // (# LEDs is always > 0) and multiply by 3 for R,G,B.
              D_LED(ON);
              bytesRemaining = 3L * (256L * (long)hi + (long)lo + 1L);
              // Serial0_println_u(bytesRemaining);
              outPos = 0;
//              outPos1 = 0;
//              outPos2 = 0;
//              outPos3 = 0;
//              outPos4 = 0;
//              outPos5 = 0;
//              outPos6 = 0;
//              outPos7 = 0;
//              outPos8 = 0;
//              outPos9 = 0;
//              outPos10 = 0;
              // memset(leds, 0, Num_Leds * sizeof(struct CGRB));
              memset(ledData, 0, NUM_BYTES_PER_FAN );
              //              memset(ledData_Fan2, 0, sizeof(ledData_Fan2));
              //              memset(ledData_Fan3, 0, sizeof(ledData_Fan3));
              //              memset(ledData_Fan4, 0, sizeof(ledData_Fan4));
              //              memset(ledData_Fan5, 0, sizeof(ledData_Fan5));
              //              memset(ledData_Fan6, 0, sizeof(ledData_Fan6));
              //              memset(ledData_Fan7, 0, sizeof(ledData_Fan7));
              //              memset(ledData_Fan8, 0, sizeof(ledData_Fan8));
              //              memset(ledData_Fan9, 0, sizeof(ledData_Fan9));
              //              memset(ledData_Fan10, 0, sizeof(ledData_Fan10));
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

  if (outPos < 48) {
    ledData[outPos] = serialChar; // Issue next byte
    outPos++;
    // Serial0_println_u(outPos);
  }
  //  else if (outPos >= 48 && outPos < 96) {
  //    ledData_Fan2[outPos2] = serialChar; // Issue next byte
  //    outPos2++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 96 && outPos < 144) {
  //    ledData_Fan3[outPos3] = serialChar; // Issue next byte
  //    outPos3++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 144 && outPos < 192) {
  //    ledData_Fan4[outPos4] = serialChar; // Issue next byte
  //    outPos4++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 192 && outPos < 240 ) {
  //    ledData_Fan5[outPos5] = serialChar; // Issue next byte
  //    outPos5++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 240 && outPos < 288) {
  //    ledData_Fan6[outPos6] = serialChar; // Issue next byte
  //    outPos6++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 288 && outPos < 336) {
  //    ledData_Fan7[outPos7] = serialChar; // Issue next byte
  //    outPos7++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 336 && outPos < 384) {
  //    ledData_Fan8[outPos8] = serialChar; // Issue next byte
  //    outPos8++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 384 && outPos < 432) {
  //    ledData_Fan9[outPos9] = serialChar; // Issue next byte
  //    outPos9++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }
  //  else if (outPos >= 432 && outPos < 480) {
  //    ledData_Fan10[outPos10] = serialChar; // Issue next byte
  //    outPos10++;
  //    outPos++;
  //    // Serial0_println_u(outPos);
  //  }

  bytesRemaining--;
  //Serial0_println_u(bytesRemaining);

  if (bytesRemaining == 0) {
    // End of data -- issue latch:
    Stage = Header; // Begin next header search
    neopixel_show_P1_5(ledData, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P1_4(ledData_Fan2, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P3_2(ledData_Fan3, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P1_6(ledData_Fan4, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P1_7(ledData_Fan5, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P3_1(ledData_Fan6, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P3_0(ledData_Fan7, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P1_1(ledData_Fan8, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P1_0(ledData_Fan9, NUM_BYTES_PER_FAN); // FastLED.show();
    //    neopixel_show_P3_3(ledData_Fan10, NUM_BYTES_PER_FAN); // FastLED.show();
    USBSerial_flush();
    D_LED(OFF);
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
