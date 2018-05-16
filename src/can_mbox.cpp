#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_bus.h"
#include "can_mbox.h"
#include "can_mbox_si.h"
#include "can_rb.h"
#include "can_intr.h"
#include "can_intr_si.h"

//+============================================================================= =======================================
bool  mbox_reset (int bus,  int mbox)
{
	if ( !intr_disable(bus, intr_mask_mbox[mbox]) )  return false ;
	if ( !mbox_abort(  bus, mbox                ) )  return false ;
	if ( !mbox_mot_set(bus, mbox, MBOX_DIS      ) )  return false ;
	rb_reset(bus, mbox);

	return true;
}

//+=============================================================================
bool  mbox_reset_all (int bus)
{
	bool  rv = true;  // presume success

	for (int  mbox = 0;  mbox < MBOX_CNT;  mbox++)
		if ( !mbox_reset(bus, mbox) )  rv = false ;  // Flag a failure, but carry on!

	return rv;
}

//+============================================================================= ======================================= MAM
//_pp1232 : [RW] Message Acceptance Mask
uint32_t  mbox_mam_get (int bus,  int mbox,  mid_t* se)
{
	uint32_t  mam = MBOX(bus,mbox).CAN_MAM;

	if (mam & CAN_MAM_MIDE) {
		if (se)  *se = MBOX_EXT ;
		return mam & (CAN_MAM_MIDvA_Msk | CAN_MAM_MIDvB_Msk);

	} else {
		if (se)  *se = MBOX_STD ;
		return (mam & CAN_MAM_MIDvA_Msk) >> CAN_MAM_MIDvA_Pos;
	}
}

//+============================================================================= ======================================= MID
//_pp1233 : [RW] Message ID
uint32_t  mbox_mid_get (int bus,  int mbox,  mid_t* se)
{
	uint32_t  mid = MBOX(bus,mbox).CAN_MID;

	if (mid & CAN_MID_MIDE) {
		if (se)  *se = MBOX_EXT ;
		return mid & (CAN_MID_MIDvA_Msk | CAN_MID_MIDvB_Msk);

	} else {
		if (se)  *se = MBOX_STD ;
		return (mid & CAN_MID_MIDvA_Msk) >> CAN_MID_MIDvA_Pos;
	}
}

//+============================================================================= =======================================
//_pp1201 : "Message ID and Message Acceptance Mask must be set before the Receive Mode is enabled"
//
bool  mbox_mode_set (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se,  mot_t mot)
{
	if ( !mbox_reset(  bus, mbox                ) )  return false ;  // ** first
	if ( !mbox_mid_set(bus, mbox, mid, se       ) )  return false ;  // ** }
	if ( !mbox_mam_set(bus, mbox, mam, se       ) )  return false ;  // ** } any order
	rb_reset(bus, mbox);                                             // ** }
	if ( !intr_enable(bus, intr_mask_mbox[mbox])  )  return false ;  // ** penultimate
	return mbox_mot_set(bus, mbox, mot);                             // ** last
}
