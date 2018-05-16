#if 0



char           hostname[16];
IPAddress      zip    = {0, 0, 0, 0};          // 0.0.0.0 - used to reset the interface
IPAddress      local  = {172, 26, 26, 250};    // Default address
unsigned char  mac[6] = {0xAA, 0xAA, 0xAA, 0xAA, 0xBC, 0x01};

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

//+============================================================================= =======================================
void  eth_setIP (IPAddress ip) 
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

//+============================================================================= =======================================
#define PORT 54321
EthernetUDP   udp;

//+============================================================================= =======================================

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
    uint8_t   padding[46 - sizeof(canPkt_t)]
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


#endif

