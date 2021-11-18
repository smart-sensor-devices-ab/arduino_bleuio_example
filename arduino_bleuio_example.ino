#include <cdcacm.h>
#include <usbhub.h>

#include "pgmstrings.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

class ACMAsyncOper : public CDCAsyncOper
{
public:
    uint8_t OnInit(ACM *pacm);
};

uint8_t ACMAsyncOper::OnInit(ACM *pacm)
{
    uint8_t rcode;
    // Set DTR = 1 RTS=1
    rcode = pacm->SetControlLineState(3);

    if (rcode)
    {
        ErrorMessage<uint8_t>(PSTR("SetControlLineState"), rcode);
        return rcode;
    }

    LINE_CODING	lc;
    lc.dwDTERate	= 115200;
    lc.bCharFormat	= 0;
    lc.bParityType	= 0;
    lc.bDataBits	= 8;

    rcode = pacm->SetLineCoding(&lc);

    if (rcode)
        ErrorMessage<uint8_t>(PSTR("SetLineCoding"), rcode);

    return rcode;
}

USB     Usb;
//USBHub     Hub(&Usb);
ACMAsyncOper  AsyncOper;
ACM           Acm(&Usb, &AsyncOper);
uint8_t start_flag;
// Starting commands, first changing the advertising device name to BleuIO Arduino Example then start advertising
#define START_CMDS "AT+ADVRESP=17:09:42:6C:65:75:49:4F:20:41:72:64:75:69:6E:6F:20:45:78:61:6D:70:6C:65\rAT+ADVSTART\r"
uint16_t recv_counter;

void setup()
{
  start_flag = 0x00;
  recv_counter = 0;
  Serial.begin( 115200 );
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  if (Usb.Init() == -1)
      Serial.println("OSCOKIRQ failed to assert");

  delay( 200 );
}

void loop()
{
    Usb.Task();

    if( Acm.isReady()) {
       uint8_t rcode;
       uint8_t rcode2;
       uint8_t rcode3;

       /* reading the keyboard */
       if(Serial.available()) {
         uint8_t data= Serial.read();
         /* sending to the BleuIO Dongle */
         rcode = Acm.SndData(1, &data);
         if (rcode)
            ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
       }//if(Serial.available()...

       //delay(50);
        if(start_flag == 0x00) 
        {
            rcode = Acm.SndData(strlen((char *)START_CMDS), (uint8_t *)START_CMDS);
            if (rcode)
            {
                ErrorMessage<uint8_t>(PSTR("SndData"), rcode);
            }

          start_flag = 0x01;
        }
        /* reading the BleuIO Dongle */
        uint8_t  buf[64];
        uint16_t rcvd = 64;
        uint8_t  buf2[64];
        uint16_t rcvd2 = 64;
        uint8_t  buf3[64];
        uint16_t rcvd3 = 64;
        uint8_t  dongle_input[3*64];
        uint16_t input_indx = 0;

        memset(dongle_input, 0, sizeof(dongle_input));
             
        rcode = Acm.RcvData(&rcvd, buf);
        delay(1);
        rcode2 = Acm.RcvData(&rcvd2, buf2);
        delay(1);
        rcode3 = Acm.RcvData(&rcvd3, buf3);
         if (rcode && rcode != hrNAK)
         {
            ErrorMessage<uint8_t>(PSTR("Ret"), rcode);
         }
            
         if (rcode2 && rcode2 != hrNAK)
         {
             ErrorMessage<uint8_t>(PSTR("Ret"), rcode2);
         }

         if (rcode3 && rcode3 != hrNAK)
         {
             ErrorMessage<uint8_t>(PSTR("Ret"), rcode3);
         }


            if( rcvd ) { //more than zero bytes received
              for(uint16_t i=0; i < rcvd; i++ ) {
                Serial.print((char)buf[i]); //printing on the screen
                dongle_input[input_indx] = buf[i];
                input_indx++;                
              }
            }
            
            if( rcvd2 ) { //more than zero bytes received
              for(uint16_t i=0; i < rcvd2; i++ ) {
                Serial.print((char)buf2[i]); //printing on the screen
                dongle_input[input_indx] = buf2[i];
                input_indx++;                
              }
            }
            
            if( rcvd3 ) { //more than zero bytes received
              for(uint16_t i=0; i < rcvd3; i++ ) {
                Serial.print((char)buf3[i]); //printing on the screen
                dongle_input[input_indx] = buf3[i];
                input_indx++;
              }
            }            
            dongle_input[input_indx] = 0x00;
            
            // Example on a way for the Arduino to react to BleuIO events
            if(strlen((char *)dongle_input) != 0)
            {
              if(strstr((char *)dongle_input, "handle_evt_gap_connected") != NULL)
              {
                Serial.print("<<CONNECTION DETECTED!>>");
              }
              else if(strstr((char *)dongle_input, "handle_evt_gap_disconnected") != NULL)
              {
                Serial.print("<<CONNECTION LOST!>>");
              }
            }
    }//if( Usb.getUsbTaskState() == USB_STATE_RUNNING..
}
