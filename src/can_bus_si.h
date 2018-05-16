#ifndef CAN_BUS_SI_H_
#define CAN_BUS_SI_H_

#include <stdint.h>
#include "bool.h" // To portably circumvent a bug in Arduino IDE

#include "can_bus.h"

//@============================================================================= =======================================
static inline  int       can_busCnt     (void)     {  return CAN_CNT;  }

//@============================================================================= ======================================= SR
//_pp1220-1222 : [R-] Status Register
// * CAN_SR_MB0    : Mailbox 0 Event
// * CAN_SR_MB1    : Mailbox 1 Event
// * CAN_SR_MB2    : Mailbox 2 Event
// * CAN_SR_MB3    : Mailbox 3 Event
// * CAN_SR_MB4    : Mailbox 4 Event
// * CAN_SR_MB5    : Mailbox 5 Event
// * CAN_SR_MB6    : Mailbox 6 Event
// * CAN_SR_MB7    : Mailbox 7 Event
// * CAN_SR_ERRA   : Error Active Mode
// * CAN_SR_WARN   : Warning Limit
// * CAN_SR_ERRP   : Error Passive Mode
// * CAN_SR_BOFF   : Bus Off Mode
// * CAN_SR_SLEEP  : CAN controller in Low power Mode
// * CAN_SR_WAKEUP : CAN controller is not in Low power Mode
// * CAN_SR_TOVF   : Timer Overflow
// * CAN_SR_TSTP   : Timestamp Interrupt Mask
// * CAN_SR_CERR   : Mailbox CRC Error
// * CAN_SR_SERR   : Mailbox Stuffing Error
// * CAN_SR_AERR   : Acknowledgment Error
// * CAN_SR_FERR   : Form Error
// * CAN_SR_BERR   : Bit Error
// - CAN_SR_RBSY   : Receiver busy
// - CAN_SR_TBSY   : Transmitter busy
// - CAN_SR_OVLSY  : Overload busy
// *(Interrupt event)
//
static inline  uint32_t  can_sr_get    (int bus)  {  return PCAN(bus)->CAN_SR;  }
// Interrupt events...
static inline  bool      can_sr_mb0    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB0   ) ? true : false;  }
static inline  bool      can_sr_mb1    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB1   ) ? true : false;  }
static inline  bool      can_sr_mb2    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB2   ) ? true : false;  }
static inline  bool      can_sr_mb3    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB3   ) ? true : false;  }
static inline  bool      can_sr_mb4    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB4   ) ? true : false;  }
static inline  bool      can_sr_mb5    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB5   ) ? true : false;  }
static inline  bool      can_sr_mb6    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB6   ) ? true : false;  }
static inline  bool      can_sr_mb7    (int bus)  {  return (can_sr_get(bus) & CAN_SR_MB7   ) ? true : false;  }
static inline  bool      can_sr_erra   (int bus)  {  return (can_sr_get(bus) & CAN_SR_ERRA  ) ? true : false;  }
static inline  bool      can_sr_warn   (int bus)  {  return (can_sr_get(bus) & CAN_SR_WARN  ) ? true : false;  }
static inline  bool      can_sr_errp   (int bus)  {  return (can_sr_get(bus) & CAN_SR_ERRP  ) ? true : false;  }
static inline  bool      can_sr_boff   (int bus)  {  return (can_sr_get(bus) & CAN_SR_BOFF  ) ? true : false;  }
static inline  bool      can_sr_sleep  (int bus)  {  return (can_sr_get(bus) & CAN_SR_SLEEP ) ? true : false;  }
static inline  bool      can_sr_wakeup (int bus)  {  return (can_sr_get(bus) & CAN_SR_WAKEUP) ? true : false;  }
static inline  bool      can_sr_tovf   (int bus)  {  return (can_sr_get(bus) & CAN_SR_TOVF  ) ? true : false;  }
static inline  bool      can_sr_tstp   (int bus)  {  return (can_sr_get(bus) & CAN_SR_TSTP  ) ? true : false;  }
static inline  bool      can_sr_cerr   (int bus)  {  return (can_sr_get(bus) & CAN_SR_CERR  ) ? true : false;  }
static inline  bool      can_sr_serr   (int bus)  {  return (can_sr_get(bus) & CAN_SR_SERR  ) ? true : false;  }
static inline  bool      can_sr_aerr   (int bus)  {  return (can_sr_get(bus) & CAN_SR_AERR  ) ? true : false;  }
static inline  bool      can_sr_ferr   (int bus)  {  return (can_sr_get(bus) & CAN_SR_FERR  ) ? true : false;  }
static inline  bool      can_sr_berr   (int bus)  {  return (can_sr_get(bus) & CAN_SR_BERR  ) ? true : false;  }
// Non-interrupt events...
static inline  bool      can_sr_rbsy   (int bus)  {  return (can_sr_get(bus) & CAN_SR_RBSY  ) ? true : false;  }
static inline  bool      can_sr_tbsy   (int bus)  {  return (can_sr_get(bus) & CAN_SR_TBSY  ) ? true : false;  }
static inline  bool      can_sr_ovlsy  (int bus)  {  return (can_sr_get(bus) & CAN_SR_OVLSY ) ? true : false;  }

//@============================================================================= ======================================= MR
//_pp1212 : [RW] Timer
static inline  bool      can_en_set     (int bus)  {  return (PCAN(bus)->CAN_MR |=  CAN_MR_CANEN), true;  }
static inline  bool      can_en_clr     (int bus)  {  return (PCAN(bus)->CAN_MR &= ~CAN_MR_CANEN), true;  }
static inline  uint32_t  can_en_get     (int bus)  {  return  PCAN(bus)->CAN_MR &   CAN_MR_CANEN;  }

static inline  bool      can_pause      (int bus)  {  return can_en_clr(bus);  }
//                       can_run()

//@=============================================================================
// Finish transmitting all pending messsages -then- turn off the clock
static inline  bool      can_lpm_set    (int bus)  {  return     can_mr_set(bus, CAN_MR_LPM);     }
static inline  bool      can_lpm_clr    (int bus)  {  return     can_mr_clr(bus, CAN_MR_LPM);     }
static inline  uint32_t  can_lpm_get    (int bus)  {  return PCAN(bus)->CAN_MR & CAN_MR_LPM;      }

static inline  void      can_sleep      (int bus)  {  can_lpm_set(bus);  }
static inline  void      can_wake       (int bus)  {  can_lpm_clr(bus);  }

//@=============================================================================
// Do not ACK frames on the bus
static inline  bool      can_abm_set    (int bus)  {  return     can_mr_set(bus, CAN_MR_ABM);     }
static inline  bool      can_abm_clr    (int bus)  {  return     can_mr_clr(bus, CAN_MR_ABM);     }
static inline  uint32_t  can_abm_get    (int bus)  {  return PCAN(bus)->CAN_MR & CAN_MR_ABM;      }

static inline  bool      can_listen_on  (int bus)  {  return can_abm_set(bus);  }
static inline  bool      can_listen_off (int bus)  {  return can_abm_clr(bus);  }
static inline  uint32_t  can_listen_get (int bus)  {  return can_abm_get(bus);  }

//@=============================================================================
// Generate an overload frame after -every- frame received (incl. Producers & Consumers)
static inline  bool      can_ovl_set    (int bus)  {  return     can_mr_set(bus, CAN_MR_OVL);     }
static inline  bool      can_ovl_clr    (int bus)  {  return     can_mr_clr(bus, CAN_MR_OVL);     }
static inline  uint32_t  can_ovl_get    (int bus)  {  return PCAN(bus)->CAN_MR & CAN_MR_OVL;      }

//@=============================================================================
// Timestamp is when the frame started [SoF] or when the frame ended [EoF]
static inline  bool      can_teof_set   (int bus)  {  return     can_mr_set(bus, CAN_MR_TEOF);    }
static inline  bool      can_teof_clr   (int bus)  {  return     can_mr_clr(bus, CAN_MR_TEOF);    }
static inline  uint32_t  can_teof_get   (int bus)  {  return PCAN(bus)->CAN_MR & CAN_MR_TEOF;     }

static inline  bool      can_mark_eof   (int bus)  {  return can_teof_set(bus);  }
static inline  bool      can_mark_sof   (int bus)  {  return can_teof_clr(bus);  }
static inline  bool      can_mark_get   (int bus)  {  return 0!= can_teof_get(bus);  }  // 0=SOF, 1=EOF

//@=============================================================================
// Enable Time Trigger Mode
static inline  bool      can_ttm_set    (int bus)  {  return     can_mr_set(bus, CAN_MR_TTM);     }
static inline  bool      can_ttm_clr    (int bus)  {  return     can_mr_clr(bus, CAN_MR_TTM);     }
static inline  uint32_t  can_ttm_get    (int bus)  {  return PCAN(bus)->CAN_MR & CAN_MR_TTM;      }

//@=============================================================================
//_pp1208 - Timer Freeze (RTFM)
static inline  bool      can_timFrz_set (int bus)  {  return     can_mr_set(bus, CAN_MR_TIMFRZ);  }
static inline  bool      can_timFrz_clr (int bus)  {  return     can_mr_clr(bus, CAN_MR_TIMFRZ);  }
static inline  uint32_t  can_timFrz_get (int bus)  {  return PCAN(bus)->CAN_MR & CAN_MR_TIMFRZ;   }

//@=============================================================================
//_pp1213 - If a Tx mailbox fails set CAN_MSR[n] |= (MABT | MRDT) ...instead of retry
static inline  bool      can_drpt_set   (int bus)  {  return      can_mr_set(bus, CAN_MR_DRPT);   }
static inline  bool      can_drpt_clr   (int bus)  {  return      can_mr_clr(bus, CAN_MR_DRPT);   }
static inline  uint32_t  can_drpt_get   (int bus)  {  return  PCAN(bus)->CAN_MR & CAN_MR_DRPT;    }
// Double-negativees aren't non-confusing  ... repeat == !don't_repeat
static inline  bool      can_rpt_set    (int bus)  {  return /**/ can_mr_clr(bus, CAN_MR_DRPT);   }
static inline  bool      can_rpt_clr    (int bus)  {  return /**/ can_mr_set(bus, CAN_MR_DRPT);   }
static inline  uint32_t  can_rpt_get    (int bus)  {  return (PCAN(bus)->CAN_MR & CAN_MR_DRPT) ^ CAN_MR_DRPT;  }

//@=============================================================================
// This is a debug feature
static inline  bool      can_rxsync_set (int bus,  uint32_t val)  {  return can_mr_set(bus, val);  }
static inline  uint32_t  can_rxsync_get (int bus)                 {  return PCAN(bus)->CAN_MR & CAN_MR_RXSYNC_Msk;   }

//@============================================================================= ======================================= TIM
//_pp1224 : [R-] Timer
static inline
uint32_t  can_tim_get (int bus)
{
	return (PCAN(bus)->CAN_TIM & CAN_TIM_TIMER_Msk) >> CAN_TIM_TIMER_Pos;
}

//@============================================================================= ======================================= TIMESTP
//_pp1225 : [R-] Timestamp
static inline
uint32_t  can_timestp_get (int bus)
{
	return (PCAN(bus)->CAN_TIMESTP & CAN_TIMESTP_MTIMESTAMP_Msk) >> CAN_TIMESTP_MTIMESTAMP_Pos;
}

//@============================================================================= ======================================= TCR
//_pp1226 : [R-] Transfer Command
static inline  uint32_t  can_ecr_get (int bus)  {  return PCAN(bus)->CAN_ECR;  }
static inline  uint32_t  can_rec_get (int bus)  {  return (can_ecr_get(bus) & CAN_ECR_REC_Msk) >> CAN_ECR_REC_Pos;  }
static inline  uint32_t  can_tec_get (int bus)  {  return (can_ecr_get(bus) & CAN_ECR_TEC_Msk) >> CAN_ECR_TEC_Pos;  }

//@============================================================================= ======================================= TCR
//_pp1227 : [-W] Transfer Command
static inline
bool  can_xfr (int bus,  uint32_t mask,  bool timrst)
{
	return (PCAN(bus)->CAN_TCR = mask | (timrst ? CAN_TCR_TIMRST : 0)), true;
}

//@============================================================================= ======================================= ACR
//_pp1228 : [-W] Abort Command
static inline  bool  can_abort (int bus,  uint32_t mask)  {  return (PCAN(bus)->CAN_ACR = mask), true;  }

//@============================================================================= ======================================= WP
// Protects the registers:
//     Ref Pages     Name      Description
//    ------------- --------- ----------------------------------
//     pp1212        CAN_MR    Mode Register
//     pp1192-1195   CAN_BR    Baudrate Register
//     pp            CAN_MMR   Mode Register(3)
//     pp1189        CAN_MAM   Message Acceptance Mask Register
//     pp1189        CAN_MID   Message ID Register
//
#define CAN_WP_MAGIC  (('C' << 24) | ('A' << 16) | ('N' << 8))

//_pp1229 : [RW] Write Protect enable|disable
static inline  void      can_wp_set  (int bus)  {          PCAN(bus)->CAN_WPMR = CAN_WP_MAGIC | CAN_WPMR_WPEN;  }
static inline  void      can_wp_clr  (int bus)  {          PCAN(bus)->CAN_WPMR = CAN_WP_MAGIC | 0x00;           }
static inline  uint32_t  can_wp_get  (int bus)  {  return  PCAN(bus)->CAN_WPMR & CAN_WPMR_WPEN;                 }

//_pp1230 : [R-] Write Protect Voilations
static inline  uint32_t  can_wpv_who (int bus)  {  return (PCAN(bus)->CAN_WPSR & CAN_WPSR_WPVSRC_Msk) >> CAN_WPSR_WPVSRC_Pos;  }
static inline  uint32_t  can_wpv_get (int bus)  {  return  PCAN(bus)->CAN_WPSR & CAN_WPSR_WPVS;  }

#endif // CAN_BUS_SI_H_
