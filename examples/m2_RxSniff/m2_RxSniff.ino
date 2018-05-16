#include <m2_can.h>
#include <m2_io.h>

#include "macros.h"
#include "bigdec.h"
#include "setup.h"

// frame.ino
extern canFrame_t  frame;

//------------------------------------------------------------------------------ ---------------------------------------
#define LONG_CLICK  (500)

//------------------------------------------------------------------------------ ---------------------------------------
bool          logging = true;//false;
int           bus     = 0;
unsigned int  tick    = 0;  // Timer

//+============================================================================= =======================================
void  can_run (int n)
{
  bus = n;
  can_setup();          // Setup the CAN controller
  mbox_setup();         // Configure a Mailbox
  can_listen_off(bus);  // Disable listen-only mode
}  

//+============================================================================= =======================================
void  setup (void) 
{
  led_init_all(NULL);  // Configure the LEDs
  led_on(LED_RED);     // Announce we've booted
  
  btn_init_all(5);     // Configure the buttons with a 5mS debounce
  
  serial_setup();      // Check for a serial monitor
  help();              // Display help
  
  can_run(bus);        // Reset the CAN bus
}

//+============================================================================= =======================================
void check_buttons (void)
{
  unsigned int  ms;
  
  // --- SW2 ---
  if (btn_onDown(BTN_SW2)) {
    ms = millis();
    while(btn_isDown(BTN_SW2) && (millis() < ms + LONG_CLICK)) ;
    
    if (millis() < ms + LONG_CLICK) {  // Short press
      FSAYLN("\r\n+ [SW2] Restart timer") ;
      
    } else {
      FSAYLN("\r\n+ [SW2+] Change CAN bus") ;
      bus = (bus == 1) ? 0 : 1 ;
      can_run(bus);  // Reset the CAN bus
    }
    
    tick = 0;
  }
    
  // --- SW1 ---
  if (btn_onDown(BTN_SW1)) {
    ms = millis();
    while(btn_isDown(BTN_SW1) && (millis() < ms + LONG_CLICK)) ;
    
    if (millis() < ms + LONG_CLICK) {  // Short press
      FSAYLN("\r\n+ [SW1] Renegotiate Speed") ;
      can_run(bus);  // Reset the CAN bus
      
    } else {
      FSAY("\r\n+ [SW1+] Toggle logging: ") ;
      logging = !logging;
      SAYLN(logging ? F("ON") : F("OFF")) ;
    }
    
    tick = 0;
  }    
}

//+============================================================================= =======================================
void(*pFn)(int,int) = prepPkt;

void  loop (void) 
{
  static int    rx0   = 0;
  static int    rx1   = 0;
  static int    logs  = 0;
  static int    loge  = 0;
  static int    lpCnt = 0;
  unsigned int  ms    = 0;  // millis() read

  // This loop avoids the need to jump in-and-out of main() every iteration
  for (lpCnt = 0; true; lpCnt++) {

    // Catch Packets
    if (MSR_MRDY(mbox_sr_get(bus,0))) {
      rx0++;
      if (logging) {
        intr_isr(bus);
        pFn(bus,0);
        if (frame.ext)  loge++ ;
        else            logs++ ;
      } else {
        mbox_rx(bus,0);
      }
    }

// See notes against mbox_setup()    
//    // Mailbox[bus,1] is 2.0b/Ext
//    if (MSR_MRDY(mbox_sr_get(bus,1))) {
//      rx1++;
//      if (logging) {
//        intr_isr(bus);
//        pFn(bus,1);
//      } else {
//        mbox_rx(bus,1);
//      }
//    }

    check_buttons();
    
    ms = millis();
    if (ms >= tick) {
      FSAY("@ ");
      BIGDEC((tick = millis() + 10000));
      FSAY(" : rx.0=");       BIGDEC(rx0);  
//      FSAY(" : rx.1=");       BIGDEC(rx1);  
      if (logging) {
        FSAY(" : log.std=");  BIGDEC(logs);  
        FSAY(" : log.ext=");  BIGDEC(loge);  
      }
      FSAY("   lpCnt=");      BIGDEC(lpCnt - rx0 - rx1);
      FSAYLN("");
      if (serialEventRun) serialEventRun();
      //if (logging)  delay(3000) ;
      rx0 = rx1 = logs = loge = lpCnt = 0;  
    }
    
  } // for (lpCnt
}

