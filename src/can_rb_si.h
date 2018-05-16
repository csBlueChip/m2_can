#ifndef CAN_RB_SI_H_
#define CAN_RB_SI_H_

#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_rb.h"

//@============================================================================= =======================================
static inline  void  rb_init    (int bus)           {  memset(_can[bus].rb, 0, sizeof(_can[bus].rb));  }
static inline  int   rb_len_get (int bus,  int rb)  {  return _can[bus].rb[rb].len;  }

//@============================================================================= =======================================
static inline
bool  rb_frame_add (int bus,  int rb,  const canFrame_t* frame)
{
	return _rb_frame_add(bus, rb, frame) ? true : false ;
}

//@=============================================================================
static inline
canFrame_t*  rb_frame_blank (int bus,  int rb)
{
	return _rb_frame_add(bus, rb, NULL);
}


#endif // CAN_RB_SI_H_
