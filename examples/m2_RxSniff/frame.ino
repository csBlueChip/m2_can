#include "macros.h"

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
    };
  }
canPkt_t;

#define CPKT_MBOX_SHIFT           (0u)
#define CPKT_MBOX_MASK            (7u << CPKT_MBOX_SHIFT)
#define CPKT_MBOX_SET(cpkt, val)  ((cpkt.dev & ~CPKT_MBOX_MASK) | ((val << CPKT_MBOX_SHIFT) & CPKT_MBOX_MASK))

#define CPKT_BUS_SHIFT            (3u)
#define CPKT_BUS_MASK             (7u << CPKT_BUS_SHIFT)
#define CPKT_BUS_SET(cpkt, val)   ((cpkt.dev & ~CPKT_BUS_MASK) | ((val << CPKT_BUS_SHIFT) & CPKT_BUS_MASK))

#define CPKT_AID_MASK             ((1 << 29) - 1)
#define CPKT_TX                   (1u << 6)
#define CPKT_RX                   (0)
#define CPKT_TX_SET(cpkt)         (cpkt.dev |=  CPKT_TX_MASK)
#define CPKT_TX_CLR(cpkt)         (cpkt.dev &= ~CPKT_TX_MASK)
#define CPKT_TX_GET(cpkt)         (cpkt.dev &   CPKT_TX_MASK != 0)

#define CPKT_EXT                  (1u << 29)
#define CPKT_STD                  (0)
#define CPKT_EXT_SET(cpkt)        (cpkt.mid |=  CPKT_EXT_MASK)
#define CPKT_EXT_CLR(cpkt)        (cpkt.mid &= ~CPKT_EXT_MASK)
#define CPKT_EXT_GET(cpkt)        (cpkt.mid &   CPKT_EXT_MASK != 0)

#define CPKT_DLC_MASK             (7u)
#define CPKT_RTR                  (1u << 7)
#define CPKT_DATA                 (0)
#define CPKT_RTR_SET(cpkt)        (cpkt.dlc |=  CPKT_RTR_MASK)
#define CPKT_RTR_CLR(cpkt)        (cpkt.dlc &= ~CPKT_RTR_MASK)
#define CPKT_RTR_GET(cpkt)        (cpkt.dlc &   CPKT_RTR_MASK != 0)

//------------------------------------------------------------------------------ ---------------------------------------
canPkt_t    pkt;
canFrame_t  frame;

//+============================================================================= =======================================
// Right. Sod this human-readable crap...
//

// Oh great! No endian macros available!
#define htons(x) ( ((x)<< 8 & 0xFF00) | \
                   ((x)>> 8 & 0x00FF) )
#define ntohs(x) htons(x)

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)

void  prepPkt (int bus,  int mbox)
{
  uint64_t    md;
  int         i;
  
  if ( !rb_frame_get(bus, mbox, &frame) ) {
    SerialUSB.println("ring buffer : get failed") ;
    return;
  }

  pkt.dev = ((frame.dir == CDIR_TX) ? CPKT_TX : CPKT_RX)
          | (bus << CPKT_BUS_SHIFT) 
          | (mbox << CPKT_MBOX_SHIFT);
  pkt.ts  = 0;
  pkt.aid = (frame.ext ? CPKT_EXT : CPKT_STD) | htonl(frame.aid & CPKT_AID_MASK);
  pkt.fid = htonl(frame.fid);
  pkt.dlc = (frame.rtr ? CPKT_RTR : CPKT_DATA) | htonl(frame.dlc & CPKT_DLC_MASK);
  
  for (md = frame.data.md,  pkt.md = i = 0;  i < frame.dlc;  md >>= 8, i++)
    pkt.data[i] = md & 0xFF;
}

//+============================================================================= 
void  dumpPkt (int bus,  int mbox)
{
  prepPkt(bus, mbox);
//! oh my what to do with this data now we have it ?! :o
}

//+============================================================================= =======================================
void  countFrame (int bus,  int mbox)
{
  static uint32_t  cnt = 0;

  if (rb_frame_get(bus, mbox, &frame)) {
    cnt++ ;
    SAY(cnt, DEC) ;
  } else {
    FSAY("--") ;
  }
}

//+============================================================================= =======================================
void  justRead (int bus,  int mbox)
{
  if ( !rb_frame_get(bus, mbox, &frame) )  FSAY("!") ;
}

//+============================================================================= =======================================
void  readAndReport (int bus,  int mbox)
{
  if ( !rb_frame_get(bus, mbox, &frame) )  FSAY("!") ;
  else                                     FSAY(".") ;
}

//------------------------------------------------------------------------------ ---------------------------------------
char  s[128];

//+============================================================================= =======================================
// It seems that sprintf() is RIDICULOUSLY expensive
// Way too expensive to use between frames!
//
void  calcFrame (int bus,  int mbox)
{
  canFrame_t  frame;
  int         n;

  if ( !rb_frame_get(bus, mbox, &frame) ) {
    FSAYLN("ring buffer : get failed") ;
    return;
  }

  s[(n = 0)] = '\0';
  n += sprintf(s+n, "%c[%d:%d] @ %08lX : ", (frame.dir == CDIR_RX) ? '>' : '<', bus, mbox, frame.ts.time);
  
  if (frame.ext)  n += sprintf(s+n, "+%08lX/%08lX", frame.aid, frame.fid) ;
  else            n += sprintf(s+n, "-%03lX/%03lX", frame.aid, frame.fid) ;

  // Dump the MessageData
  n += sprintf(s+n, " data[%d]={", frame.dlc);
  
  if (frame.rtr) {
    n += sprintf(s+n, "rtr}");
    
  } else {
    int       i;
    char      hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    uint64_t  md      = frame.data.md;
    
    for (i = 0 ;  i < frame.dlc;  i++) {
      *(s+n++) = hex[(md & 0xF0) >> 4];
      *(s+n++) = hex[(md & 0x0F)];
      *(s+n++) = ',';
      md >>= 8;
    }
    
    if (i)  *(s+n-1) = '}' ;  
    else    *(s+n++) = '}' ;  
    *(s+n) = '\0';
  }
}

//+============================================================================= 
void  dumpFrame (int bus,  int mbox)
{
  calcFrame(bus, mbox);
  SAYLN(s);
}
  
//+============================================================================= =======================================
// What happens if we avoid sprintf?
//
char  x2a (int x)
{
  return (char)((x < 10) ? ('0' + x) : ('A' - 10 + x)) ;
}

//+============================================================================= 
int hex8 (char* s,  uint32_t x)
{
  uint8_t  i;
  for (i = 0;  i < (2 * 8);  i++)
    *s++ = x2a( (x & (0xF << i))  >>  ((((2 * 8) - i) - 1) * 4) );
  return 8;
}

//+============================================================================= 
void  calcFrame2 (int bus,  int mbox)
{
  canFrame_t  frame;
  int         n;

  if ( !rb_frame_get(bus, mbox, &frame) ) {
    FSAYLN("ring buffer : get failed") ;
    return;
  }

  s[(n = 0)] = '\0';
  
  //n += sprintf(s+n, "%c[%d:%d] @ %08lX : ", (frame.dir == CDIR_RX) ? '>' : '<', bus, mbox, frame.ts.time);
  s[n++] = (frame.dir == CDIR_RX) ? '>' : '<';
  s[n++] = '0' + bus;
  s[n++] = ',';
  s[n++] = '0' + mbox;
  s[n++] = '@';
  n += hex8(s+n, frame.ts.time);
  s[n++] = '=';

  //if (frame.ext)  n += sprintf(s+n, "+%08lX/%08lX", frame.aid, frame.fid) ;
  //else            n += sprintf(s+n, "-%03lX/%03lX", frame.aid, frame.fid) ;
  s[n++] = frame.ext ? '+' : '-';
  n += hex8(s+n, frame.aid);
  s[n++] = '/';
  n += hex8(s+n, frame.fid);

  // Dump the MessageData
  //n += sprintf(s+n, " data[%d]={", frame.dlc);
  s[n++] = '[';
  s[n++] = '0' + frame.dlc;
  s[n++] = ']';
  
  if (frame.rtr) {
    //n += sprintf(s+n, "rtr}");
    s[n++] = 'r';
    
  } else if (frame.dlc) {
    uint64_t  md = frame.data.md;
    
    for (int i = 0 ;  i < frame.dlc;  i++) {
      s[n++] = x2a((md & 0xF0) >> 4);
      s[n++] = x2a((md & 0x0F));
      s[n++] = ' ';
      md >>= 8;
    }
  }
  
  s[n++] = '|';
  s[n] = '\0';
}

//+============================================================================= 
void  dumpFrame2 (int bus,  int mbox)
{
  calcFrame2(bus, mbox);
  SAYLN(s);
}
  
