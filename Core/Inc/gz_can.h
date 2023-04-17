

#ifndef INC_GZ_CAN_H_
#define INC_GZ_CAN_H_

/*
 * gz_can.h
 *
 *  Created on: 12 Kas 2022
 *      Author: Abdul Samet Durmaz
 *
 *
 *CAN RX0 and CAN SCE interrupt açık olmalı
 *CAN
*/


#include "stm32f1xx_hal.h"
#include "string.h"

#define CAN_DEADTIME 3000 //millisecond

#define MYSTDID 0x010
//10
#define OTHERSTDID 0x017

extern CAN_HandleTypeDef hcan;

extern void Error_Handler(void);

struct MOTOR{
	int8_t temp;
	uint8_t voltage;
	uint8_t current;

};

void GZ_CAN_Init();

void GZ_CAN_Transmit(struct MOTOR motorx);

void GZ_CAN_Receive(float *m1speed, float *m2speed);







#endif /* INC_GZ_CAN_H_ */
