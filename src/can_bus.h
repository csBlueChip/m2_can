// This is an ugly little hack which allows you to compile the library independently of any actual code
// Useful for testing compilation during code development
// The linker will belch like nobodies busniess, but it allows to code to compile
#if 0
#	include "fudge/fudge.h"
#endif

#ifndef CAN_BUS_H_
#define CAN_BUS_H_

//C:\Users\Username\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\system\CMSIS\Device\ATMEL\sam3xa\include\component\component_can.h

#include <Arduino.h>

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_rb.h"
#include "can_mbox.h"
#include "can_intr.h"

//------------------------------------------------------------------------------ ---------------------------------------
// Look what I just pulled out of my arse...
#define CAN0_EN  (50)
#define CAN1_EN  (48)

//------------------------------------------------------------------------------ ---------------------------------------
#define CAN_CNT  (2)

typedef
	struct canErr {
		int  rb;   // Ring buffer full
		int  mmi;  // MMI : Lost message
	}
canErr_t;


typedef
	struct canBus {
		volatile Can*  pCan;
		int            enPin;
		IRQn_Type      irqn;
		uint8_t        irqPri;
		uint32_t       PID;
		uint32_t       baud;
		ringBuffer_t   rb[MBOX_CNT];
///		isr_t          isr;
		canErr_t       canErr;
		canErr_t       mboxErr[MBOX_CNT];
	}
canBus_t;

extern canBus_t  _can[];

#define PCAN(bus)       _can[bus].pCan
#define MERR(bus,mbox)  _can[bus].mboxErr[mbox]

//============================================================================== =======================================
void      can_init_m2    (void) ;

void      can_enable_m2  (int bus) ;
void      can_disable_m2 (int bus) ;

bool      can_run        (int bus,  uint32_t timeout) ;
//void    can_pause      (int bus) ;

bool      can_start      (int bus) ;
void      can_stop       (int bus) ;

bool      can_isRunning  (int bus) ;

bool      can_mr_set     (int bus,  uint32_t bits) ;
bool      can_mr_clr     (int bus,  uint32_t bits) ;

#endif // CAN_BUS_H_
