#ifndef CAN_MBOX_DATA_SI_H_
#define CAN_MBOX_DATA_SI_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

//============================================================================== =======================================
//__pp1238
// The CAN bus data field (and only the data field) is little-endian
// Bytes are received/sent on the bus in the following order:
//     1. d7 : CAN_MDL[7:0]
//     2. d6 : CAN_MDL[15:8]
//     3. d5 : CAN_MDL[23:16]
//     4. d4 : CAN_MDL[31:24]
//
//     5. d3 : CAN_MDH[7:0]
//     6. d2 : CAN_MDH[15:8]
//     7. d1 : CAN_MDH[23:16]
//     8. d0 : CAN_MDH[31:24]
//
//  ,--------.-------------------------------.----------.---------------------------------------.
//  | 1 x 64 |              MD               | uint64_t | mbox_md_{set/get}                     | -> mdh, mdl
//  |        |-------------------------------|          |                                       |
//  | 2 x 32 |      MDH      |      MDL      | uint32_t | mbox_{mdh/mdl}_{set/get}              | -> mdx
//  |        |-------------------------------|          |                                       |
//  | 4 x 16 | MDH(h)| MDH(l)| MDL(h)| MDL(l)| uint16_t | mbox_{mdhh/mdhl/ mdhh/mdhl}_{set/get} | -> mdxh, mdxl -> mdx
//  |        |-------------------------------|          |                                       |
//  | 8 x  8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | uint8_t  | mbox_byte_{set/get}                   | ->
//  `--------^-------------------------------^----------^---------------------------------------'
//

//------------------------------------------------------------------------------ --------------------------------------- ---------------------------------------
// Yes. I realise this is HORRIFIC abuse of line length
// But it turns a 200-line file in to a 1-screen file, albeit with some a horizontal scroll
// And makes for FAR more maintainble code
//

//@============================================================================= ======================================= =======================================
// Technically, I guess we should be checking MRDY before reading/writing the mdx registers
// But after much consideration, I decided against it, because it caused too much trouble with error handling
//   and all the error handling would be to protect the driver from bad code
// ...If you're writing bad code, use an n00b-friendly library instead!
//
static inline  void      _mbox_mdx_set  (RwReg* pReg,  uint32_t  u32)       {         *pReg = u32;  }
static inline  uint32_t  _mbox_mdx_get  (RwReg* pReg)                       {  return *pReg;        }

//@============================================================================= ======================================= =======================================

static inline  void      mbox_mdl_set  (int bus,  int mbox,  uint16_t u32)   {         _mbox_mdx_set(&(MBOX(bus,mbox).CAN_MDL), u32);  }
static inline  void      mbox_mdh_set  (int bus,  int mbox,  uint16_t u32)   {         _mbox_mdx_set(&(MBOX(bus,mbox).CAN_MDH), u32);  }

static inline  uint32_t  mbox_mdl_get  (int bus,  int mbox)                  {  return _mbox_mdx_get(&(MBOX(bus,mbox).CAN_MDL));       }
static inline  uint32_t  mbox_mdh_get  (int bus,  int mbox)                  {  return _mbox_mdx_get(&(MBOX(bus,mbox).CAN_MDH));       }

//@============================================================================= ======================================= =======================================

static inline  void      mbox_md_set   (int bus,  int mbox,  uint64_t u64)   {  mbox_mdl_set(bus, mbox, (uint32_t) (u64 & 0x00000000FFFFFFFF)       );
                                                                                mbox_mdh_set(bus, mbox, (uint32_t)((u64 & 0xFFFFFFFF00000000) >> 32));  }

static inline  uint64_t  mbox_md_get   (int bus,  int mbox)                  {  return (mbox_mdh_get(bus, mbox) << 16) | mbox_mdl_get(bus, mbox);       }

//@============================================================================= ======================================= =======================================

static inline  void      _mbox_mdxl_set (RwReg* pReg,  uint16_t u16)         {  _mbox_mdx_set(pReg, (_mbox_mdx_get(pReg) & 0xFFFF0000) |  u16       );  }
static inline  void      _mbox_mdxh_set (RwReg* pReg,  uint16_t u16)         {  _mbox_mdx_set(pReg, (_mbox_mdx_get(pReg) & 0x0000FFFF) | (u16 << 16));  }

static inline  uint16_t  _mbox_mdxl_get (RwReg* pReg)                        {  return              (_mbox_mdx_get(pReg) & 0x0000FFFF);                 }
static inline  uint16_t  _mbox_mdxh_get (RwReg* pReg)                        {  return              (_mbox_mdx_get(pReg) & 0xFFFF0000) >> 16;           }

static inline  void      mbox_mdll_set  (int bus,  int mbox,  uint16_t u16)  {         _mbox_mdxl_set(&(MBOX(bus,mbox).CAN_MDL), u16);  }
static inline  uint16_t  mbox_mdll_get  (int bus,  int mbox)                 {  return _mbox_mdxl_get(&(MBOX(bus,mbox).CAN_MDL));       }

static inline  void      mbox_mdlh_set  (int bus,  int mbox,  uint16_t u16)  {         _mbox_mdxh_set(&(MBOX(bus,mbox).CAN_MDL), u16);  }
static inline  uint16_t  mbox_mdlh_get  (int bus,  int mbox)                 {  return _mbox_mdxh_get(&(MBOX(bus,mbox).CAN_MDL));       }

static inline  void      mbox_mdhl_set  (int bus,  int mbox,  uint16_t u16)  {         _mbox_mdxl_set(&(MBOX(bus,mbox).CAN_MDH), u16);  }
static inline  uint16_t  mbox_mdhl_get  (int bus,  int mbox)                 {  return _mbox_mdxl_get(&(MBOX(bus,mbox).CAN_MDH));       }

static inline  void      mbox_mdhh_set  (int bus,  int mbox,  uint16_t u16)  {         _mbox_mdxh_set(&(MBOX(bus,mbox).CAN_MDH), u16);  }
static inline  uint16_t  mbox_mdhh_get  (int bus,  int mbox)                 {  return _mbox_mdxh_get(&(MBOX(bus,mbox).CAN_MDH));       }

//@============================================================================= u64

#endif // CAN_MBOX_DATA_SI_H_
