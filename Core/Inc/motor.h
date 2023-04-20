/*
 * motor.h
 *
 *  Created on: 12 Kas 2022
 *      Author: Merthan
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include <stdint.h>
#ifdef __cplusplus
 extern "C" {
#endif

#define MOTOR12_TIM					htim1 // MOTOR1==CCR1|CCR2	MOTOR2==CCR3|CC4

#define MOTOR1_EN_PORT 				GPIOA
#define MOTOR1_EN_PIN				GPIO_PIN_3
#define MOTOR2_EN_PORT				GPIOA
#define MOTOR2_EN_PIN				GPIO_PIN_4

#define MOTOR1_ID	0x01
#define MOTOR2_ID	0x02


//encoder
#define ENCODERTIM_1 				htim2
#define ENCODERTIM_1_CHANNEL		TIM_CHANNEL_ALL
#define ENCODERTIM_2				htim3
#define ENCODERTIM_2_CHANNEL		TIM_CHANNEL_ALL



void motor_Init();
void encoder_loop(double left,double right);

void motor1_set_speed(int16_t pwm);
void motor2_set_speed(int16_t pwm);

#ifdef __cplusplus
}
#endif



#endif

