#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_mbox.h"
#include "can_mbox_data_si.h"

//+============================================================================= =======================================
// This is a rather expensive way to fill in a full complement of bytes
// You're much better off building your data in a canbusData_t and calling mbox_md_set(myCanData.md)
// But I can imagine this might be useful in Producer mode
//
void  mbox_byte_set (int bus,  int mbox,  int pos,  uint8_t u8)
{
	int     shift = (pos & 0x03) * 8;
	RwReg*  pReg  = (pos & 0x04) ? &(MBOX(bus,mbox).CAN_MDH)
	                             : &(MBOX(bus,mbox).CAN_MDL) ;

	_mbox_mdx_set(pReg, (_mbox_mdx_get(pReg) & ~(0xFFl << shift)) | (((uint32_t)u8) << shift));
}
