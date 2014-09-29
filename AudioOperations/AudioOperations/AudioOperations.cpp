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

	int i = 0;

	while (!(original[i] == 'R' && original[i + 1] == 'I' && original[i + 2] == 'F' && original[i + 3] == 'F')) {
		i++;
	}

	riffStart = i;

	fseek(in, riffStart+4, 0);
	fread(&fileSize, 4, 1, in);

	i = 12;

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
	
	blockAlign = original[i+20];

	bitsPerSample = original[i+22];

	i = 12;

	while (!(original[i] == 'd' && original[i + 1] == 'a' && original[i + 2] == 't' && original[i + 3] == 'a')) {
		i++;
	}

	dataStart = i;

	fseek(in, dataStart+4, 0);
	fread(&dataSize, 4, 1, in);

	numSamples = dataSize/2;
	
	runningTime = (numSamples/numChannels)/(double)sampleRate;

	samples = (int16_t *)malloc(dataSize + 1);
	fread(samples, dataSize, 1, in);

	fclose(in);
}

AudioOperations::~AudioOperations(void) {

}

void AudioOperations::printInfo() {

	std::cout << "\n         File Name: " << originalFileName;
	if (fileSize+8 >= 1048576) {
		std::cout << "\n         File Size: " << ((fileSize+8) / 1048576.0) << " MB";
	} else if(fileSize+8 >= 1024) {
		std::cout << "\n         File Size: " << ((fileSize+8) / 1024.0) << " KB";
	} else {
		std::cout << "\n         File Size: " << fileSize+8 << " bytes";
	}
	std::cout << "\n      Running Time: " << runningTime << " seconds";
	if (runningTime >= 60) {
		std::cout << " (" << (int)runningTime / 60 << ":";
		if ((int)runningTime % 60 < 10 && (int)runningTime % 60 != 0) {
			std::cout << "0" << (int)runningTime % 60;
		} else {
			std::cout << (int)runningTime % 60;
		}
		if ((int)runningTime % 10 == 0) {
			std::cout << "0";
		}
		std::cout << ")";
	}
	std::cout << "\n      Audio Format: " << audioFormat;
	if (audioFormat == 1) {
		std::cout << " (PCM)";
	} else {
		std::cout << " (not PCM)";
	}
	std::cout << "\n          Channels: " << numChannels;
	std::cout << "\n       Sample Rate: " << sampleRate << " kHz";
	std::cout << "\n           Bitrate: " << (byteRate*8)/1000 << " kbps";
	std::cout << "\n   Bytes Per Block: " << blockAlign;
	std::cout << "\n   Bits Per Sample: " << bitsPerSample;
	std::cout << "\n Number of Samples: " << numSamples;
	std::cout << '\n';
}

void AudioOperations::amplify(double ratio) {

	//Input check
	if (ratio < 0) {
		std::cout << "\nRatio must be at least zero.";
		return;
	}
	
	//Finds audio's peak amplitude
	int16_t peak = 0;

	for (int i = 0; i < numSamples; i++) {
		if (abs(samples[i]) > peak) {
			peak = abs(samples[i]);
		}
	}

	//Returns if given parameter would cause audio to clip
	if (ratio > (32767.0/peak)) {
		std::cout << "\nFrom this file, ratio must be below " << (32767.0/peak);
		return;
	}
	
	//Creates new wav file
	std::string fileName;
	fileName = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_amplify_" + std::to_string(ratio) + ".wav";
	std::ofstream out(fileName, std::ios::binary);

	//Writes header
	writeHeader(&out, fileSize, audioFormat, numChannels, sampleRate, byteRate, blockAlign, bitsPerSample, dataSize);
	
	int16_t n = 0;

	//Writes audio data
	for (int i = 0; i < numSamples; i++) {
		n = (int16_t)(samples[i]);

		//Applies amplification
		n = (int16_t)(n*ratio);
	
		write16BitSample(&out, n);
	}

	std::cout << "Amplify Complete";
}

void AudioOperations::fadeIn(double fadeSeconds) {

	//Checks input
	if (fadeSeconds < 0) {
		std::cout << "\Seconds must be at least zero.";
		return;
	}

	//Creates new wav file
	std::string fileName;
	fileName = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_fadeIn_" + std::to_string(fadeSeconds) + ".wav";
	std::ofstream out(fileName, std::ios::binary);

	//Writes header
	writeHeader(&out, fileSize, audioFormat, numChannels, sampleRate, byteRate, blockAlign, bitsPerSample, dataSize);
	
	int16_t n = 0;
	double multiplier;

	//Calculates how many samples will be affected by the fade
	int fadeSamples = sampleRate*numChannels*fadeSeconds;

	//Writes audio data
	for (int i = 0; i < numSamples; i++) {
		n = (int16_t)(samples[i]);

		//If sample is within the fade, reduce 
		// its volume by the appropriate factor
		if (i < fadeSamples) {
			multiplier = ((double)i) / fadeSamples;
			n = (int16_t)(multiplier*samples[i]);
		}
	
		write16BitSample(&out, n);
	}

	std::cout << "Fade In Complete";
}

void AudioOperations::fadeOut(double fadeSeconds) {

	//Checks input
	if (fadeSeconds < 0) {
		std::cout << "\Seconds must be at least zero.";
		return;
	}

	//Creates new wav file
	std::string fileName;
	fileName = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_fadeOut_" + std::to_string(fadeSeconds) + ".wav";
	std::ofstream out(fileName, std::ios::binary);

	//Writes header
	writeHeader(&out, fileSize, audioFormat, numChannels, sampleRate, byteRate, blockAlign, bitsPerSample, dataSize);
	
	int16_t n = 0;
	double multiplier;

	//Calculates how many samples will be affected by the fade
	int fadeSamples = sampleRate*numChannels*fadeSeconds;

	//Writes audio data
	for (int i = 0; i < numSamples; i++) {
		n = (int16_t)(samples[i]);

		//If sample is within the fade, reduce 
		// its volume by the appropriate factor
		if (i > (numSamples-fadeSamples)) {
			multiplier = ((double)(numSamples - i)) / fadeSamples;
			n = (int16_t)(multiplier*samples[i]);
		}
	
		write16BitSample(&out, n);
	}

	std::cout << "Fade Out Complete";
}

void AudioOperations::scale(double scale) {

	//Checks input
	if (scale <0) {
		std::cout << "\Scale must be at least zero.";
		return;
	}

	//Creates new wav file
	std::string fileName;
	fileName = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_scale_" + std::to_string(scale) + ".wav";
	std::ofstream out(fileName, std::ios::binary);

	//Writes header
	writeHeader(&out, fileSize - dataSize + dataSize*scale, audioFormat, numChannels, sampleRate, byteRate, blockAlign, bitsPerSample, dataSize*(1.0/scale));
	
	int16_t n = 0;
	double index = 0;
	double weight;
	
	//Splits the file into mono channels
	int16_t * left = (int16_t *)malloc(dataSize/2 + 1);
	int16_t * right = (int16_t *)malloc(dataSize/2 + 1);
	monoSplit(samples, left, right);

	//Writes audio data
	for (int i = 0; i < numSamples/2; i = (int)index) {

		//The weight is used to find the weighted average between two
		// samples.
		weight = 1 - (index-i);
		
		//Writes left sample
		n = (weight*left[i] + (1-weight)*left[i+1]);
 		write16BitSample(&out, n);

		//Writes right sample
		n = (weight*right[i] + (1-weight)*right[i+1]);
		write16BitSample(&out, n);

		//The scale influences the rate at which the loop parses 
		// through the data
		index += (1.0/scale);
	}

	std::cout << "Scale Complete";
}

void AudioOperations::split() {

	std::string fileNameL, fileNameR;

	//Creates new wav file for left channel
	fileNameL = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_split_left.wav";
	std::ofstream outL(fileNameL, std::ios::binary);

	//Creates new wav file for right channel
	fileNameR = ((std::string)originalFileName).substr(0,((std::string)originalFileName).find_last_of('.'))
		+ "_split_right.wav";
	std::ofstream outR(fileNameR, std::ios::binary);
	
	//Writes headers
	writeHeader(&outL, fileSize - (dataSize/2), audioFormat, 1, sampleRate, byteRate/2, blockAlign/2, bitsPerSample, dataSize/2);
	writeHeader(&outR, fileSize - (dataSize/2), audioFormat, 1, sampleRate, byteRate/2, blockAlign/2, bitsPerSample, dataSize/2);
	
	//Copies audio data
	for (int i = 0; i < numSamples; i++) {
	
		if (i%2 == 0) {
			//Left channel
			write16BitSample(&outL, samples[i]);
		} else {
			//Right channel
			write16BitSample(&outR, samples[i]);
		}
	}

	std::cout << "Split Complete";
}

//Private function
void AudioOperations::writeHeader(std::ofstream* out, int fileSizeX, int audioFormatX,
		int numChannelsX, int sampleRateX, int byteRateX, int blockAlignX, int bitsPerSampleX, int dataSizeX) {

	*out << "RIFF";
	*out << ((char*)(&fileSizeX))[0] << ((char*)(&fileSizeX))[1] << ((char*)(&fileSizeX))[2] << ((char*)(&fileSizeX))[3];
	*out << "WAVEfmt ";
	*out << ((char*)(&fmtSize))[0] << ((char*)(&fmtSize))[1] << ((char*)(&fmtSize))[2] << ((char*)(&fmtSize))[3];
	*out << ((char*)(&audioFormatX))[0] << ((char*)(&audioFormatX))[1];
	*out << ((char*)(&numChannelsX))[0] << ((char*)(&numChannelsX))[1];
	*out << ((char*)(&sampleRateX))[0] << ((char*)(&sampleRateX))[1] << ((char*)(&sampleRateX))[2] << ((char*)(&sampleRateX))[3];
	*out << ((char*)(&byteRateX))[0] << ((char*)(&byteRateX))[1] << ((char*)(&byteRateX))[2] << ((char*)(&byteRateX))[3];
	*out << ((char*)(&blockAlignX))[0] << ((char*)(&blockAlignX))[1];
	*out << ((char*)(&bitsPerSampleX))[0] << ((char*)(&bitsPerSampleX))[1];
	*out << "data";
	*out << ((char*)(&dataSizeX))[0] << ((char*)(&dataSizeX))[1] << ((char*)(&dataSizeX))[2] << ((char*)(&dataSizeX))[3];
}

//Private function
void AudioOperations::monoSplit(int16_t *samples, int16_t *left, int16_t *right) {
	//Parses through and splits up the given array of samples
	// in the most unnecessarily ugly way
	for (int i = 0; i < numSamples; i += 2) {
		left[i/2] = samples[i];
		right[i/2] = samples[i+1];
	}
}

//Private function
void AudioOperations::write16BitSample(std::ofstream* outputFile, int16_t value) {
		//Writes the sample value as characters into a file
		*outputFile << std::hex << ((char*)(&value))[0];
		*outputFile << std::hex << ((char*)(&value))[1];
		//Hallelujah
}