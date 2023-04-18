/*
 * gz_can.c
 *
 *  Created on: 12 Kas 2022
 *      Author: Abdul Samet Durmaz
 */

#include "gz_can.h"

 CAN_TxHeaderTypeDef myTxHeader;
 CAN_RxHeaderTypeDef myRxHeader;
 uint8_t rxData[8] = { 0 };
 uint8_t rxData_1[8] = {0};
 uint8_t rxData_2[8] = {0};



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
	canfilterconfig.FilterMaskIdHigh = 0x0000 << 5;//0xFFFF << 5
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

	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &myRxHeader, rxData)
			!= HAL_OK) {
		Error_Handler();
	}
	if(myRxHeader.StdId == 0x17){
		memcpy(rxData_1, rxData,8);

	}else if(myRxHeader.StdId == 0x16){
		memcpy(rxData_2, rxData,8);
	}
	/*
	if (myRxHeader.StdId != OTHERSTDID) {
		Error_Handler();
	}*/
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

double my_atof(char *str) {
    double result = 0.0;
    int sign = 1, decimal = 0;

    // Skip leading whitespace
    while (*str == ' ') {
        str++;
    }

    // Check for sign character
    if (*str == '+' || *str == '-') {
        if (*str == '-') {
            sign = -1;
        }
        str++;
    }

    // Convert digits before decimal point
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }

    // Convert digits after decimal point
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            result = result * 10.0 + (*str - '0');
            decimal++;
            str++;
        }
    }

    // Calculate the final result
    while (decimal--) {
        result /= 10.0;
    }
    return sign * result;
}



void GZ_CAN_Receive_motor1(double *p){
	dTime_1 = HAL_GetTick();
	double rslt=0;
	if(dTime_1 - nowTime_1 > CAN_DEADTIME){
		rslt =0;
	}
	else{
		rslt= my_atof(&rxData_1);
	}
	*p = rslt;
}

void GZ_CAN_Receive_motor2(double *p){
	dTime_2 = HAL_GetTick();
	double rslt=0;
	if(dTime_2 - nowTime_1 > CAN_DEADTIME){
		rslt = 0;
	}
	else{
		rslt= my_atof(&rxData_2);
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

