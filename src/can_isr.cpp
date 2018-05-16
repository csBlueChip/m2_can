#include <stdint.h>

#include "can_bus_si.h"
#include "can_data.h"
#include "can_mbox.h"
#include "can_mbox_si.h"
#include "can_mbox_data_si.h"
#include "can_rb_si.h"

//+============================================================================= =======================================
static inline
void  isr_mbox_pro (int bus,  int mbox,  uint32_t sr)
{
	(void)bus; (void)mbox;  (void)sr;
}

//+============================================================================= =======================================
static inline
void  isr_mbox_tx (int bus,  int mbox,  uint32_t sr)
{
	(void)bus; (void)mbox;  (void)sr;
}

//+============================================================================= =======================================
static inline
void _isr_rx_getFrame (canFrame_t* frame,  int bus,  int mbox,  uint32_t sr)
{
	mid_t  se;

	frame->aid = mbox_mid_get(bus, mbox, &se);
	frame->ext = (0!=se);
	frame->fid = mbox_fid_get(bus, mbox);

	frame->dlc          = MSR_MDLC(sr);
	if (   !(frame->rtr = MSR_MRTR(sr)) ) {
		frame->data.mdh = mbox_mdh_get(bus, mbox);
		frame->data.mdl = mbox_mdl_get(bus, mbox);
	}

//!	frame->ts  = time thingy;
//	frame->pri = mbox_getmeaningless for Rx

	// internal variables
	frame->bus = bus;
	frame->dir = CDIR_RX;
}

//+=============================================================================
static inline
void  isr_mbox_rxo (int bus,  int mbox,  uint32_t sr)
{
	canFrame_t*  frame = rb_frame_blank(bus, mbox);  // reserve a slot in the ring buffer

	if (!frame) {  // No space in ring buffer?
		MERR(bus,mbox).rb++;

	} else {
		for(;;) {
			_isr_rx_getFrame(frame, bus, mbox, sr);
			// IF packets are arriving faster than this loop can iterate - this WILL hang the system!!
			// So: You have 47uS to get in and out of this loop!
			// ...because that's how long it takes to Tx the smallest frame @ 1Mb/S [qv. readme.txt]
			if (!mbox_mi_get(bus, mbox)) break ;  // Mailbox did NOT Rx another frame since we started - we're done
			MERR(bus,mbox).mmi++ ;                // Mailbox   DID   Rx another frame - flag the error, and try again!
		}
	}

	// Restart the mailbox
	mbox_rx(bus, mbox);
}

//+=============================================================================
static inline
void  isr_mbox_con (int bus,  int mbox,  uint32_t sr)
{
	canFrame_t*  frame = rb_frame_blank(bus, mbox);  // reserve a slot in the ring buffer

	if (!frame)  MERR(bus,mbox).rb++ ;               // No space in ring buffer?
	else         _isr_rx_getFrame(frame, bus, mbox, sr);

	// Check for missed-frames ...This will, realistically, only ever happen once at most
//!	while (mbox_mi_get(bus, mbox))  MERR(bus,mbox).mmi++ ;
}

//+=============================================================================
// An Rx Mailbox is (in this respect) like a Consumer, but it automatically restarts
//
static inline
void  isr_mbox_rx (int bus,  int mbox,  uint32_t sr)
{
	isr_mbox_con(bus, mbox, sr);

	// Restart the mailbox
	mbox_rx(bus, mbox);
}

//+============================================================================= =======================================
void  isr_mbox (int bus,  int mbox)
{
	register uint32_t  mboxSR = mbox_sr_get(bus, mbox);   // Remember, this will clear MMI

	// Check for missed-frames
	if (MSR_MMI(mboxSR))  MERR(bus,mbox).mmi++ ;

	switch (mbox_mot_get(bus, mbox)) {
		case CAN_MMR_MOT_MB_RX_OVERWRITE :  isr_mbox_rxo(bus, mbox, mboxSR);  return ;
		case CAN_MMR_MOT_MB_RX           :  isr_mbox_rx( bus, mbox, mboxSR);  return ;
		case CAN_MMR_MOT_MB_CONSUMER     :  isr_mbox_con(bus, mbox, mboxSR);  return ;

		case CAN_MMR_MOT_MB_TX           :  isr_mbox_tx( bus, mbox, mboxSR);  return ;
		case CAN_MMR_MOT_MB_PRODUCER     :  isr_mbox_pro(bus, mbox, mboxSR);  return ;
	}
}

//+============================================================================= =======================================
void  isr_x (int bus)  {  (void)bus;  }  //! placeholder for unimplemented interrupts

//+============================================================================= =======================================
// Our secondary Interrupt Service Routine is a Dispatcher
//_pp1218,1222 : OVLSY, TBSY, and RBSY are not interrupt events
// Nested Interrupts: https://stackoverflow.com/questions/24128926/what-happens-when-an-isr-is-running-and-another-interrupt-happens
//
void  intr_isr  (int bus)
{
	uint32_t  canSR = can_sr_get(bus);

//SerialUSB.print("<");
//SerialUSB.print(bus);
//SerialUSB.print("=");
//SerialUSB.print(canSR, HEX);
//SerialUSB.print("> ");

	if (canSR & CAN_SR_MB0   )  isr_mbox(bus, 0) ;  // Mailbox 0 Event
	if (canSR & CAN_SR_MB1   )  isr_mbox(bus, 1) ;  // Mailbox 1 Event
	if (canSR & CAN_SR_MB2   )  isr_mbox(bus, 2) ;  // Mailbox 2 Event
	if (canSR & CAN_SR_MB3   )  isr_mbox(bus, 3) ;  // Mailbox 3 Event
	if (canSR & CAN_SR_MB4   )  isr_mbox(bus, 4) ;  // Mailbox 4 Event
	if (canSR & CAN_SR_MB5   )  isr_mbox(bus, 5) ;  // Mailbox 5 Event
	if (canSR & CAN_SR_MB6   )  isr_mbox(bus, 6) ;  // Mailbox 6 Event
	if (canSR & CAN_SR_MB7   )  isr_mbox(bus, 7) ;  // Mailbox 7 Event

	if (canSR & CAN_SR_ERRA  )  isr_x(bus) ;  // Error Active Mode
	if (canSR & CAN_SR_WARN  )  isr_x(bus) ;  // Warning Limit
	if (canSR & CAN_SR_ERRP  )  isr_x(bus) ;  // Error Passive Mode
	if (canSR & CAN_SR_BOFF  )  isr_x(bus) ;  // Bus Off Mode
	if (canSR & CAN_SR_SLEEP )  isr_x(bus) ;  // CAN controller in Low power Mode
	if (canSR & CAN_SR_WAKEUP)  isr_x(bus) ;  // CAN controller is not in Low power Mode
	if (canSR & CAN_SR_TOVF  )  isr_x(bus) ;  // Timer Overflow
	if (canSR & CAN_SR_TSTP  )  isr_x(bus) ;  // Timestamp Interrupt Mask

	if (canSR & CAN_SR_CERR  )  isr_x(bus) ;  // Mailbox CRC Error
	if (canSR & CAN_SR_SERR  )  isr_x(bus) ;  // Mailbox Stuffing Error
	if (canSR & CAN_SR_AERR  )  isr_x(bus) ;  // Acknowledgment Error
	if (canSR & CAN_SR_FERR  )  isr_x(bus) ;  // Form Error
	if (canSR & CAN_SR_BERR  )  isr_x(bus) ;  // Bit Error

}

//+=============================================================================
// These functions are protoyped in
//   C:\Users\Jesus\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\system\CMSIS\Device\ATMEL\sam3xa\include\sam3x8e.h
// These are what will get called in the event of an interrupt
//
void  CAN0_Handler (void)  {  intr_isr(0);  }
void  CAN1_Handler (void)  {  intr_isr(1);  }
