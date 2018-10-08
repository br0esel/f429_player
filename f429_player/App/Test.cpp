/*
 * Test.cpp
 *
 *  Created on: 08.10.2018
 *      Author: dennis
 */

#include "Test.h"
#include "stm32f4xx.h"
#include "dac.h"
#include "../Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h"

void etrf(){

}

#ifdef __cplusplus
	extern "C" {
#endif

void Start(){
	HAL_GPIO_WritePin(LD_RED_GPIO_Port, LD_RED_Pin, GPIO_PIN_SET);

}

#ifdef __cplusplus
	}
#endif
