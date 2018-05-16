#ifndef CAN_BAUD_H_
#define CAN_BAUD_H_

#include <stdint.h>

#include "can_bus.h"

//----------------------------------------------------------------------------------------------------------------------
// Do NOT use the baud rates specified in
//   C:\Users\Username\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\system\libsam\include\can.h
// ...these as multiple of 1000, meaning we can't do baud rates such as 3333
// For sanity I have undef'd them!
//
#ifdef CAN_BPS_500K
#   undef CAN_BPS_1000K
#   undef CAN_BPS_800K
#   undef CAN_BPS_500K
#   undef CAN_BPS_250K
#   undef CAN_BPS_125K
#   undef CAN_BPS_50K
#   undef CAN_BPS_25K
#   undef CAN_BPS_10K
#   undef CAN_BPS_5K
#endif

#define CAN_BAUD_1000K  (1000000ul)
#define CAN_BAUD_800K   (800000ul)
#define CAN_BAUD_500K   (500000ul)
#define CAN_BAUD_250K   (250000ul)
#define CAN_BAUD_125K   (125000ul)
#define CAN_BAUD_50K    (50000ul)
#define CAN_BAUD_33333  (33333ul)
#define CAN_BAUD_25K    (25000ul)
#define CAN_BAUD_10K    (10000ul)
#define CAN_BAUD_5K     (5000ul)

#define CAN_BAUD_AUTO   (1ul)
#define CAN_BAUD_EOL    (0ul)

//----------------------------------------------------------------------------------------------------------------------
#define CAN_TIMEOUT       (100000)  // Synch timeout

//============================================================================== =======================================
uint32_t  can_baud_set  (int bus,  uint32_t  baud) ;
uint32_t  can_baud_auto (int bus,  uint32_t* list) ;

//+============================================================================= =======================================
static inline  uint32_t  can_baud_get (int bus)  {  return _can[bus].baud;  }

#endif // CAN_BAUD_H_
