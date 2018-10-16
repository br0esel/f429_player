/*
 * Mp3Player.cpp
 *
 *  Created on: 10.10.2018
 *      Author: dennis
 */

#include <Mp3Player.h>
#include "stm32f4xx.h"
#include "fatfs.h"
#include "algorithm"

namespace App {

InputBuffer::InputBuffer(){

}

void InputBuffer::Defrag(){
	if(Data != FirstItem){
		memmove(Data,FirstItem,Count);
		FirstItem = Data;
	}
}

//No Loop alowd
void Mp3Player::WorkerThread() {
	HAL_GPIO_TogglePin(LD_RED_GPIO_Port, LD_RED_Pin);

	if (fileLoaded) {
		UINT bytesRead = 0;
		inputBuffer.Defrag();
		if (!f_eof(&mp3File)) {
			if (inputBuffer.Count == 0) {
				f_read(&mp3File, inputBuffer.Data, inputBuffer.Size, &bytesRead);
			} else {
				f_read(&mp3File, inputBuffer.FirstItem + inputBuffer.Count, inputBuffer.Size - inputBuffer.Count, &bytesRead);
			}
			inputBuffer.Count += bytesRead;
		} else if(f_eof(&mp3File) && inputBuffer.Count == 0){
			return;
		}
		int offsetToSyncWord = MP3FindSyncWord((unsigned char*)inputBuffer.Data, inputBuffer.Count);
		if (offsetToSyncWord == -1) {// no syncword found
			inputBuffer.Count = 1;
			inputBuffer.FirstItem = inputBuffer.Data + inputBuffer.Count - 1;
			return;
		}
		else if(offsetToSyncWord > 0){
			inputBuffer.FirstItem += offsetToSyncWord;
			inputBuffer.Count -= offsetToSyncWord;
		}

		int errorCode = MP3Decode(hMp3Decoder,(unsigned char**)&inputBuffer.FirstItem, (int*)&inputBuffer.Count,(short int*)OutputPcmBuffer,0);
	    switch (errorCode) {
	    case ERR_MP3_NONE:
	        break;
	    case ERR_MP3_INVALID_FRAMEHEADER:
	        break;
	    default:
	    	inputBuffer.FirstItem++;
	        break;
	    }
	}
	return;
}

Mp3Player::Mp3Player() {
	osThreadDef(mp3PlayerTask, (void (*)(const void*))Mp3Player::WorkerThreadManager, osPriorityNormal, 4, 1024);
	mp3PlayerTaskHandle = osThreadCreate(osThread(mp3PlayerTask), this);

    hMp3Decoder = MP3InitDecoder();
}

Mp3Player::~Mp3Player() {
	// TODO: kill worker thread
}

void Mp3Player::Play() {
}

void Mp3Player::Stop() {
}

void Mp3Player::Load(std::string fileName) {
	if(f_open(&mp3File,fileName.c_str(),FA_READ) == FR_OK){
		HAL_GPIO_WritePin(LD_GREEN_GPIO_Port,LD_GREEN_Pin,GPIO_PIN_SET);
		fileLoaded = true;
	} else{
		HAL_GPIO_WritePin(LD_RED_GPIO_Port,LD_RED_Pin,GPIO_PIN_SET);
		fileLoaded = false;
	}
}

void Mp3Player::WorkerThreadManager(void const *arg){
	while(true){
		((Mp3Player*)arg)->WorkerThread();
		osDelay(100);
	}
}

void Mp3Player::Unload() {
	f_close(&mp3File);
	fileLoaded = false;
	HAL_GPIO_WritePin(LD_GREEN_GPIO_Port,LD_GREEN_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LD_RED_GPIO_Port,LD_RED_Pin,GPIO_PIN_RESET);
}

} /* namespace App */


