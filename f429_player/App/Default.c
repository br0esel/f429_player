/*
 * Default.c
 *
 *  Created on: 17.09.2018
 *      Author: dennis
 */

#include <math.h>
#include "dac.h"
#include "tim.h"
#include "fatfs.h"
#include "usb_host.h"
#include "mp3dec.h"

extern ApplicationTypeDef Appli_state;
static char inputBuffer[16 * 1024];
static int16_t outputBuffer[MAX_NCHAN * MAX_NGRAN * MAX_NSAMP];
static uint16_t dacBuffer1[MAX_NSAMP * MAX_NGRAN];
static uint16_t dacBuffer2[MAX_NSAMP * MAX_NGRAN];

static uint16_t* pointerToActiveDacBuffer;

#define INPUT_BUFFER_SIZE (16 * 1024)

//static void ClearDacBuffer() {
//  for (int i = 0; i < MAX_NSAMP * MAX_NGRAN; ++i) {
//      dacBuffer[i] = 0;
//  }
//}

static void SwapDacBuffer() {
    if(pointerToActiveDacBuffer == dacBuffer1){
        pointerToActiveDacBuffer = dacBuffer2;
    }else{
        pointerToActiveDacBuffer = dacBuffer1;
    }
}

void StartMp3() {
    HAL_GPIO_WritePin(LD_GREEN_GPIO_Port, LD_GREEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LD_RED_GPIO_Port, LD_RED_Pin, GPIO_PIN_RESET);
    HAL_TIM_Base_Start(&htim7);
    while (Appli_state != APPLICATION_READY) {
        osDelay(1);
    }

    if (f_mount(&USBHFatFS, USBHPath, 1) == FR_OK) {
        FIL file;
        if (f_open(&file, "insom.mp3", FA_READ) == FR_OK) {
            HMP3Decoder hMp3Decoder = MP3InitDecoder();
            unsigned int bufferFillCount = 0;
            char * pointerToInputbufferStart = inputBuffer;
            pointerToActiveDacBuffer = dacBuffer1;
//          ClearDacBuffer();

            do {
                if ((void*) pointerToInputbufferStart == (void*) inputBuffer) {
                    f_read(&file, inputBuffer, INPUT_BUFFER_SIZE, &bufferFillCount);
                } else {
                    memmove(inputBuffer, pointerToInputbufferStart, bufferFillCount);
                    unsigned int bytesRead = 0;
                    f_read(&file, inputBuffer + bufferFillCount, INPUT_BUFFER_SIZE - bufferFillCount, &bytesRead);
                    bufferFillCount += bytesRead;
                    pointerToInputbufferStart = inputBuffer;
                }

                if (bufferFillCount <= 0) {
                    break;
                }

                int offsetToMp3Frame = MP3FindSyncWord(inputBuffer, bufferFillCount);
                if (offsetToMp3Frame == -1) {
                    break;
                }

                pointerToInputbufferStart += offsetToMp3Frame;
                bufferFillCount -= offsetToMp3Frame;

                int errorCode = MP3Decode(hMp3Decoder, &pointerToInputbufferStart, &bufferFillCount, outputBuffer, 0);

                switch (errorCode) {
                case ERR_MP3_NONE:

//                  ClearDacBuffer();
                    for (int i = 0; i < MAX_NSAMP * MAX_NGRAN; ++i) {
                        pointerToActiveDacBuffer[i] = (int) outputBuffer[i * 2] / 16
                                + UINT16_MAX / 32;
                    }
                    while (HAL_DAC_GetState(&hdac) != HAL_DAC_STATE_READY) {
                        ;
                    }
                    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, pointerToActiveDacBuffer, MAX_NSAMP * MAX_NGRAN, DAC_ALIGN_12B_R);
                    SwapDacBuffer();

                    break;
                case ERR_MP3_INVALID_FRAMEHEADER:
                    pointerToInputbufferStart++;
                    bufferFillCount--;
                    break;
                default:
                    break;
                }
                osDelay(1);
            } while (TRUE);
        }
    } else {
        HAL_GPIO_WritePin(LD_GREEN_GPIO_Port, LD_GREEN_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LD_RED_GPIO_Port, LD_RED_Pin, GPIO_PIN_SET);
    }
}
