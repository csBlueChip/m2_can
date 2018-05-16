#include <UIPEthernet.h>
#include <utility/logging.h>

#include <m2_io.h>

#include "macros.h"

//------------------------------------------------------------------------------ ---------------------------------------
#define VER_MAJ 1
#define VER_MIN 0
#define VER_SUB 0

#define STR(s)  #s
#define XSTR(s) STR(s)
#define VERS    "v" XSTR(VER_MAJ) "." XSTR(VER_MIN) "." XSTR(VER_SUB)

//------------------------------------------------------------------------------ ---------------------------------------
IPAddress      myIP     = {192, 168, 0, 1};    // Default address
unsigned char  myMAC[6] = {0xAA, 0xAA, 0xAA, 0xAA, 0xBC, 0x01};

//+============================================================================= =======================================
void  help (void)
{
  FSAY(
    "# Help:\r\n" 
    "  LED:Red  = IO initialised\r\n"
    "     :Yel1 = Serial Port active (Flash = Waiting)\r\n"
    // AnyKey to abort waiting for a serial connection [Can't display - No serial connection yet!]
    "  SW1  = \r\n" 
    "  SW1+ = \r\n"
    "  SW2  = \r\n"
    "  SW2+ = \r\n"
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
    FSAYLN("# Sniff2Ethernet " VERS);
    led_on(LED_YEL1);
    
  } else {
    led_off(LED_YEL1);
  }
}

//+============================================================================= =======================================
void setup()
{
  led_init_all(NULL);  // Configure the LEDs
  led_on(LED_RED);     // Announce we've booted
  
  btn_init_all(5);     // Configure the buttons with a 5mS debounce
  
  serial_setup();      // Check for a serial monitor
  help();              // Display help

  setup_uip();

}

//+============================================================================= =======================================
#include <utility/uip.h>
#include <utility/uip_arp.h>

typedef
  union canPkt {
    uint8_t  pkt[];
    struct {
      uint32_t  ts;         // timestamp [undefined format]
      uint32_t  aid;        // Arbitration ID & 0x1FFFFFFF ...2^29 set for EXT packet
      uint32_t  fid;        // Family ID & 0x1FFFFFFF
      union {
        uint8_t   data[8];  // Always 8 byes, only 'dlc' bytes are used  // fixed packet length!
        uint64_t  md;       // Might be useful for htonll()
      };
      uint8_t   dlc;        // Data Length Count & 0x07 ...2^7 set for RTR packet
      uint8_t   dev;        // Device 
    } __attribute__ ((packed)) ;
  }
canPkt_t;

#define FCNT (10)
typedef
  struct c5bc {
    struct uip_eth_hdr  ethhdr;
    uint32_t            id;
    uint16_t            len;
    canPkt_t            canPkt[FCNT];
  } __attribute__ ((packed))
c5bc_t;

#define MAGIC_BE(s)   ( (uint32_t)((s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3]) )
#define ETHTYPE_C5BC  (0xC5BC)
#define C5BC_ID_CAN   MAGIC_BE("CAN1")

#define UIPBUF  ((c5bc_t*)&uip_buf[0])

void setup_uip () 
{
  Ethernet.begin(myMAC, myIP);
  FSAY("  Local IP    : ");  SAYLN(Ethernet.localIP());
  
  memset(UIPBUF->ethhdr.dest.addr, 0xff, 6);             // Dst MAC : Broadcast
  memcpy(UIPBUF->ethhdr.src.addr, uip_ethaddr.addr, 6);  // Src MAC : local
  UIPBUF->ethhdr.type = HTONS(ETHTYPE_C5BC);             // Type    : C5BC

  UIPBUF->id  = htonl(C5BC_ID_CAN);
  UIPBUF->len = HTONS(sizeof(canPkt_t) * FCNT);
  
  uip_len = sizeof(c5bc_t);
}

void loop()
{
  unsigned int ms;
  unsigned int cnt;
  unsigned int b4 = 0, ft = 0;
  for (cnt = 0,  ms = millis() + 1000;  millis() < ms;  cnt++) {
//    UIPBUF->d = htonl(0x1A2B3C4D);
//    b4 = millis();
//    SAY(b4-ft);
//    FSAY(" : ");
//    SAY(b4);
    Ethernet.network_send();
//    ft = millis();
//    FSAY(" - eth - ");
//    SAY(ft);
//    FSAY(" : ");
//    SAYLN(ft-b4);
  }
//  SAYLN();
  SAY(cnt);  FSAY(" * ");  SAY(FCNT);  FSAY(" = ");  SAYLN(cnt * FCNT);
//  delay(2000);
}

