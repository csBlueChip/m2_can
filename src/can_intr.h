#ifndef CAN_INTR_H_
#define CAN_INTR_H_

#include "can_bus.h"

//----------------------------------------------------------------------------------------------------------------------
//_pp1214
#define CAN_IER_ALL \
	( CAN_IER_MB0    | CAN_IER_MB1    | CAN_IER_MB2    | CAN_IER_MB3    | CAN_IER_MB4    | \
	  CAN_IER_MB5    | CAN_IER_MB6    | CAN_IER_MB7    | CAN_IER_ERRA   | CAN_IER_WARN   | \
	  CAN_IER_ERRP   | CAN_IER_BOFF   | CAN_IER_SLEEP  | CAN_IER_WAKEUP | CAN_IER_TOVF   | \
	  CAN_IER_TSTP   | CAN_IER_CERR   | CAN_IER_SERR   | CAN_IER_AERR   | CAN_IER_FERR   | \
	  CAN_IER_BERR )

//_pp1216
#define CAN_IDR_ALL \
	( CAN_IDR_MB0    | CAN_IDR_MB1    | CAN_IDR_MB2    | CAN_IDR_MB3    | CAN_IDR_MB4    | \
	  CAN_IDR_MB5    | CAN_IDR_MB6    | CAN_IDR_MB7    | CAN_IDR_ERRA   | CAN_IDR_WARN   | \
	  CAN_IDR_ERRP   | CAN_IDR_BOFF   | CAN_IDR_SLEEP  | CAN_IDR_WAKEUP | CAN_IDR_TOVF   | \
	  CAN_IDR_TSTP   | CAN_IDR_CERR   | CAN_IDR_SERR   | CAN_IDR_AERR   | CAN_IDR_FERR   | \
	  CAN_IDR_BERR )

//_pp1218
#define CAN_IMR_ALL \
	( CAN_IMR_MB0    | CAN_IMR_MB1    | CAN_IMR_MB2    | CAN_IMR_MB3    | CAN_IMR_MB4    | \
	  CAN_IMR_MB5    | CAN_IMR_MB6    | CAN_IMR_MB7    | CAN_IMR_ERRA   | CAN_IMR_WARN   | \
	  CAN_IMR_ERRP   | CAN_IMR_BOFF   | CAN_IMR_SLEEP  | CAN_IMR_WAKEUP | CAN_IMR_TOVF   | \
	  CAN_IMR_TSTP   | CAN_IMR_CERR   | CAN_IMR_SERR   | CAN_IMR_AERR   | CAN_IMR_FERR   | \
	  CAN_IMR_BERR )

#define INTR_ALL (CAN_IER_ALL)

extern  const  uint32_t  intr_mask_mbox[];

//============================================================================== =======================================
void  intr_init     (int bus) ;
void  intr_STOP     (int bus) ;

#endif // CAN_INTR_H_
