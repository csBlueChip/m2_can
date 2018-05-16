#ifndef CAN_MBOX_DATA_H_
#define CAN_MBOX_DATA_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_mbox.h"

//============================================================================== =======================================
bool  mbox_byte_set (int bus,  int mbox,  int pos,  uint8_t u8) ;

#endif // CAN_MBOX_DATA_H_
