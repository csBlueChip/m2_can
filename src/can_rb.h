#ifndef CAN_RB_H_
#define CAN_RB_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE
#include <string.h>

#include "can_data.h"

//------------------------------------------------------------------------------ ---------------------------------------
typedef
	enum canDir {
		CDIR_RX = 0,
		CDIR_TX = 1,
	}
canDir_t;

typedef
  struct canFrame {
    // Defined in a strange order to help the compiler optimise data storage
    uint32_t   aid;   // Arbitration ID
    uint32_t   fid;   // Family ID
    canData_t  data;  // Payload
    canTime_t  ts;    // Timestamp
    uint8_t    dlc;   // Length {0..8}
    uint8_t    pri;   // Priority {0..15} - Only used when transmitting frames
    bool       ext;   // Extended
    bool       rtr;   // Remote Tranmit Request
    uint8_t    bus;   // CAN Bus number   - Used by data logging systems
    canDir_t   dir;   // CDIR_RX, CDIR_TX - Used by data logging systems
  }
canFrame_t;


//------------------------------------------------------------------------------ ---------------------------------------
// A ring buffer can hold, at most, n-1 elements, because 'head' points at the next available slot
// So if n==2, your buffer can hold  a maximum of 1 element. And you're probably better off using another method!
// Here, all values of n are a power of 2 (for later maths efficiency)
// The integer value of the RBSIZ_N is n-1 (ready to be used as a modulo-mask)
typedef
	enum rbSize {
		RBSIZ_0   = 0,             // Disabled
		RBSIZ_2   = (1 << 1) - 1,  // Max elements =   1 ;  in array positions {0..  1}
		RBSIZ_4   = (1 << 2) - 1,  // Max elements =   3 ;  in array positions {0..  3}
		RBSIZ_8   = (1 << 3) - 1,  // Max elements =   7 ;  in array positions {0..  7}
		RBSIZ_16  = (1 << 4) - 1,  // Max elements =  15 ;  in array positions {0.. 15}
		RBSIZ_32  = (1 << 5) - 1,  // Max elements =  31 ;  in array positions {0.. 31}
		RBSIZ_64  = (1 << 6) - 1,  // Max elements =  63 ;  in array positions {0.. 63}
		RBSIZ_128 = (1 << 7) - 1,  // Max elements = 127 ;  in array positions {0..127}
	}
rbSize_t;

#define RBSIZ_DISABLE (RBSIZ_0)   // 0 is used to mean 'disabled'
#define RBSIZ_DEFAULT (RBSIZ_32)  // 0 is used to mean 'disabled'

typedef
	struct ringBuffer {
		canFrame_t*   frame;
		unsigned int  head;
		unsigned int  tail;
		rbSize_t      size;
		unsigned int  len;
	}
ringBuffer_t;

#define RB_CNT  (MBOX_CNT)

//------------------------------------------------------------------------------ ---------------------------------------
// We need to define ringBuffer_t before we include can_bus.h
#include "can_bus.h"
#include "can_mbox.h"

//============================================================================== =======================================
bool         rb_reset      (int bus,  int rb) ;
bool         rb_reset_all  (int bus) ;
bool         rb_set        (int bus,  int rb,  rbSize_t          size) ;
bool         rb_frame_get  (int bus,  int rb,  canFrame_t*       frame) ;
canFrame_t*  _rb_frame_add (int bus,  int rb,  const canFrame_t* frame) ;

#endif // CAN_RB_H_
