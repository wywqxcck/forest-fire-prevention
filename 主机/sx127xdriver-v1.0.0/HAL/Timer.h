#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"
#include "key.h"

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM1_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);
#endif
