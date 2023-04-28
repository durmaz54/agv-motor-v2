/*
 * encoder_v2.cpp
 *
 *  Created on: 11 Kas 2022
 *      Author: Merthan
 */
#include "motor.h"
#include "pid.h"
#include "math.h"

#define PID_TIME 50


extern TIM_HandleTypeDef MOTOR12_TIM;

extern TIM_HandleTypeDef ENCODERTIM_1;

extern TIM_HandleTypeDef ENCODERTIM_2;

//const float pulsesperturn = 600.0, wheel_diameter = 0.20;
//const float distancePerPulse = (M_PI * wheel_diameter) / pulsesperturn;

 double speed_act_1 = 0;
 double speed_act_2 = 0;


  double kp = 80, ki = 0.00, kd = 10; //kp=40_5

 double pid_input_1, pid_output_1, pid_setpoint_1 = 0;
 double pid_input_2, pid_output_2, pid_setpoint_2 = 0;


PID PID1(&pid_input_1, &pid_output_1, &pid_setpoint_1, kp, ki, kd, _PID_CD_DIRECT);

PID PID2(&pid_input_2, &pid_output_2, &pid_setpoint_2, kp, ki, kd, _PID_CD_DIRECT);

 double speed_1;
 double speed_2;


static int32_t nowTime = 0, dTime = 0;

 int32_t encoder_1_pulses, encoder_1_pulses_prev;
 int32_t encoder_2_pulses, encoder_2_pulses_prev;

 int16_t encoder1_temp = 0;
 int16_t encoder2_temp = 0;

 int16_t encoder1_temp_shifting = 0;
 int16_t encoder2_temp_shifting = 0;

 uint32_t encoder1_temp_timer = 0;
 uint32_t encoder2_temp_timer = 0;


/*pwm 20khz timer 72mhz
 16 prescale 4.5mhz 225aar
 8  prescale 9mhz 450aar
 4  prescale 18mhz 900aar
 2  prescale 36mhz 1800aar
 1  prescale 72mhz 3600aar
 */

void motor_pins_setup() {

	HAL_TIM_PWM_Start(&MOTOR12_TIM, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&MOTOR12_TIM, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&MOTOR12_TIM, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&MOTOR12_TIM, TIM_CHANNEL_4);
}

void encoder_setup() {
	HAL_TIM_Encoder_Start(&ENCODERTIM_1, ENCODERTIM_1_CHANNEL);
	HAL_TIM_Encoder_Start(&ENCODERTIM_2, ENCODERTIM_2_CHANNEL);

	HAL_TIM_Base_Start_IT(&ENCODERTIM_1);
	HAL_TIM_Base_Start_IT(&ENCODERTIM_2);

	encoder1_temp = 0;
	encoder2_temp = 0;
}

void pid_setup() {
	PID1.SetMode(_PID_MODE_AUTOMATIC);
	PID1.SetSampleTime(PID_TIME); //millisecond
	PID1.SetOutputLimits(-50, 50);

	PID2.SetMode(_PID_MODE_AUTOMATIC);
	PID2.SetSampleTime(PID_TIME); //millisecond
	PID2.SetOutputLimits(-50, 50);
}

void motor_Init() {
	pid_setup();
	encoder_setup();
	motor_pins_setup();
}

void motor1_set_speed(int16_t pwm) {
	if (pwm != 10000) {
		HAL_GPIO_WritePin(MOTOR1_EN_PORT, MOTOR1_EN_PIN, GPIO_PIN_SET);
		if (pwm >= 0) {
			MOTOR12_TIM.Instance->CCR1 = pwm; //PA6
			MOTOR12_TIM.Instance->CCR2 = 0;  //PA7
		} else {
			pwm *= -1;
			MOTOR12_TIM.Instance->CCR1 = 0;
			MOTOR12_TIM.Instance->CCR2 = pwm;
		}
	} else {
		HAL_GPIO_WritePin(MOTOR1_EN_PORT, MOTOR1_EN_PIN, GPIO_PIN_RESET);
		;
	}
}

void motor2_set_speed(int16_t pwm) {
	if (pwm != 10000) {
		HAL_GPIO_WritePin(MOTOR2_EN_PORT, MOTOR2_EN_PIN, GPIO_PIN_SET);
		if (pwm > 0) {
			MOTOR12_TIM.Instance->CCR3 =pwm;  //PA6
			MOTOR12_TIM.Instance->CCR4 = 0;  //PA7
		} else {
			pwm *= -1;
			MOTOR12_TIM.Instance->CCR3 = 0;
			MOTOR12_TIM.Instance->CCR4 = pwm;
		}
	} else {
		HAL_GPIO_WritePin(MOTOR2_EN_PORT, MOTOR2_EN_PIN, GPIO_PIN_RESET);

	}
}

//interrupt
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &ENCODERTIM_1) {
		encoder1_temp_timer = (uint32_t) __HAL_TIM_GET_COUNTER(&ENCODERTIM_1);
		if (encoder1_temp_timer > 30000) {
			encoder1_temp -= 1;
		} else {
			encoder1_temp += 1;
		}


	} else if (htim == &ENCODERTIM_2) {
		encoder2_temp_timer = (uint32_t) __HAL_TIM_GET_COUNTER(&ENCODERTIM_2);
		if (encoder2_temp_timer > 30000) {
			encoder2_temp -= 1;
		} else {
			encoder2_temp += 1;
		}
	}

}

void encoder_loop(double left,double right) {
	nowTime = HAL_GetTick();
	if (nowTime - dTime >= PID_TIME) {

		pid_setpoint_1=right;
		pid_setpoint_2=left;

		encoder_1_pulses = (int32_t) __HAL_TIM_GET_COUNTER(&ENCODERTIM_1) / 4;
		encoder1_temp_shifting = encoder1_temp;
		encoder_1_pulses = (encoder1_temp_shifting << 16) | encoder_1_pulses;
		encoder_1_pulses_prev = encoder_1_pulses - encoder_1_pulses_prev;

		speed_act_1 = ((1000.00 * (double) encoder_1_pulses_prev)
				/ ((double) (nowTime - dTime) * 600)) / 2.89;


		pid_input_1 = speed_act_1;//encoder kablosundan dolayı


		PID1.Compute();

		speed_1 += pid_output_1;
		//speed_1 = (int16_t)pid_output_1;
		if(speed_1 > 900){
			speed_1 = 900;
		}else if(speed_1 < -900){
			speed_1 = -900;
		}

		if(pid_setpoint_1 == 0){
			speed_1=0;
			motor1_set_speed(0);
		}
		else{
			motor1_set_speed((int16_t)speed_1);

		}
		encoder_1_pulses_prev = encoder_1_pulses;

		//motor222----------------------------------------------------

		encoder_2_pulses = (int32_t) __HAL_TIM_GET_COUNTER(&ENCODERTIM_2) / 4 ;
		encoder2_temp_shifting = encoder2_temp;
		encoder_2_pulses = (encoder2_temp_shifting << 16) | encoder_2_pulses;
		encoder_2_pulses_prev = encoder_2_pulses - encoder_2_pulses_prev;

		speed_act_2 = (((1000.00 * (double) encoder_2_pulses_prev)
				/ ((double) (nowTime - dTime) * 600)) / 2.89) * -1;//encoder kablosundan dolayı -1


		pid_input_2 = speed_act_2;//encoder kablosundan dolayı



		PID2.Compute();

		speed_2 += pid_output_2;
		//speed_2 = (int16_t)pid_output_2;

		if(speed_2 > 900){
			speed_2 = 900;
		}else if(speed_2 < -900){
			speed_2 = -900;
		}
		if(pid_setpoint_2 == 0){
			speed_2=0;
			motor2_set_speed(0);
		}
		else{
			motor2_set_speed((int16_t)speed_2);
		}
		encoder_2_pulses_prev = encoder_2_pulses;

		//---------time-----------------
		dTime = HAL_GetTick();
	}
}

