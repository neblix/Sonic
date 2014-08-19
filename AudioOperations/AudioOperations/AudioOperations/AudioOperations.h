#pragma once

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>

class AudioOperations
{
public:

	AudioOperations(const char* file);

	~AudioOperations(void);
	
	void fadeIn(double fadeSeconds);
	
	void fadeOut(double fadeSeconds);
private:
	const char* originalFileName;
	int numChannels;
	int audioFormat;
	int sampleRate;
	int byteRate;
	int dataSize;
	char *original;
	int16_t *samples;
	int fmtSize;
	int fmtStart;
	int dataStart;
	int numSamples;
};