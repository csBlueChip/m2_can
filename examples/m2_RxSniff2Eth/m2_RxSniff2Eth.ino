#include <m2_io.h>
#include <m2_can.h>

#include <UIPEthernet.h>
#include <utility/logging.h>
#include <utility/uip.h>
#include <utility/uip_arp.h>

#include "bigdec.h"
#include "macros.h"
#include "setup.h"
#include "uip_eth.h"

//------------------------------------------------------------------------------ ---------------------------------------
#define INTERVAL     (1000)  // Logging period in milliseconds (1S = 1000mS)
unsigned int  tick = 0;      // Logging timer

//------------------------------------------------------------------------------ ---------------------------------------
#define LONG_CLICK   (500)   // longClick time in milliseconds (1S = 1000mS)
bool    logging    = false;  // Logging control    [SW2]
bool    listenOnly = false;  // ListenOnly control [SW2+]
int     bus        = 0;      // Bus number         [SW1+]

//+============================================================================= =======================================
void  heartbeat (m2led_t led)
{
  // 'timer' is the list of times for which the LED will be on/off
  //         assuming the LED is OFF before we call heartbeat() for the first time
  //           the list starts in the ON condition
  // 'pi'    is a pointer to the period in the timer list which is currently active
  // 'ms'    is the last timer reading we made
  int                  timer[] = {/*on*/100,/*off*/160,/*on*/100,/*off*/1400, 0};
  static int*          pi      = NULL;
  static unsigned int  ms      = millis();

  // This is only ever executed the FIRST time you call heartbeat
  if (!pi) {
    led_on(led);
    pi = timer;
  }
  
  // When the current timer expires,  toggle the LED and move on to the next entry
  // ...If you find a timer of value 0, wrap back round to the start of the list
  if (millis() >= ms + *pi) {
    led_toggle(led);
    if (!*(++pi))  pi = timer ;
    ms = millis();
  }
}

//+============================================================================= =======================================
void check_buttons (void)
{
  static  unsigned int  ms1;
  static  unsigned int  ms2;

  switch (btn_get(BTN_SW1)) {
    case ONDOWN :  
      ms1 = millis() + LONG_CLICK;  // Start click timer
      // Fallthrough : ISDOWN
    case ISDOWN :
      break;
    
    case ONUP :
      if (millis() < ms1) {  // Short click
        FSAYLN("\r\n+ [SW1] Renegotiate Speed") ;
        can_run(bus);  // Reset the CAN bus
        
      } else {  // Long click
        FSAYLN("\r\n+ [SW1+] Change CAN bus") ;
        bus = (bus == 1) ? 0 : 1 ;
        can_run(bus);  // Reset the CAN bus
      }
      tick = 0;  // restart timer
      // Fallthrough : ISUP
    case ISUP :  
      // This section will commonly be empty
      break;
        
    default :  break ;  // Good coding practice
  }

  switch (btn_get(BTN_SW2)) {
    case ONDOWN :  
      ms2 = millis() + LONG_CLICK;  // Start click timer
      // Fallthrough : ISDOWN
    case ISDOWN :
      break;
    
    case ONUP :
      if (millis() < ms2) {  // Short click
        FSAY("\r\n+ [SW2] Toggle logging: ") ;
        SAYLN((logging = !logging) ? F("ON") : F("OFF"));
        
      } else {  // Long click
        FSAY("\r\n+ [SW2+] Toggle Listen-Only: ") ;
        if ( (listenOnly = !listenOnly) )  {  FSAYLN("ON");   can_listen_on(bus);   }  // Listen-only mode
        else                               {  FSAYLN("OFF");  can_listen_off(bus);  }  // Ack received packets
      }
      tick = 0;  // restart timer
      // Fallthrough : ISUP
    case ISUP :  
      // This section will commonly be empty
      break;
        
    default :  break ;  // Good coding practice
  }

  return;
}

//+============================================================================= =======================================
void  can_run (int n)
{
  bus = n;                                 // Set the bus
  if (!can_setup())  return;               // Setup the CAN controller
  mbox_setup();                            // Configure Mailboxes
  if (!listenOnly)  can_listen_off(bus) ;  // Disable listen-only mode
}  

//+============================================================================= =======================================
void setup (void)
{
  led_init_all(NULL);  // Configure the LEDs
  led_on(LED_RED);     // Announce we've booted
  btn_init_all(3);     // Configure the buttons with a 3mS debounce
  serial_setup();      // Check for a serial monitor
  help();              // Display help
  eth_setup();         // Configure the Ethernet device
  can_run(bus);        // Reset the CAN bus

  FSAY("# Logging is : ") ;
  SAYLN(logging ? F("ON") : F("OFF"));
  
  FSAY("# Listen-Only is : ") ;
  SAYLN(listenOnly ? F("ON") : F("OFF"));
}

//+============================================================================= =======================================
void  loop (void) 
{
  int           rx[MBOX_USED + 1];  // Mailbox receive counters
  
  int           logs  = 0;          // log standard packets
  int           loge  = 0;          // log extrended packets
  int           lpCnt = 0;          // loop counter
  unsigned int  ms    = 0;          // millis() read

  // This loop avoids the need to jump in-and-out of main() every iteration
  for (memset(rx, 0, sizeof(rx));  true;  lpCnt++) {

    heartbeat(LED_RED2);  // 'cos why not
    check_buttons();      // All button events trigger a logging event (tick = 0)...

    // Catch Packets
    for (int mbox = 0;  mbox < MBOX_USED;  mbox++) {
      if (MSR_MRDY(mbox_sr_get(bus, mbox))) {  // If the mailbox is ready...
        rx[mbox]++;               // Count the packet
        intr_isr(bus);            // Call the m2_can interrupt service routine
        if (frame.ext)  loge++ ;  // Log extended packets
        else            logs++ ;  // Log Standard packets
        frame2eth(bus, mbox);     // Send the frame to the LAN
        //frame2pkt(bus, mbox);
      }
    }

    if (logging) {
      // Every INTERVAL ms we dump out some results
      ms = millis();
      if (ms >= tick) {
        // Dump logging stats
        FSAY("@ ");        BIGDEC(ms);
        for (int mbox = 0;  mbox < MBOX_USED;  mbox++) {
          FSAY(" : [");  SAY(bus);  FSAY(",");  SAY(mbox);  FSAY("]=");  BIGDEC(rx[mbox]);  
          rx[MBOX_USED] += rx[mbox];
        }
        FSAY(" : std=");   BIGDEC(logs);  
        FSAY(" : ext=");   BIGDEC(loge);  
        FSAY(" : pCnt=");  BIGDEC(pCnt);  FSAY("*");  SAY(FPP);  FSAY("=");  BIGDEC(pCnt * FPP);  
        FSAY("   Free=");  BIGDEC(lpCnt - rx[MBOX_USED]);
        FSAYLN("");

        if (serialEventRun) serialEventRun();  // I think this (essentially) flushes the serial port
        
        pCnt = logs = loge = lpCnt = 0;        // Reset all the counters
        memset(rx, 0, sizeof(rx));             // ...
        tick = millis() + INTERVAL;            // We will log again in INTERVAL mS
      }
    }

  } // for (lpCnt
}

