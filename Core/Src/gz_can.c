/*
 * gz_can.c
 *
 *  Created on: 12 Kas 2022
 *      Author: Abdul Samet Durmaz
 */

#include "gz_can.h"




static CAN_TxHeaderTypeDef myTxHeader;
static CAN_RxHeaderTypeDef myRxHeader;
static uint8_t rxData[8] = { 0 };
static int32_t nowTime=0, dTime=0;
char motor1STR[4],motor2STR[4];

void GZ_CAN_Init() {

	CAN_FilterTypeDef canfilterconfig;

	canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
	canfilterconfig.FilterBank = 10;
	canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	// değişken 16, id 11 bit olduğu için kaydırıyoruz.
	canfilterconfig.FilterIdHigh = OTHERSTDID << 5; //OTHERSTDID << 5
	canfilterconfig.FilterIdLow = 0x0000; // for ext id //0x0000
	canfilterconfig.FilterMaskIdHigh = 0xFFFF << 5;
	canfilterconfig.FilterMaskIdLow = 0xFFFF; // for ext id

	canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
	canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
	canfilterconfig.SlaveStartFilterBank = 10;


	if (HAL_CAN_ConfigFilter(&hcan, &canfilterconfig) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK) {
		Error_Handler();
	}

	//CAN hata durumunda bir kere interrupt girmesi için CAN_IT_ERROR_WARNING
	//hata boyunca interrupta kalması için CAN_IT_LAST_ERROR_CODE
	if (HAL_CAN_ActivateNotification(&hcan,
			CAN_IT_ERROR | CAN_IT_LAST_ERROR_CODE) != HAL_OK) {
		Error_Handler();
	}

	if (HAL_CAN_Start(&hcan) != HAL_OK) {
		Error_Handler();
	}

	myTxHeader.DLC = 8; // max byte
	myTxHeader.StdId = MYSTDID;
	myTxHeader.RTR = CAN_RTR_DATA;
	myTxHeader.IDE = CAN_ID_STD;

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &myRxHeader, rxData)
			!= HAL_OK) {
		Error_Handler();
	}
	if (myRxHeader.StdId != OTHERSTDID) {
		Error_Handler();
	}
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	nowTime = HAL_GetTick();

}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {

	//hata logları

}

float test(char *str) {
	float rslt=0;
	int8_t i1=str[0] - '0';
	int8_t i2=str[2] - '0';
	int8_t i3=str[3] - '0';

	rslt = i1+((float)i2/10.00)+((float)i3/100.00);
	return rslt;
}


void GZ_CAN_Receive(float *m1speed, float *m2speed) {
	dTime = HAL_GetTick();

	memcpy(motor1STR, &rxData,4);
	memcpy(motor2STR, &rxData[4],4);


	if(dTime-nowTime > CAN_DEADTIME){
		*m1speed = 0x00;
		*m2speed = 0x00;
	}
	else{
		//sscanf(motor1STR, "%f",m1speed);
		//sscanf(motor2STR, "%f",m2speed);
		*m1speed = test(motor1STR);
		*m2speed = test(motor2STR);

	}

}

void GZ_CAN_Transmit(struct MOTOR motorx) {
	uint32_t txmailbox;
	uint8_t txdata[8];

	txdata[0] = motorx.temp;
	txdata[1] = motorx.voltage;
	txdata[2] = motorx.current;
	myTxHeader.DLC = 3;
	HAL_CAN_AddTxMessage(&hcan, &myTxHeader, txdata, &txmailbox);

}

