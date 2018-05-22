#ifndef UIP_ETH_H_
#define UIP_ETH_H_

#include <UIPEthernet.h>
#include <utility/logging.h>
#include <utility/uip.h>
#include <utility/uip_arp.h>

#include "m2_can.h"

extern uint32_t pCnt;

//------------------------------------------------------------------------------ ---------------------------------------
IPAddress      myIP     = {192, 168,   0,  10};
IPAddress      myMask   = {255, 255, 255,   0};
IPAddress      bcastIP  = {192, 168,   0, 255};
IPAddress      myGW     = {192, 168,   0,   1};
IPAddress      myDNS    = {  8,   8,   8,   8};

unsigned char  myMAC[6]    = {0x02, 0xAA, 0xAA, 0xAA, 0xBC, 0x01};
unsigned char  bcastMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//------------------------------------------------------------------------------ ---------------------------------------
// This is what wireshark will see 
//
typedef
  union canEth {
    uint8_t  pkt[];
    struct {
      uint8_t   dev;        // Device [tlbb'bmmm = tx:lcl:bus:mbox] or [rabb'bmmm = rx:ack:bus:mbox]
      uint32_t  ts;         // timestamp [undefined format]
      uint32_t  aid;        // Arbitration ID & 0x1FFFFFFF ...2^29 set for EXT packet
      uint32_t  fid;        // Family ID & 0x1FFFFFFF
      uint8_t   dlc;        // Data Length Count & 0x07 ...2^7 set for RTR packet
      union {
        uint8_t   data[8];  // Always 8 byes, only 'dlc' bytes are used  // fixed frame length!
        uint64_t  md;       // Might be useful for htonll()
      };
    } __attribute__ ((packed)) ;
  }
canEth_t;

// AID
#define CPKT_AID_MASK             ((1 << 29) - 1)

#define CPKT_EXT                  (1u << 29)
#define CPKT_STD                  (0u << 29)
#define CPKT_EXT_MASK             (CPKT_EXT)
#define CPKT_EXT_SET(cpkt)        (cpkt.mid |=  CPKT_EXT_MASK)
#define CPKT_EXT_CLR(cpkt)        (cpkt.mid &= ~CPKT_EXT_MASK)
#define CPKT_EXT_GET(cpkt)        (cpkt.mid &   CPKT_EXT_MASK != 0)

// DLC
#define CPKT_DLC_MASK             (0xFu)
#define CPKT_RTR                  (1u << 7)
#define CPKT_DATA                 (0u << 7)
#define CPKT_RTR_MASK             (CPKT_RTR)
#define CPKT_RTR_SET(cpkt)        (cpkt.dlc |=  CPKT_RTR_MASK)
#define CPKT_RTR_CLR(cpkt)        (cpkt.dlc &= ~CPKT_RTR_MASK)
#define CPKT_RTR_GET(cpkt)        (cpkt.dlc &   CPKT_RTR_MASK != 0)

// DEV
#define CPKT_TX                   (1u << 7)
#define CPKT_RX                   (0u << 7)
#define CPKT_TX_MASK              (CPKT_TX)
#define CPKT_TX_SET(cpkt)         (cpkt.dev |=  CPKT_TX_MASK)
#define CPKT_TX_CLR(cpkt)         (cpkt.dev &= ~CPKT_TX_MASK)
#define CPKT_TX_GET(cpkt)         (cpkt.dev &   CPKT_TX_MASK != 0)

// If Rx (acked, not-acked)
#define CPKT_ACK                  (1 << 6)
#define CPKT_NOACK                (0 << 6)

// If Tx (generated/received)
#define CPKT_GEN                  (1 << 6)
#define CPKT_RXD                  (0 << 6)

#define CPKT_BUS_SHIFT            (3u)
#define CPKT_BUS_MASK             (7u << CPKT_BUS_SHIFT)
#define CPKT_BUS_SET(cpkt, val)   ((cpkt.dev & ~CPKT_BUS_MASK) | ((val << CPKT_BUS_SHIFT) & CPKT_BUS_MASK))
#define CPKT_BUS_GET(cpkt)        ((cpkt.dev & CPKT_BUS_MASK) >> CPKT_BUS_SHIFT)

#define CPKT_MBOX_SHIFT           (0u)
#define CPKT_MBOX_MASK            (7u << CPKT_MBOX_SHIFT)
#define CPKT_MBOX_SET(cpkt, val)  ((cpkt.dev & ~CPKT_MBOX_MASK) | ((val << CPKT_MBOX_SHIFT) & CPKT_MBOX_MASK))
#define CPKT_MBOX_GET(cpkt)       ((cpkt.dev & CPKT_MBOX_MASK) >> CPKT_MBOX_SHIFT)

//------------------------------------------------------------------------------ 
#define FPP (2)  // frames per packet

typedef
  struct ethii {
    struct uip_eth_hdr  ethhdr;
    canEth_t            canEth[FPP];
  } __attribute__ ((packed))
ethii_t;

// Compile-time Fragmentation check
#define MTU  1500
CASSERT(sizeof(ethii_t) <= MTU, uip_eth_h_);

#define ETHTYPE_CB   (0xCB00 | FPP)

//------------------------------------------------------------------------------ ---------------------------------------
// Buffer pointers
//
#define ETHPKT       ((ethii_t*)&uip_buf[0])
#define CANFRAME(n)  ((canEth_t*)&(ETHPKT->canEth[n]))

//------------------------------------------------------------------------------ ---------------------------------------
extern  ethii_t     pkt;
extern  canFrame_t  frame;

//+============================================================================= =======================================
void  prepPkt   (int bus,  int mbox, canEth_t* canEth) ;
void  frame2pkt (int bus,  int mbox) ;
void  frame2eth (int bus,  int mbox) ;

#endif // UIP_ETH_H_

