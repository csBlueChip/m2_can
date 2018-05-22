#include "macros.h"
#include "bigdec.h"
#include "uip_eth.h"

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
    "     :Grn  = Mailbox(es) configured (Flash = Error)\r\n"
    // AnyKey to abort waiting for a serial connection [Can't display - No serial connection yet!]
    "  SW1  = Renegotiate Speed\r\n" 
    "  SW1+ = Change CAN bus\r\n"
    "  SW2  = Toggle decoding/logging\r\n"
    "  SW2+ = Toggle Listen-Only mode (IE. No Frame Ack)\r\n"
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
bool  can_setup (void)
{
  uint32_t  baud;
  bool      fail;

  // Initialise the CAN library
  can_init_m2();         
  FSAYLN("# CAN controller initialised");
  led_on(LED_YEL2);

  // Start the CAN busses running at suitable baud rates
  fail = false; 
  
  FSAY("# Try CAN");  SAY(bus);  FSAY(" = Autobaud : ");
  if ( !(baud = can_baud_set(bus, CAN_BAUD_AUTO)) )  {  FSAYLN("*FAIL*");  fail = true;  }
  else                                               {  BIGDEC(baud);  SAYLN();  }
  
  if (fail)  return led_off(LED_YEL3), false ;
  else       return led_on(LED_YEL3),  true ;
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
// LIE: There is no difference between these two mailboxes! 
//      As SAM (demonstrably) does NOT check the actual IDE bit when seelcting a mailbox!
//      ...and ALL packets end up in mailbox 0 (because 0 < 1)
//
// It seems that STD mailboxes will collect EXT packets
// ...but NOT the other way around!
// ...This DOES actually make sense if you think about how packets get masked
// ...It's still (imho) a bug in the SAM though
//
void  mbox_setup (void)
{
  int  fail = 0;    

  for (int mbox = 0;  mbox < MBOX_USED;  mbox++) {
    FSAY("# mbox[");  SAY(bus);  FSAY(",");  SAY(mbox);  FSAY("]");
    if ( !mbox_mode_set(bus,mbox, 0x00000000,0x00000000,MBOX_STD, MBOX_RX) ) {
      FSAYLN(" - *FAIL*");
      fail++;
    } else {
      FSAYLN(" = Rx,Promiscuous,Std+Ext");
    }
    FSAY("  ringbuffer[");  SAY(bus);  FSAY(",");  SAY(mbox);  FSAY("]");
    if ( !rb_set(bus,mbox, RBSIZ_16) ) {
      FSAYLN(" - *FAIL*");
      fail++;
    } else {
      FSAYLN(" - OK");
    } 
  }
    
  if (fail)  led_flash_die(LED_GRN, 300) ;
}

//+============================================================================= =======================================
void  eth_setup (void) 
{
  //eth_setup_IP(myIP);
  Ethernet.begin(myMAC, myIP);  // We only supply an IP address to stop UIP from trying to DHCP
  
  memcpy(ETHPKT->ethhdr.dest.addr, bcastMAC, 6);         // Dst MAC : Broadcast
  memcpy(ETHPKT->ethhdr.src.addr, uip_ethaddr.addr, 6);  // Src MAC : local
  ETHPKT->ethhdr.type = HTONS(ETHTYPE_CB);               // Type    : CB??

  memset(ETHPKT->canEth, 0, sizeof(canEth_t) * FPP);

  uip_len = sizeof(ethii_t);
}

//+============================================================================= =======================================
// This function is not used by this Sketch - because we don't need an IP address
//
#if 0
void  eth_setup_IP (IPAddress ip) 
{
  int  len = sprintf(hostname, "%.*s_%02X%02X", sizeof(hostname) - 6, "MacChinaM2", mac[4], mac[5]);
  Ethernet.setHostname(hostname, len);
  
  Ethernet.configure(zip, zip, zip, zip); // ip, dns, gw, subnet

  led_off(LED_RED);
  led_off(LED_GRN);
  if (memcmp(&ip, &zip, sizeof(IPAddress)) == 0) {
    FSAY("# DHCP...");
    led_on(LED_BLU);
    if (!Ethernet.begin(mac)) {
      led_off(LED_BLU);
      led_on(LED_RED);
      FSAYLN(" FAIL");
      return;
      
    } else {
      led_off(LED_BLU);
      led_on(LED_GRN);
      FSAYLN(" OK");
    }
    
  } else {
    FSAYLN("# Set IP...");
    led_on(LED_RED2);
    Ethernet.begin(mac, ip);
    led_off(LED_RED2);
    
    led_on(LED_RED);
    led_on(LED_GRN);
  }
  
  FSAY("  Local IP    : ");  SAYLN(Ethernet.localIP());
  FSAY("  Gateway     : ");  SAYLN(Ethernet.gatewayIP());
  FSAY("  DNS Server  : ");  SAYLN(Ethernet.dnsServerIP());
  FSAY("  Netmask     : ");  SAYLN(Ethernet.subnetMask());
}
#endif

