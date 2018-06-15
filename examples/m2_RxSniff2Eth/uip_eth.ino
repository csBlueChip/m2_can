#include "macros.h"
#include "uip_eth.h"

//------------------------------------------------------------------------------ ---------------------------------------
ethii_t     pkt;
canFrame_t  frame;
uint32_t    pCnt;

//+============================================================================= =======================================
void  prepPkt (int bus,  int mbox, canEth_t* canEth)
{
  uint64_t    md;
  int         i;
  
  if ( !rb_frame_get(bus, mbox, &frame) ) {
    SerialUSB.println("ring buffer : get failed") ;
    return;
  }

  canEth->dev = ((frame.dir == CDIR_TX) ? CPKT_TX : CPKT_RX)
              | (listenOnly ? CPKT_NOACK : CPKT_ACK)  //! This is the Rx condition - need to add Tx conditition      AROOOGAHHHHH <--- *****
              | (bus << CPKT_BUS_SHIFT) 
              | (mbox << CPKT_MBOX_SHIFT);
  canEth->ts  = 0;
  canEth->aid = htonl((frame.ext ? CPKT_EXT : CPKT_STD) | (frame.aid & CPKT_AID_MASK));
  canEth->fid = htonl(frame.fid);
  canEth->dlc = (frame.rtr ? CPKT_RTR : CPKT_DATA) | (frame.dlc & CPKT_DLC_MASK);
  
  for (md = frame.data.md,  canEth->md = i = 0;  i < frame.dlc;  md >>= 8, i++)
    canEth->data[i] = md & 0xFF;
}

//+============================================================================= 
void  frame2pkt (int bus,  int mbox)
{
  static int  fCnt = 0;

  prepPkt(bus, mbox, CANFRAME(fCnt));

  if (++fCnt == FPP) {
    fCnt = 0;  
  }
}

  //+============================================================================= 
void  frame2eth (int bus,  int mbox)
{
  static int  fCnt = 0;
  
  prepPkt(bus, mbox, CANFRAME(fCnt));

  if (++fCnt == FPP) {
    Ethernet.network_send();  
    pCnt++;
    fCnt = 0;  
  }
}

