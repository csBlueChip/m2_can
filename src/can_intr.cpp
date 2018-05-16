#include <stdint.h>

#include "can_bus.h"
#include "can_bus_si.h"
#include "can_intr.h"
#include "can_intr_si.h"
#include "can_isr.h"

//----------------------------------------------------------------------------------------------------------------------
// intr_mask_mbox[MBOX_CNT] == ALL mailboxes
const  uint32_t  intr_mask_mbox[MBOX_CNT + 1] = {
	CAN_IER_MB0, CAN_IER_MB1, CAN_IER_MB2, CAN_IER_MB3,
	CAN_IER_MB4, CAN_IER_MB5, CAN_IER_MB6, CAN_IER_MB7,
	(CAN_IER_MB0 | CAN_IER_MB1 | CAN_IER_MB2 | CAN_IER_MB3 | CAN_IER_MB4 | CAN_IER_MB5 | CAN_IER_MB6 | CAN_IER_MB7)
};

//+============================================================================= =======================================
void  intr_init (int bus)
{
	intr_STOP(bus);                                  // Disable all interrupts - user will enable required interrupts
	intr_pri_set(bus, _can[bus].irqPri);             // Set the intr priority
///	intr_isr_attach(_can[bus].irqn, _can[bus].isr);  // Attach the ISR
}

//+============================================================================= =======================================
void  intr_STOP (int bus)
{
	intr_disable(bus, INTR_ALL);      // Disable all interrupts
///	intr_isr_detach(_can[bus].irqn);  // Detach the ISR
}
