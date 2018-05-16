#ifndef CAN_MBOX_H_
#define CAN_MBOX_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

//------------------------------------------------------------------------------ ---------------------------------------
// We need to set MBOX_CNT *before* we include can_bus.h
#define MBOX_CNT  (CANMB_NUMBER)
#include "can_bus.h"

//------------------------------------------------------------------------------ ---------------------------------------
#define MBOX(bus,mbox)  PCAN(bus)->CAN_MB[mbox]

typedef
	enum mid {
		MBOX_STD = 0,
		MBOX_EXT = CAN_MID_MIDE,
	}
mid_t;

typedef
	enum mot {
		MBOX_DIS = CAN_MMR_MOT_MB_DISABLED,
		MBOX_RXO = CAN_MMR_MOT_MB_RX_OVERWRITE,
		MBOX_RX  = CAN_MMR_MOT_MB_RX,
		MBOX_CON = CAN_MMR_MOT_MB_CONSUMER,
		MBOX_TX  = CAN_MMR_MOT_MB_TX,
		MBOX_PRO = CAN_MMR_MOT_MB_PRODUCER,
	}
mot_t;

//------------------------------------------------------------------------------ ---------------------------------------
// See mbox_sr_get() comments about these macros
//
#define MSR_MMI(sr)        (0!= ((sr & CAN_MSR_MMI           )                          ))
#define MSR_MRDY(sr)       (0!= ((sr & CAN_MSR_MRDY          )                          ))
#define MSR_MABT(sr)       (0!= ((sr & CAN_MSR_MABT          )                          ))
#define MSR_MRTR(sr)       (0!= ((sr & CAN_MSR_MRTR          )                          ))
#define MSR_MDLC(sr)       (    ((sr & CAN_MSR_MDLC_Msk      ) >> CAN_MSR_MDLC_Pos      ))
#define MSR_TIMESTAMP(sr)  (    ((sr & CAN_MSR_MTIMESTAMP_Msk) >> CAN_MSR_MTIMESTAMP_Pos))

//============================================================================== =======================================
bool      mbox_reset     (int bus,  int mbox) ;
bool      mbox_reset_all (int bus) ;
uint32_t  mbox_mam_get   (int bus,  int mbox,  mid_t* se) ;
uint32_t  mbox_mid_get   (int bus,  int mbox,  mid_t* se) ;
bool      mbox_mode_set  (int bus,  int mbox,  uint32_t mid,  uint32_t mam,  mid_t se,  mot_t mot) ;

#endif // CAN_MBOX_H_
