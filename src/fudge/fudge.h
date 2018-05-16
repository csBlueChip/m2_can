#ifndef FUDGE_H_
#define FUDGE_H_

#include "sam3x8e.h"
void NVIC_SetPriority(IRQn_Type, uint32_t);

extern int SystemCoreClock;

#include <stdint.h>
void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);

#include "component_can.h"

void delay(int);
#define OUTPUT 2
#define HIGH 1
#define LOW 0

//typedef void* Can*;
Can*       CAN0      = (void*)0;
int        CAN0_EN   = 0;
//IRQn_Type  CAN0_IRQn = 0;
uint32_t   ID_CAN0   = 0;

Can*       CAN1      = (void*)1;
int        CAN1_EN   = 1;
//IRQn_Type  CAN1_IRQn = 1;
uint32_t   ID_CAN1   = 1;


void pmc_enable_periph_clk(uint32_t);
void pmc_disable_periph_clk(uint32_t);
void pinMode(int,int);
void digitalWrite(int,int);

#endif // FUDGE_H_
