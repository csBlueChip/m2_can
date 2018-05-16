#include "macros.h"
#include "bigdec.h"

//------------------------------------------------------------------------------ ---------------------------------------
extern uint32_t   baud_list[] ;
extern uint32_t*  pBaud ;
extern int        bus ;
extern mid_t      se ;

//+============================================================================= =======================================
void  help (void)
{
  FSAY(
    "# Help:\r\n" 
    "  LED:Red  = IO initialised\r\n"
    "     :Yel1 = Serial Port active (Flash = Waiting)\r\n"
    "     :Yel2 = CAN Initialised\r\n"
    "     :Yel3 = Baud Configured    (Flash = Error)\r\n"
    "     :Grn  = Mailbox configured (Flash = Error)\r\n"
    // AnyKey to abort waiting for a serial connection [Can't display - No serial connection yet!]
    "  SW1  = Change Speed\r\n" 
    "  SW1+ = Change {std,ext}\r\n"
    "  SW2  = Restart timer\r\n"
    "  SW2+ = Change CAN bus\r\n"
  );
}

//+============================================================================= =======================================
void  serial_setup (void)
{
  SerialUSB.begin(115200);
  
  // Wait for a Serial port; stop waiting if a button is pressed  
  for (0;  !SerialUSB && !btn_onDown(BTN_SW1) && !btn_onDown(BTN_SW2);  delay(300))
    led_toggle(LED_YEL1);  // Flash the LED while we are waiting
  
  // If we found a monitor - say hello
  if (SerialUSB) {
    FSAYLN("# Transmission Flood " VERS);
    led_on(LED_YEL1);
    
  } else {
    led_off(LED_YEL1);
  }
}

//+============================================================================= =======================================
void  can_setup (void)
{
  uint32_t  baud;

  // Initialise the CAN library
  can_init_m2();         
  FSAYLN("# CAN controller initialised");
  led_on(LED_YEL2);

  FSAY("# Set CAN");
  SAY(bus);
  FSAY(" @ ");
  BIGDEC(*pBaud);
  
  if ( (baud = can_baud_set(bus, *pBaud)) ) {  
    FSAYLN(" - OK");
    led_on(LED_YEL3) ;
    
  } else {
    FSAYLN(" - *FAIL*");
    led_flash_die(LED_YEL3, 300) ;
  }
  
}

//+============================================================================= =======================================
// We use MID/MAM=0x155 (or 0x15555555) to avoid Bit Stuffing
//
void  mbox_setup (void)
{
  uint32_t  mid;
  uint32_t  mam;
  
  mbox_reset(bus,0);
  
  if (se == MBOX_STD)  mid = mam = 0x155 ;
  else                 mid = mam = 0x15555555 ; 
  
  FSAY("# mbox[");
  SAY(bus);
  FSAY(",0]");
  
  if (mbox_mode_set(bus,0, mid,mam,se, MBOX_TX)) {
    FSAY(" = Tx,");
    SAY((se==MBOX_STD) ? F("std") : F("ext"));
    FSAY(":");
    SAY(mid, HEX);
    FSAY("/");
    SAYLN(mam, HEX);
    led_on(LED_GRN) ;
    
  } else {
    FSAYLN(" - *FAIL*"); 
    led_flash_die(LED_GRN, 300);
  }
}

