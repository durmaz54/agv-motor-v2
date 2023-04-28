/*
 * gz_can.c
 *
 *  Created on: 12 Kas 2022
 *      Author: Abdul Samet Durmaz
 */

#include "gz_can.h"


extern UART_HandleTypeDef huart3;

uint8_t uartData[8]={0};

uint8_t canData[8]={0};


 CAN_TxHeaderTypeDef myTxHeader;
 CAN_RxHeaderTypeDef myRxHeader;
 uint8_t rxData[8] = { 0 };
 uint8_t rxData_1[8] = {0};
 uint8_t rxData_2[8] = {0};
	int32_t data1, data2;

	union byte_to_float {
	    float f;
	    unsigned char bytes[4];
	};


static int32_t nowTime_1=0, dTime_1=0;
static int32_t nowTime_2=0, dTime_2=0;


void GZ_CAN_Init() {

	CAN_FilterTypeDef canfilterconfig;

	canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
	canfilterconfig.FilterBank = 10;
	canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	// değişken 16, id 11 bit olduğu için kaydırıyoruz.
	canfilterconfig.FilterIdHigh = 0x0000; //OTHERSTDID << 5
	canfilterconfig.FilterIdLow = 0x0000; // for ext id //0x0000
	canfilterconfig.FilterMaskIdHigh = 0x0000;//0xFFFF << 5
	canfilterconfig.FilterMaskIdLow = 0x0000; // for ext id

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

	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &myRxHeader, canData)
			!= HAL_OK) {
		Error_Handler();
	}
	if(myRxHeader.StdId == 0x16){
		memcpy(uartData,canData,8);
		HAL_UART_Transmit(&huart3, uartData, 8, 50);
	}
	else{
		memcpy(rxData,canData,8);
	}


	nowTime_1 = HAL_GetTick();
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {

	//hata logları

}



/*
void GZ_CAN_Receive(double *m1speed, double *m2speed) {
	dTime = HAL_GetTick();



	//memcpy(motor1STR, &rxData[1],3);
	//memcpy(motor2STR, &rxData[5],3);



	if(dTime-nowTime > CAN_DEADTIME){
		*m1speed = 0x00;
		*m2speed = 0x00;
	}
	else{
		*m1speed = atof(rxData);
		*m2speed = atof(rxData);

	}

}*/



void GZ_CAN_Receive_motor1(float *p, float *p2){
	dTime_1 = HAL_GetTick();
	if(dTime_1 - nowTime_1 > CAN_DEADTIME){
		*p =0;
		*p2=0;
	}
	else{
		memcpy(p, rxData,4);
		memcpy(p2, &rxData[4],4);
		//memcpy(p2,rxData,4);
		/*
		memcpy(data_temp,rxData,8);
		data1=0;
		data2=0;
		//data1 =	(data_temp[3] << 24) | (data_temp[2] << 16) | (data_temp[1] << 8) | (data_temp[0]);
		data1 |= (uint16_t)data_temp[0];
		data1 |= ((uint16_t)data_temp[1] << 8);
		data1 |= ((uint16_t)data_temp[2] << 16);
		data1 |= ((uint16_t)data_temp[3] << 24);

		data2 =	(data_temp[7] << 24) | (data_temp[6] << 16) | (data_temp[5] << 8) | (data_temp[4]);
		*p = (float)data1/100.00;
		*p2 = (float)data2/100.00;*/
	}
}

void GZ_CAN_Receive_motor2(double *p){
	dTime_2 = HAL_GetTick();
	double rslt=0;
	if(dTime_2 - nowTime_1 > CAN_DEADTIME){
		rslt = 0;
	}
	else{
	}
	*p = rslt;
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

