#ifndef CAN_INTR_SI_H_
#define CAN_INTR_SI_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_bus.h"

#define IRQN(bus)  ((IRQn_Type)digitalPinToInterrupt(_can[bus].irqn))

//@============================================================================= =======================================
//! "only external interrupts"??  http://www.engblaze.com/we-interrupt-this-program-to-bring-you-a-tutorial-on-arduino-interrupts/

///	static inline  void      intr_isr_attach (int bus,  isr_t isr      )  {  attachInterrupt (_can[bus].irqn, isr, HIGH);  }
///	static inline  void      intr_isr_detach (int bus                  )  {  detachInterrupt (_can[bus].irqn);             }

static inline  void      intr_pri_set    (int bus,  uint8_t  irqPri)  {  NVIC_SetPriority(_can[bus].irqn, irqPri);     }

///	static inline  void      intr_isr_attach (int bus,  isr_t isr      )  {  attachInterrupt (IRQN(bus), isr, HIGH);  }
///	static inline  void      intr_isr_detach (int bus                  )  {  detachInterrupt (IRQN(bus));             }
///
///	static inline  void      intr_pri_set    (int bus,  uint8_t  irqPri)  {  NVIC_SetPriority(IRQN(bus), irqPri);     }

//_pp1214-1219 - Interrupt Control {Enable/Disable/Mask}
static inline  bool      intr_enable     (int bus,  uint32_t mask  )  {  return (PCAN(bus)->CAN_IER = mask), true;     }
static inline  bool      intr_disable    (int bus,  uint32_t mask  )  {  return (PCAN(bus)->CAN_IDR = mask), true;     }
static inline  uint32_t  intr_get        (int bus,  uint32_t mask  )  {  return  PCAN(bus)->CAN_IMR & mask;            }

#endif // CAN_INTR_SI_H_
