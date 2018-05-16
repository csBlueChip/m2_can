#include <m2_can.h>
#include <m2_io.h>

#include <stdint.h>

#include "macros.h"
#include "bigdec.h"
#include "setup.h"

//------------------------------------------------------------------------------ ---------------------------------------
#define LONG_CLICK  (500)    // longClick time in milliseconds (1S = 1000mS)
#define INTERVAL    (10000)  // Output interval in milliseconds (1S = 1000mS)

//------------------------------------------------------------------------------ ---------------------------------------
// Baud selection : Start with the first one - wrap when you get to the EndOfList
uint32_t   baud_list[] = {
  CAN_BAUD_125K, CAN_BAUD_250K, CAN_BAUD_500K, CAN_BAUD_1000K,
  CAN_BAUD_EOL
};
uint32_t*     pBaud = baud_list;

//------------------------------------------------------------------------------ 
int           bus   = 0;
mid_t         se    = MBOX_STD;  // Send STD/EXT packets (2.0a/2.0b)

//------------------------------------------------------------------------------ 
unsigned int  tick  = 0;         // Timer

//+============================================================================= =======================================
void  can_run (void)
{
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
  
  can_run();           // Reset the CAN bus
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
      FSAYLN("\r\n+ [SW2] Restart timer");
        
    } else {
      FSAYLN("\r\n+ [SW2+] Change CAN bus");
      bus = (bus == 1) ? 0 : 1 ;
      can_run();  // Reset the CAN bus
    }
      
    tick = 0;
  }

  // --- SW1 ---
  if (btn_onDown(BTN_SW1)) {
    ms = millis();
    while(btn_isDown(BTN_SW1) && (millis() < ms + LONG_CLICK)) ;
      
    if (millis() < ms + LONG_CLICK) {  // Short press
      FSAYLN("\r\n+ [SW1] Change Speed");
      if (*(++pBaud) == CAN_BAUD_EOL)  pBaud = baud_list ;
      can_run();  // Reset the CAN bus
       
    } else {
      FSAYLN("\r\n+ [SW1+] Change std/ext");
      se = (se == MBOX_STD) ? MBOX_EXT : MBOX_STD ;
      mbox_setup();
    }
    
    tick = 0;
  }
}

//+============================================================================= =======================================
void  loop (void) 
{
  static int    lpCnt = 0;  // Loop Counter
  static int    tx    = 0;  // Tx Counter
  unsigned int  ms    = 0;  // millis() read

  // This loop avoids the need to jump in-and-out of main() every iteration
  for (lpCnt = 0; true; lpCnt++) {
    
    // Catch Packets
    if (MSR_MRDY(mbox_sr_get(bus,0))) {
      tx++;
      // We send 0 bytes of data ...because we want the smallest packet possible
      mbox_tx(bus,0, 0);
    }

    check_buttons();
    
    // Display logs (every INTERVAL mS)
    ms = millis();
    if (ms >= tick) {
      FSAY("@ ");
      BIGDEC((tick = ms + INTERVAL));
      FSAY(" : tx=");
      BIGDEC(tx);        
      FSAY("   lpCnt=");  
      BIGDEC(lpCnt);
      FSAYLN("");      
      if (serialEventRun) serialEventRun();
      tx = lpCnt = 0;
    }

  } // for (lpCnt
}

