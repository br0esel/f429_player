/*
 * Mp3Player.h
 *
 *  Created on: 10.10.2018
 *      Author: dennis
 */

#ifndef MP3PLAYER_H_
#define MP3PLAYER_H_

#include "cmsis_os.h"
#include "fatfs.h"
#include "string"
#include "mp3dec.h"
#include "ff.h"

namespace App {

struct StereoAudioSample{
	uint16_t L;
	uint16_t R;
};

class InputBuffer{
private:
public:
	InputBuffer();
	static constexpr int Size = 1024 * 16;
	unsigned int Count = 0;
	char *FirstItem = Data;
	//void char *LastItem;
	char Data[Size];
	void Update();
	void Defrag();
	void Reset();
};

class Mp3Player {
private:
	osThreadId mp3PlayerTaskHandle;
	FIL mp3File;
	void WorkerThread();
	InputBuffer inputBuffer;
	HMP3Decoder hMp3Decoder;
	static constexpr int OutputBufferSize = MAX_NCHAN * MAX_NGRAN * MAX_NSAMP;
	StereoAudioSample OutputPcmBuffer[OutputBufferSize];
	bool fileLoaded = false;
public:
	Mp3Player();
	~Mp3Player();
	void Play();
	void Stop();
	void Load(std::string fileName);
	void Unload();
	static void WorkerThreadManager(void const *arg);
	void RefillInputBuffer();
};

} /* namespace App */

#endif /* MP3PLAYER_H_ */
