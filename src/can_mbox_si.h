#ifndef CAN_MBOX_SI_H_
#define CAN_MBOX_SI_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_mbox.h"
#include "can_intr_si.h"

//@============================================================================= =======================================
static inline  bool  mbox_init    (int bus)             {  return mbox_reset_all(bus);  }

static inline  bool  mbox_run     (int bus,  int mbox)  {  return intr_enable( bus, intr_mask_mbox[mbox]);  }
static inline  bool  mbox_pause   (int bus,  int mbox)  {  return intr_disable(bus, intr_mask_mbox[mbox]);  }

static inline  bool  mbox_disable (int bus,  int mbox)  {  return mbox_reset(bus, mbox);  }

//============================================================================== =======================================
// These functions MUST assume all masks sent are valid!
// Technically it is impossible to parse an invalid mask
// Eg. It is not possible to catch: mbox_setPriority(CAN_MMR_MOT_MB_DISABLED)
//     because CAN_MMR_MOT_MB_DISABLED==0  and  0 is a valid priority
//     ...but it will probably not have the desired effect
// Oh for the day people start using "typedef enum" with a bit more fervence :/
//

//@============================================================================= ======================================= MR
//_pp1231 : [RW] Message Mode Register
static inline
bool  mbox_mr_set (int bus,  int mbox,  uint32_t mask,  uint32_t bits)
{
	MBOX(bus,mbox).CAN_MMR = (MBOX(bus,mbox).CAN_MMR & ~mask) | bits;
	return true;
}

//@=========================================================
//_pp1231 : [RW] Message Mode Register
static inline
uint32_t  mbox_mr_get (int bus,  int mbox)
{
	return MBOX(bus,mbox).CAN_MMR;
}

//@=============================================================================
// CAN_MMR_MOT_MB_DISABLED......Mailbox is disabled.
//                              This prevents receiving or transmitting any messages with this mailbox.
// CAN_MMR_MOT_MB_RX............Reception Mailbox.
//                              If a message is received while the mailbox data register is full, it is discarded.
// CAN_MMR_MOT_MB_RX_OVERWRITE..Reception mailbox with overwrite.
//                              If a message is received while the mailbox is full, it overwrites the previous message.
// CAN_MMR_MOT_MB_TX............Transmit mailbox.
// CAN_MMR_MOT_MB_CONSUMER......Consumer Mailbox.
//                              Mailbox is configured in reception but behaves as a Transmit Mailbox,
//                              i.e., it sends a remote frame and waits for an answer.
// CAN_MMR_MOT_MB_PRODUCER......Producer Mailbox.
//                              Mailbox is configured in transmission but also behaves like a reception mailbox,
//                              i.e., it waits to receive a Remote Frame before sending its contents.
//
static inline
bool  mbox_mot_set (int bus,  int mbox,  uint32_t mode)
{
	return mbox_mr_set(bus, mbox, CAN_MMR_MOT_Msk, mode);
}

//@=========================================================
static inline
uint32_t  mbox_mot_get (int bus,  int mbox)
{
	return mbox_mr_get(bus, mbox) & CAN_MMR_MOT_Msk;
}

//@=============================================================================
// Priority is {0..15} ...Lower number is better priority [like chart music]
//
static inline
bool  mbox_pri_set (int bus,  int mbox,  uint32_t pri)
{
	return mbox_mr_set(bus, mbox, CAN_MMR_PRIOR_Msk, CAN_MMR_PRIOR(pri));
}

//@=========================================================
static inline
bool  mbox_pri_get (int bus,  int mbox)
{
	return (mbox_mr_get(bus, mbox) & CAN_MMR_PRIOR_Msk) ? true : false ;
}

//@=============================================================================
//_pp1208 : TimeMark
//
static inline
bool  mbox_tmark_set (int bus,  int mbox,  uint32_t tmark)
{
	return mbox_mr_set(bus, mbox, CAN_MMR_MTIMEMARK_Msk, CAN_MMR_MTIMEMARK(tmark));
}

//@=========================================================
static inline
uint32_t  mbox_tmark_get (int bus,  int mbox)
{
	return (mbox_mr_get(bus, mbox) & CAN_MMR_MTIMEMARK_Msk) >> CAN_MMR_MTIMEMARK_Pos;
}

//@============================================================================= ======================================= MAM
//_pp1232 : [RW] Message Acceptance Mask
// 
// | 00 | MIDE | IDvA.11bits | IDvB.18bits |
// 
// • MIDE: Identifier Version 
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
static inline
bool  mbox_mam_set (int bus,  int mbox,  uint32_t mam,  mid_t se)
{
	MBOX(bus,mbox).CAN_MAM = (se == MBOX_EXT) ? (CAN_MAM_MIDE | mam) : CAN_MAM_MIDvA(mam);
	return true;
}

//@============================================================================= ======================================= MID
//_pp1233 : [RW] Message ID
static inline
bool  mbox_mid_set (int bus,  int mbox,  uint32_t mid,  mid_t se)
{
	MBOX(bus,mbox).CAN_MID = (se == MBOX_EXT) ? (CAN_MID_MIDE | mid) : CAN_MID_MIDvA(mid);
	return true;
}

//@============================================================================= ======================================= FID
//_pp1234 : [R-] Family ID
static inline  uint32_t  mbox_fid_get (int bus,  int mbox)  {  return MBOX(bus,mbox).CAN_MFID;  }

//@============================================================================= ======================================= SR
//_pp1235-1237 : [R-] Message Status
//_PP1235
// NOTE: The action of reading SR clears MMI
//
static inline  uint32_t  mbox_sr_get  (int bus,  int mbox)  {  return MBOX(bus,mbox).CAN_MSR;   }

static inline  uint32_t  mbox_mi_get  (int bus,  int mbox)  {  return 0!= (mbox_sr_get(bus, mbox) | CAN_MSR_MMI);   }

//
// Reading the SR register clears MMI
// So implementing reads for   MRDY, MABT, MRTR, MDLC, and MTIMESTAMP   is probably unwise!
//
// There are MACROs in the header which will extract the fields for you. Namely:
//    MSR_MMI(sr), MSR_MRDY(sr), MSR_MABT(sr), MSR_MRTR(sr), MSR_MDLC(sr), MSR_TIMESTAMP(sr)
//
#if 0
	static inline  uint32_t  mbox_rdy_get (int bus,  int mbox)  {  return 0!= (mbox_sr_get(bus, mbox) | CAN_MSR_MRDY);  }
	static inline  uint32_t  mbox_abt_get (int bus,  int mbox)  {  return 0!= (mbox_sr_get(bus, mbox) | CAN_MSR_MABT);  }
	static inline  uint32_t  mbox_rtr_get (int bus,  int mbox)  {  return 0!= (mbox_sr_get(bus, mbox) | CAN_MSR_MRTR);  }

	//@=============================================================================
	static inline
	uint32_t  mbox_dlc_get (int bus,  int mbox)
	{
		return (mbox_sr_get(bus, mbox) & CAN_MSR_MDLC_Msk) >> CAN_MSR_MDLC_Pos;
	}

	//@=============================================================================
	static inline
	uint32_t  mbox_tstamp_get (int bus,  int mbox)
	{
		return (mbox_sr_get(bus, mbox) & CAN_MSR_MTIMESTAMP_Msk) >> CAN_MSR_MTIMESTAMP_Pos;
	}
#endif

//@============================================================================= ======================================= CR
//_pp1240-1241 : [-W] Message Control Register
static inline
bool  mbox_cr_set (int bus,  int mbox,  uint32_t mask,  uint32_t bits)
{
	return (MBOX(bus,mbox).CAN_MCR = (MBOX(bus,mbox).CAN_MCR & ~mask) | bits), true;
}

//@=============================================================================
static inline  bool  mbox_abort (int bus,  int mbox)  {  return mbox_cr_set(bus, mbox, CAN_MCR_MACR, CAN_MCR_MACR);  }

//@=============================================================================
// As far as activating Rx mode is concerned, Rx and RxO are the same
//
static inline  bool  mbox_trig  (int bus,  int mbox)  {  return mbox_cr_set(bus, mbox, CAN_MCR_MTCR, CAN_MCR_MTCR);  }
static inline  bool  mbox_rx    (int bus,  int mbox)  {  return mbox_trig(bus, mbox);  }
static inline  bool  mbox_rxo   (int bus,  int mbox)  {  return mbox_trig(bus, mbox);  }

//@=============================================================================
//_pp1204-1205
// With a Tx, you need to specify how many bytes {0..8} you are sending
//
static inline
bool  mbox_tx (int bus,  int mbox,  uint32_t dlc)
{
	return mbox_cr_set(bus, mbox,
	                   CAN_MCR_MTCR |                CAN_MCR_MDLC_Msk               ,
	                   CAN_MCR_MTCR | ((dlc << CAN_MCR_MDLC_Pos) & CAN_MCR_MDLC_Msk) );
}

//@=============================================================================
//_pp1205
// With a Tx, you need to specify how many bytes {0..8} you are sending
// RTR frames don't actually contain any data, JUST the length of the requested reply
//
static inline
bool  mbox_tx_rtr (int bus,  int mbox,  uint32_t dlc)
{
// ReadMe.txt : "Atmel SAM cannot generate RTR packets with DLC > 0"
#if 1  // This is what the CAN spec says we SHOULD do
	return mbox_cr_set(bus, mbox,
	                   CAN_MCR_MRTR | CAN_MCR_MTCR |                CAN_MCR_MDLC_Msk               ,
	                   CAN_MCR_MRTR | CAN_MCR_MTCR | ((dlc << CAN_MCR_MDLC_Pos) & CAN_MCR_MDLC_Msk) );

#else  // This is what the Atmel docs say is possible
	(void)dlc;
	return mbox_cr_set(bus, mbox,
	                   CAN_MCR_MRTR | CAN_MCR_MTCR,
	                   CAN_MCR_MRTR | CAN_MCR_MTCR );
#endif
}

//@=============================================================================
//_pp
// Consumer : ||: prepare RTR-frame ; send RTR-frame ; become Rx  ; wait for reply-frame  :||
// Producer : ||: prepare replyframe ; when RTR-frame received, auto-send reply-frame :||
//
// Consumer : ||: prepare thirst ; send tea-request ; become anticipant  ; wait for tea  :||
// Producer : ||: prepare tea ; when tea-request received, immeditately send tea :||
//
static inline  bool  mbox_con (int bus,  int mbox,  uint32_t dlc)  {  return mbox_tx_rtr(bus, mbox, dlc);  }
static inline  bool  mbox_pro (int bus,  int mbox,  uint32_t dlc)  {  return mbox_tx(bus, mbox, dlc);  }

//@=============================================================================
//_pp1201 : Receive (no Overwrite) - Lose new packets
static inline
bool  mbox_mode_rx (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se)
{
	return mbox_mode_set(bus, mbox, mid, mam, se, MBOX_RX);
}

//@=============================================================================
//_pp1202-1203 : Receive-with-Overwrite - Lose old packets
static inline
bool  mbox_mode_rxo (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se)
{
	return mbox_mode_set(bus, mbox, mid, mam, se, MBOX_RXO);
}

//@=============================================================================
//_pp1204-1205 : Transmit
//
// mbox_mode_tx( bus,  mbox,  MID,  MAM,  stdExt );
// mbox_md_set ( bus,  mbox,  u64 );
// mbox_tx     ( bus,  mbox,  dlc );
//
// mbox_mode_tx( bus,  mbox,  MID,  MAM,  stdExt );
// mbox_tx_rtr ( bus,  mode,  dlc );
//
static inline
bool  mbox_mode_tx (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se)
{
	return mbox_mode_set(bus, mbox, mid, mam, se, MBOX_TX);
}

//@=============================================================================
//_pp1206 : Producer
static inline
bool  mbox_mode_pro (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se)
{
	return mbox_mode_set(bus, mbox, mid, mam, se, MBOX_PRO);
}

//@=============================================================================
//_pp1207 : Consumer
static inline
bool  mbox_mode_con (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se)
{
	return mbox_mode_set(bus, mbox, mid, mam, se, MBOX_CON);
}

#endif // CAN_MBOX_SI_H_
