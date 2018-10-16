/*
 * App.cpp
 *
 *  Created on: 10.10.2018
 *      Author: dennis
 */

#include <App.h>
#include "Mp3Player.h"
#include "usb_host.h"

extern ApplicationTypeDef Appli_state;

void StartApp() {
	while (true) {
		while (Appli_state != APPLICATION_READY) {
			;
		}
		FATFS fatfs;
		if (f_mount(&fatfs, USBHPath, 1) == FR_OK) {
			App::Mp3Player mp3Player;
			mp3Player.Load("Insom.mp3");
			osDelay(50);
			while (Appli_state != APPLICATION_DISCONNECT) {
				;
			}
			mp3Player.Unload();
//			App::Mp3Player mp3Player2;
//			mp3Player2.Load("");
		}

	}
}
