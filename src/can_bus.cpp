#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_bus.h"
#include "can_bus_si.h"
#include "can_baud.h"
#include "can_mbox_si.h"
#include "can_intr.h"
#include "can_isr.h"
#include "can_rb.h"
#include "can_rb_si.h"

//------------------------------------------------------------------------------ ---------------------------------------
canBus_t  _can[CAN_CNT];

//+============================================================================= =======================================
void  can_init_m2 (void)
{
	memset(_can, 0, sizeof(_can));

	_can[0].pCan   = CAN0;
	_can[0].enPin  = CAN0_EN;
	_can[0].irqn   = CAN0_IRQn;
	_can[0].irqPri = 12;  // What is the range on this?
///	_can[0].isr    = intr_isr_can0;
	_can[0].baud   = 0x00000000;
	_can[0].PID    = ID_CAN0;

	_can[1].pCan   = CAN1;
	_can[1].enPin  = CAN1_EN;
	_can[1].irqn   = CAN1_IRQn;
	_can[1].irqPri = 12;  // What is the range on this?
///	_can[1].isr    = intr_isr_can1;
	_can[1].baud   = 0x00000000;
	_can[1].PID    = ID_CAN1;


//int i = 0;
	for (int bus = 0;  bus < CAN_CNT;  bus++) {
		can_disable_m2(bus);  // "Have you switched it off and on again?"
		delay(5);             // ...
		can_enable_m2(bus);   // ...
//SerialUSB.println(i++);
//!		intr_init(bus);       // Set up the Interrupts & Interrupt Service Routines
//SerialUSB.println(i++);
		can_abm_set(bus);     // Engage Listen/AutoBaud mode - just to be on the safe side (no Frame ack'ing)
		rb_init(bus);         // Initialise all ring buffers
		mbox_init(bus);       // Initialise all mailboxes
	}
}

///	//+============================================================================= =======================================
///	uint32_t  can_setup (int bus,  uint32_t baud)
///	{
///		if (baud == CAN_BAUD_AUTO)  return (_can[bus].baud = can_baud_auto(bus, NULL)) ;
///		else                        return (_can[bus].baud = can_baud_set(bus, baud)) ;
///	}

//+============================================================================= =======================================
//_pp38 -  A peripheral identifier is required for 
//           # control of the peripheral interrupt with the Nested Vectored Interrupt Controller  [NVIC]
//           # control of the peripheral   clock   with the      Power Management     Controller  [PMC]
//
// Physical device
//
void  can_enable_m2 (int bus)
{
	pmc_enable_periph_clk(_can[bus].PID);
	pinMode(_can[bus].enPin, OUTPUT);
	digitalWrite(_can[bus].enPin, HIGH);
}

//+=============================================================================
void  can_disable_m2 (int bus)
{
	digitalWrite(_can[bus].enPin, LOW);
	pmc_disable_periph_clk(_can[bus].PID);
}

//+============================================================================= =======================================
// Logcal device
//
bool  can_run (int bus,  uint32_t timeout)
{
	can_en_set(bus);

	if (!timeout)  return true ;

	// Poll the Status Register until the bus announces WAKEUP
	// I realise a software timing loop for the timeout is ugly,
	// but this code is for a specific use case [Macchina M2]
	// If you port this code, I suggest you do this with hardware timers
	for (uint32_t i = 0;  i < timeout;  i++)
		if (can_sr_wakeup(bus))  return true;
	return false;
}

//+=============================================================================
// static inilne  can_pause()

//+============================================================================= =======================================
// Virtual device
//
bool  can_start (int bus)
{
	can_enable_m2(bus);
	return can_run(bus, CAN_TIMEOUT);
}

//+=============================================================================
void  can_stop (int bus)
{
	can_pause(bus);
	can_disable_m2(bus);
}

//+=============================================================================
// https://stackoverflow.com/questions/6160963/how-can-i-digitalread-a-pin-that-is-in-pinmode-output
// https://stackoverflow.com/a/39859147
// bool value = (0!=(*portOutputRegister( digitalPinToPort(pin) ) & digitalPinToBitMask(pin)));
// hmmm, apparently digitalRead() works !?
//
bool  can_isRunning (int bus)
{
	return (can_en_get(bus) && (digitalRead(_can[bus].enPin) == HIGH)) ? true : false ;
//	return can_en_get(bus) ? true : false ;
}

//+============================================================================= =======================================
bool  can_mr_set (int bus,  uint32_t bits)
{
	if (can_isRunning(bus))  {  can_pause(bus);  PCAN(bus)->CAN_MR |= bits;  can_run(bus, 0);  }
	else                     {                   PCAN(bus)->CAN_MR |= bits;                    }

	return true;
}

//+=============================================================================
bool  can_mr_clr (int bus,  uint32_t bits)
{
	if (can_isRunning(bus))  {  can_pause(bus);  PCAN(bus)->CAN_MR &= ~bits;  can_run(bus, 0);  }
	else                     {                   PCAN(bus)->CAN_MR &= ~bits;                    }

	return true;
}
