#include "macros.h"
#include "bigdec.h"

//------------------------------------------------------------------------------ ---------------------------------------
extern int  bus ;

//+============================================================================= =======================================
void  help (void)
{
  FSAY(
    "# Help:\r\n" 
    "  LED:Red  = IO initialised\r\n"
    "     :Yel1 = Serial Port active (Flash = Waiting)\r\n"
    "     :Yel2 = CAN Initialised\r\n"
    "     :Yel3 = Baud Detected\r\n"
    "     :Grn  = Mailbox configured (Flash = Error)\r\n"
    // AnyKey to abort waiting for a serial connection [Can't display - No serial connection yet!]
    "  SW1  = Renegotiate Speed\r\n" 
    "  SW1+ = Toggle decoding/logging\r\n"
    "  SW2  = Restart timer\r\n"
    "  SW2+ = Change CAN bus\r\n"
  );
}
//+============================================================================= =======================================
void serial_setup (void)
{
  SerialUSB.begin(115200);
  
  // Wait for a Serial port; stop waiting if a button is pressed  
  for (0;  !SerialUSB && !btn_onDown(BTN_SW1) && !btn_onDown(BTN_SW2);  delay(300))
    led_toggle(LED_YEL1);  // Flash the LED while we are waiting
  
  // If we found a monitor - say hello
  if (SerialUSB) {
    FSAYLN("# Sniffer " VERS);
    led_on(LED_YEL1);
    
  } else {
    led_off(LED_YEL1);
  }
}

//+============================================================================= =======================================
void  can_setup (void)
{
  uint32_t  baud;
  bool      fail;

  // Initialise the CAN library
  can_init_m2();         
  FSAYLN("# CAN controller initialised");
  led_on(LED_YEL2);

  // Start the CAN busses running at suitable baud rates
  fail = false; 
  
  FSAY("# Try CAN");
  SAY(bus);
  FSAY(" = Autobaud : ");
  
# define METHOD 1
# if METHOD == 1  
    if ( !(baud = can_baud_set(bus, CAN_BAUD_AUTO)) )  {  FSAYLN("*FAIL*");  fail = true;  }

# elif METHOD == 2
    if ( !(baud = can_baud_auto(bus, NULL))         )  {  FSAYLN("*FAIL*");  fail = true;  }

# elif METHOD == 3
    static const  uint32_t  baudList[] = { CAN_BAUD_250K, CAN_BAUD_500K,  CAN_BAUD_1000K, CAN_BAUD_EOL };
    if ( !(baud = can_baud_auto(bus, baudList))     )  {  FSAYLN("*FAIL*");  fail = true;  }

# endif
# undef METHOD

  else                                                 {  BIGDEC(baud);  SAYLN();  }
  
//  if (fail)  led_flash_die(LED_YEL3, 300) ;

  if (fail)  led_off(LED_YEL3) ;
  else       led_on(LED_YEL3) ;
}

//+============================================================================= =======================================
//_pp1232 : [RW] Message Acceptance Mask
// 
// | 00 | MIDE | IDvA.11bits | IDvB.18bits |
// 
// * MIDE: Identifier Version 
// 0: Compares IDvA          from the received frame with the CAN_MIDx register masked with CAN_MAMx register. 
// 1: Compares IDvA and IDvB from the received frame with the CAN_MIDx register masked with CAN_MAMx register. 
// 
// SO:
//   mbox_mode_set(bus,0, 0x00000000,0x00000000,MBOX_STD, MBOX_RX);
//   mbox_mode_set(bus,1, 0x00000000,0x00000000,MBOX_EXT, MBOX_RX); 
// 
// There is no difference between these two mailboxes! 
// As SAM (demonstrably) does NOT check the actual IDE bit when seelcting a mailbox!
// ...and ALL packets end up in mailbox 0 (because 0 < 1)
//
void  mbox_setup (void)
{
  int  fail = 0;    

  FSAY("# mbox[");  SAY(bus);  FSAY(",0]");
  if ( !mbox_mode_set(bus,0, 0x00000000,0x00000000,MBOX_STD, MBOX_RX) ) {
    FSAYLN(" - *FAIL*");
    fail++;
  } else {
    FSAYLN(" = Rx,Promiscuous,Std [see notes]");
  }
  FSAY("# ringbuffer[");  SAY(bus);  FSAY(",0]");
  if ( !rb_set(bus,0, RBSIZ_16) ) {
    FSAYLN(" - *FAIL*");
    fail++;
  } else {
    FSAYLN(" - OK");
  }
    
//  if ( !mbox_mode_set(bus,1, 0x00000000,0x00000000,MBOX_EXT, MBOX_RX) ) {
//    FSAYLN("mbox[bus,1]/ext - setup failed");  
//    fail++;
//  } else {
//    FSAYLN("mbox[bus,1] = Rx,Promiscuous,Ext");
//  }
//  if ( !rb_set(bus,1, RBSIZ_16) ) {
//    FSAYLN("ringbuf[bus,1] - setup failed");
//    fail++;
//  } else {
//    FSAYLN("ringbuf[bus,1] - OK");
//  }
  
  if (fail)  led_flash_die(LED_GRN, 300) ;
}

