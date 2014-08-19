#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <stdint.h>
#include <fstream>
#include <string>
#include "AudioOperations.h"

AudioOperations::AudioOperations(const char* fileName) {
	originalFileName = fileName;
	FILE* in = fopen(originalFileName,"r");
	fseek(in, 0, SEEK_END);
	long fsize = ftell(in);
	fseek(in, 0, SEEK_SET);

	original = (char *)malloc(fsize + 1);
	fread(original, fsize, 1, in);

	original[fsize] = 0;

	int i = 12;

	while (!(original[i] == 'f' && original[i + 1] == 'm' && original[i + 2] == 't')) {
		i++;
	}

	fmtStart = i;

	fseek(in, fmtStart+4, 0);
	fread(&fmtSize, 4, 1, in);

	audioFormat = original[i+8];

	numChannels = original[i+10];

	fseek(in, fmtStart+12, 0);
	fread(&sampleRate, 4, 1, in);

	fseek(in, fmtStart+16, 0);
	fread(&byteRate, 4, 1, in);
	i = 12;

	while (!(original[i] == 'd' && original[i + 1] == 'a' && original[i + 2] == 't' && original[i + 3] == 'a')) {
		i++;
	}

	dataStart = i;

	fseek(in, dataStart+4, 0);
	fread(&dataSize, 4, 1, in);

	numSamples = dataSize/2;

	samples = (int16_t *)malloc(dataSize + 1);
	fread(samples, dataSize, 1, in);

	fclose(in);
}

AudioOperations::~AudioOperations(void) {

}

void AudioOperations::fadeIn(double fadeSeconds) {
	std::string fileName;
	fileName = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_fadeIn_" + std::to_string(fadeSeconds) + ".wav";
	std::ofstream out(fileName, std::ios::binary);

	for (int i = 0; i < 12; i++) {
		out << original[i];
	}

	for (int i = 0; i < fmtSize+8; i++) {
		out << original[fmtStart + i];
	}

	for (int i = 0; i < 8; i++) {
		out << original[dataStart+i];
	}
	
	int16_t n = 0;
	double multiplier;
	int fadeSamples = sampleRate*numChannels*fadeSeconds;

	for (int i = 0; i < numSamples; i++) {
		n = (int16_t)(samples[i]);

		if (i < fadeSamples) {
			multiplier = ((double)i) / fadeSamples;
			n = (int16_t)(multiplier*samples[i]);
		}
	
		out << std::hex << (char)((n<<8)>>8);
		out << std::hex << (char)(n>>8);
	}

	std::cout << "Fade In Complete";
}

void AudioOperations::fadeOut(double fadeSeconds) {
	std::string fileName;
	fileName = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_fadeOut_" + std::to_string(fadeSeconds) + ".wav";
	std::ofstream out(fileName, std::ios::binary);

	for (int i = 0; i < 12; i++) {
		out << original[i];
	}

	for (int i = 0; i < fmtSize+8; i++) {
		out << original[fmtStart + i];
	}

	for (int i = 0; i < 8; i++) {
		out << original[dataStart+i];
	}
	
	int16_t n = 0;
	double multiplier;
	int fadeSamples = sampleRate*numChannels*fadeSeconds;

	for (int i = 0; i < numSamples; i++) {
		n = (int16_t)(samples[i]);

		if (i > (numSamples-fadeSamples)) {
			multiplier = ((double)(numSamples - i)) / fadeSamples;
			n = (int16_t)(multiplier*samples[i]);
		}
	
		out << std::hex << (char)((n<<8)>>8);
		out << std::hex << (char)(n>>8);
	}

	std::cout << "Fade In Complete";
}