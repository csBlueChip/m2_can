#include <stdint.h>

#include "can_bus.h"
#include "can_bus_si.h"
#include "can_baud.h"
#include "can_mbox.h"
#include "can_mbox_si.h"
#include "can_intr.h"
#include "can_intr_si.h"

//------------------------------------------------------------------------------ ---------------------------------------
// 0 terminated List of speeds to check for autobaud (if no list supplied)
//
static const  uint32_t  _autoBaudList[] = {
	CAN_BAUD_250K, CAN_BAUD_500K,  CAN_BAUD_1000K,
	CAN_BAUD_125K, CAN_BAUD_33333, CAN_BAUD_50K,   CAN_BAUD_800K,
	//CAN_BAUD_25K,  CAN_BAUD_10K,   CAN_BAUD_5K,
	CAN_BAUD_EOL
};

//------------------------------------------------------------------------------ ---------------------------------------
#define CAN_BAUD_DIV_MAX  (128)     // Max value for CAN Baudrate prescaler

#define CAN_TQ_MIN        (8)       // Min Time Quanta
#define CAN_TQ_MAX        (25)      // Max Time Quanta

#define CAN_BIT_SYNC      (1)       // Fixed bit time value
#define CAN_BIT_IPT       (2)       // ...

//------------------------------------------------------------------------------ ---------------------------------------
typedef
	struct canBitTiming {
		uint8_t  tq;      // CAN_BIT_SYNC + prog + phase1 + phase2 = tq, 8 <= tq <= 25.
		uint8_t  prog;    // Propagation segment, (3-bits + 1), 1~8;
		uint8_t  phase1;  // Phase segment 1, (3-bits + 1), 1~8;
		uint8_t  phase2;  // Phase segment 2, (3-bits + 1), 1~8, CAN_BIT_IPT <= uc_phase2;
		uint8_t  sjw;     // Resynchronization jump width, (2-bits + 1), min(uc_phase1, 4);
		uint8_t  sp;      // Sample point value, 0~100 in percent.
	}
canBitTiming_t;

// Values of bit time register for different baudrates, Sample point = ((1 + prog + phase1) / tq) * 100%.
static const  canBitTiming_t  _canBitTiming[] = {
   //TQ      PROG    PHASE1    PHASE2      SJW    SAMP
	{8,    (2 + 1),  (1 + 1),  (1 + 1),  (2 + 1),  75},
	{9,    (1 + 1),  (2 + 1),  (2 + 1),  (1 + 1),  67},
	{10,   (2 + 1),  (2 + 1),  (2 + 1),  (2 + 1),  70},
	{11,   (3 + 1),  (2 + 1),  (2 + 1),  (2 + 1),  72},
	{12,   (2 + 1),  (3 + 1),  (3 + 1),  (2 + 1),  67},
	{13,   (3 + 1),  (3 + 1),  (3 + 1),  (2 + 1),  77},
	{14,   (3 + 1),  (3 + 1),  (4 + 1),  (2 + 1),  64},
	{15,   (3 + 1),  (4 + 1),  (4 + 1),  (2 + 1),  67},
	{16,   (4 + 1),  (4 + 1),  (4 + 1),  (2 + 1),  69},
	{17,   (5 + 1),  (4 + 1),  (4 + 1),  (2 + 1),  71},
	{18,   (4 + 1),  (5 + 1),  (5 + 1),  (2 + 1),  67},
	{19,   (5 + 1),  (5 + 1),  (5 + 1),  (2 + 1),  68},
	{20,   (6 + 1),  (5 + 1),  (5 + 1),  (2 + 1),  70},
	{21,   (7 + 1),  (5 + 1),  (5 + 1),  (2 + 1),  71},
	{22,   (6 + 1),  (6 + 1),  (6 + 1),  (2 + 1),  68},
	{23,   (7 + 1),  (7 + 1),  (6 + 1),  (2 + 1),  70},
	{24,   (6 + 1),  (7 + 1),  (7 + 1),  (2 + 1),  67},
	{25,   (7 + 1),  (7 + 1),  (7 + 1),  (2 + 1),  68}
};

//+============================================================================= ======================================= BR
//_pp1223 : [RW] Baudrate
// RETN: 0  .. failed
//     : >0 .. active baudrate
//
uint32_t  can_baud_set (int bus,  uint32_t baud)
{
	int              tq;
	uint32_t         mod;
	canBitTiming_t*  pBT;

	// I can't think of an example where this will bite us
	// But if autobaud never returns, it's probably this line!
	if (baud == CAN_BAUD_AUTO)  return can_baud_auto(bus, NULL) ;

	// Problem if baud prescaler > maximum divisor  or  the SystemCoreClock is too slow
	if ( ( ((SystemCoreClock + ((baud * CAN_TQ_MAX) - 1)) / (baud * CAN_TQ_MAX)) > CAN_BAUD_DIV_MAX )
	     || (SystemCoreClock <  (baud * CAN_TQ_MIN))
	   )  return 0 ;

	// Find out the approximate Time Quantum according to the baudrate.
	// ...When the remainder is 0, we have the right TQ number.
	tq  = CAN_TQ_MIN;  // Minimum Time Quantum
	mod = 0xFFFFFFFF;  // Default = max value
	for (int i = CAN_TQ_MIN;  i <= CAN_TQ_MAX;  i++) {
		if ((SystemCoreClock / (baud * i)) <= CAN_BAUD_DIV_MAX) {
			uint32_t  thisMod = SystemCoreClock % (baud * i);

			if (thisMod < mod) {
				tq = i;
				if ( !(mod = thisMod) )  break ;
			}
		}
	}

	// Configure the CAN baudrate register
	can_pause(bus);
	pBT = (canBitTiming_t*)(&_canBitTiming[tq - CAN_TQ_MIN]);  // We are safe to cast off the const
	PCAN(bus)->CAN_BR = CAN_BR_PHASE2(pBT->phase2 - 1)
	                  | CAN_BR_PHASE1(pBT->phase1 - 1)
	                  | CAN_BR_PROPAG(pBT->prog   - 1)
	                  | CAN_BR_SJW   (pBT->sjw    - 1)
	                  | CAN_BR_BRP   (SystemCoreClock / (baud * tq) - 1) ;
	if (!can_run(bus, CAN_TIMEOUT))  return 0 ;

	return baud;
}

//+=============================================================================
// RETN: 0  .. failed
//     : >0 .. active baudrate
// NOTE: Destroys mbox[MBOX_CNT-1] and mbox[MBOX_CNT-2]
//
uint32_t  can_baud_auto (int bus,  uint32_t* list)
{
	static const int  box1 = MBOX_CNT - 1 ;
	static const int  box2 = MBOX_CNT - 2 ;

	int        i;
	uint32_t*  pSpd;
	uint32_t   intr = intr_get(bus, INTR_ALL);
	uint32_t   abm  = can_abm_get(bus);

	intr_disable(bus, INTR_ALL);
	can_abm_set(bus);

	// One of each mailbox {std, ext}
	mbox_mode_set(bus,box1, 0x00000000,0x00000000,MBOX_STD, MBOX_RX);
	mbox_mode_set(bus,box2, 0x00000000,0x00000000,MBOX_EXT, MBOX_RX);

	// Try each speed and see if we hear anything on the bus
	for (pSpd = list ? list : (uint32_t*)_autoBaudList;  *pSpd != CAN_BAUD_EOL;  pSpd++) {
		if ( !can_baud_set(bus, *pSpd) )  continue ;
		// Watch the mailboxes for a while to see if anything is received
		for (i = 0;  (i < 100) && !MSR_MRDY(mbox_sr_get(bus, box1)) && !MSR_MRDY(mbox_sr_get(bus, box2));  delay(6), i++) ;  //! why 100 * 6 = 600mS ??
		if (i < 100)  break ;  // If something was received, stop searching
	}

	mbox_disable(bus, box1);
	mbox_disable(bus, box2);

	if (!abm) can_abm_clr(bus) ;  // revert to saved mode (if appropriate)

	// Re-enable interrupts - except for the mailboxes we just destroyed
	intr_enable(bus, intr & ~(intr_mask_mbox[box1] | intr_mask_mbox[box2]) );

	return *pSpd;
}
