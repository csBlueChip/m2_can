/*
===============================================================================                 
                              PINOUTS & WIRING
===============================================================================                 

Looking at the M2 from the BACK, with the SDCard/USB port on the TOP,
so you are looking IN to the FEMALE connector, from the USER-SIDE

                                            [83]
                                 SPI0_CK   SPI_CS2    +3v3  Gnd
                                       |     |           |  |
                      ,----------------^-----^-----------^--+.
    Macchina M2       | 1| 3| 5| 7| 9|11|13|15|17|19|21|23|25|
        [J5]          |--+--+--+--+--+--+--+--+--+--+--+--+--+
 "26 pin Connector"   | 2| 4| 6| 8|10|12|14|16|18|20|22|24|26|
                      `----------------^-----v-----v---------'
                                       |     |     |
                               SPI0_MISO SPI0_MOSI +5v0
                               

No, "SPI_CS2" is NOT a typing error ...Well, at least, not MY typing error!
And, YES, it IS the SPI CS pin on J5/15 ...Arduino Pin Number 83

===============================================================================                

The pins on my ENC28J60's are labelled (silkscreened)
Viewed from the TOP (with the MALE header pins pointing TOWARD you),
and with the components {driver, PHY, etc.} on the ->RIGHT->

                  ,-----.
            CLK   |  |  |   NT
                  |--+--+   
            WOL   |  |  >-- SO
                  |--+--+   
 ENC28J60    SI -->  |  <-- SCK
                  |--+--+   
             CS -->  |  |   RST
                  |--+--+
            VCC -->  |  +-- GND
                  `-----'

# These things are made out of the purest Chinesium
  ...Your pinout may well be different to mine - check it!
   
# VCC is allegedly +3V3
  ...And, indded, some of them run quite nicely on +3V3
  ...BUT I have had devices that required 5V0 to
     a) Send packets to the network, and
     b) Get very hot

# NT is probably some kind of Interrupt Line

# WOL will be a Wake-On-LAN feature
  ...So the ENC28J60 can poke the Arduino if it sees a Wakeup packet
  
# RST is the Reset pin
  Now... The M2 does NOT expose Rst on J5
  So... I'm going to ignore it for now and hope it's not a problem
        ...Which I think it might be :(
        ...Maybe we can use the Rst pin on the XBEE connector?

# CLK is ...errr, no idea - but it's sure not the SPI Clock

===============================================================================                

You need to attach your ENC28J60 Ethernet board to the M2 via SPI Bus.
The SPI bus is availabe on the "26-pin Connector" [J5]
You will also need +3V3 and Gnd to power the ENC28J60
  J5/11 = SAM/PA27 : SPI0_CK     ---->   SCK
  J5/12 = SAM/PA25 : SPI0_MISO   <----   SO
  J5/15 = SAM/PB21 : SPI_CS2     ---->   CS
  J5/16 = SAM/PA26 : SPI0_MOSI   ---->   SI
  J5/20 =          : +5V0        ---->   n/c (see above notes)
  J5/23 =          : +3V3        ---->   VCC
  J5/25 =          : Gnd         -----   GND

===============================================================================                
*/

#if 0

struct uip_eth_hdr {
  struct uip_eth_addr dest;
  struct uip_eth_addr src;
  u16_t type;
};

struct can_hdr {
  struct uip_eth_hdr  ethhdr;
  u32_t               id;
  u16_t               len;
  canPkt_t            canPkt;
};

#define MAGIC_BE(s)   ((s[0] << 24) || (s[1] << 16) || (s[2] << 8) || s[3])
#define MAGIC_LE(s)   ((s[3] << 24) || (s[2] << 16) || (s[1] << 8) || s[0])
#define ETHTYPE_C5BC  (0xC5BC)
#define C5BC_ID_CAN   (MAGIC_BE("CAN1"))

#define BUF  ((struct can_hdr *)&uip_buf[0])

  memset(BUF->ethhdr.dest.addr, 0xff, 6);             // Dst MAC : Broadcast
  memcpy(BUF->ethhdr.src.addr, uip_ethaddr.addr, 6);  // Src MAC : local
  BUF->ethhdr.type = HTONS(ETHTYPE_C5BC);             // Type    : C5BC

  BUF->id  = ETHTYPE_C5BC_CAN;
  BUF->len = sizeof(canPkt_t);

  BUF->canPkt.foo = bar;

  uip_len = sizeof(struct can_hdr);

bool UIPEthernetClass::network_send()
\  uip_packet = Enc28J60Network::allocBlock(uip_len);
\  if (uip_packet == NOBLOCK)  return false ;
\  Enc28J60Network::writePacket(uip_packet,0,uip_buf,uip_len);
\  Enc28J60Network::sendPacket(uip_packet);
\  Enc28J60Network::freeBlock(uip_packet);
\  uip_packet = NOBLOCK;

#endif


#if 0

//+============================================================================= =======================================
// This function never returns
//
void  testcs (void)
{
  // Green: SPI_CS is on J5
  // Red  : SPI_CS is <in the wrong place>
  led_on((ENC28J60_CONTROL_CS == 83) ? LED_GRN2 : LED_RED2);
  delay(1500);

  // Set the pin ready for test
  pinMode(ENC28J60_CONTROL_CS, OUTPUT);
  led_off(LED_GRN2);
  led_off(LED_RED2);

  // Start test
  led_on(LED_GRN);
  for(;;){
    // 3 seconds on
    led_on(LED_BLU);
    digitalWrite(ENC28J60_CONTROL_CS, HIGH);
    delay(3000);

    // 3 seconds off
    led_off(LED_BLU);  
    digitalWrite(ENC28J60_CONTROL_CS, LOW);
    delay(3000);
  }
}

//------------------------------------------------------------------------------ ---------------------------------------
char           hostname[16];
IPAddress      zip    = {0, 0, 0, 0};          // 0.0.0.0 - used to reset the interface
IPAddress      local  = {172, 26, 26, 250};    // Default address
unsigned char  mac[6] = {0xAA, 0xAA, 0xAA, 0xAA, 0xBC, 0x01};
EthernetUDP    udp;

#define PORT   54321

//------------------------------------------------------------------------------ ---------------------------------------
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

typedef
  struct ethii {
    uint8_t   dmac[6];    
    uint8_t   smac[6];    
    uint16_t  type;
    canPkt_t  data;
    uint8_t   padding[46 - sizeof(canPkt_t)]  // You do NOT *need* to add your own padding!
    uint32_t  crc;
  } __attribute__ ((packed));
ethii_t;

ethii_t  ethii;

//+============================================================================= =======================================
void  setup (void) 
{
  led_init_all(NULL);
  
//testcs();

  SerialUSB.begin(115200);
  led_on(LED_GRN2);
  while (!SerialUSB);
  led_off(LED_GRN2);

  eth_setIP(zip);  // use 'zip' for DHCP

  memset(ethii.dmac, 0xFF, 6);
  memcpy(ethii.smac, mac, 6);
  ethii.type = 0xC5BC;
  memset(ethii.data, 0xEE, sizeof(ethii.data));
  memset(ethii.padding, 0x00, sizeof(ethii.padding));
  ethii.crc = 0x12345678;
}

//+============================================================================= =======================================
#define XX 0

#if (XX==1)
void  loop (void) 
{
  while (!udp.beginPacket(IPAddress(172,26,26,135), PORT)) ;
  udp.write("SOSSIJ",6);
  udp.endPacket();
  udp.stop();
}

//+============================================================================= 
#elif (XX==0)
void  loop (void) 
{
  int           success;
  unsigned int  ms;
  int           len;

  //beginPacket fails if remote ethaddr is unknown. In this case an
  //arp-request is send out first and beginPacket succeeds as soon
  //the arp-response is received.
  for ( success = 0,  ms = millis() + 5000;
        !success && (millis() < ms);
        success = udp.beginPacket(IPAddress(172,26,26,135), PORT) ) ;
      
  if (success) {
    udp.write("SPANKY HAM");
    
    for ( success = udp.endPacket();
          !success && (millis() < ms);
          success = udp.parsePacket() ) ; //check for new udp-packet:
        
    if (success) {
      for (len = 0;  udp.available();  len++)  udp.read() ;
      udp.flush();
    }
  }
  udp.stop();
  FSAY(".");
}
#endif

#endif // 0

