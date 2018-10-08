/*
 * Test.cpp
 *
 *  Created on: 08.10.2018
 *      Author: dennis
 */

#include "Test.h"
#include "stm32f4xx.h"
#include "dac.h"

void Start(){
	HAL_GPIO_WritePin(LD_RED_GPIO_Port, LD_RED_Pin, GPIO_PIN_SET);
	HAL_DAC_MODULE_ENABLED
}
