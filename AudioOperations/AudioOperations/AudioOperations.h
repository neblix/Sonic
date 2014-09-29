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
	//AudioOperations constructor
	// To create an instance of the AudioOperations class, call
	// this constructor and pass it the file name (including the
	// extension). The constructor loads the file into memory
	// and extracts all information from the audio file.
	AudioOperations(const char* fileName);

	//AudioOperations destructor
	~AudioOperations(void);
	
	//Prints all relevant information about the audio file. This
	// includes: file size (in bytes), audio format (1 = PCM),
	// channels (1 = mono, 2 = stereo, etc.), sample rate (Hz),
	// byte rate (bytes/sec), bytes per block (bytes per multi-
	// channel sample), bits per sample (usually 16), and number
	// of samples (counting channels seperately).
	void AudioOperations::printInfo();

	//Creates a new wave file wherein the volume of the audio
	// is changed by the coefficient specified in the parameter
	// ratio (must be at least zero). If you provide a ratio
	// that too high and will cause the audio to clip, a new 
	// file will not be created and the maximum ratio will be
	// printed.
	void AudioOperations::amplify(double ratio);
	
	//Creates a new wave file wherein the audio volume fades
	// from zero to one at the beginning of the track
	// over the amount of time specified in the parameter
	// fadeSeconds.
	void AudioOperations::fadeIn(double fadeSeconds);
	
	//Creates a new wave file wherein the audio volume fades
	// from one to zero at the end of the track
	// over the amount of time specified in the parameter
	// fadeSeconds.
	void AudioOperations::fadeOut(double fadeSeconds);

	//Creates a new wave file wherein the audio's time scale
	// is scaled by the coefficient specified in the
	// parameter scale. This shortens/lengthens the audio
	// track, as well as alters the audio's pitch.
	// STILL GLITCHY
	void AudioOperations::scale(double scale);

	//Creates two new mono wave files from the stereo wave
	// file. Only works on stereo tracks.
	void AudioOperations::split();

	//Creates a new wave file with dynamic range compression.
	// The minimum amplitude to be affected is specified by
	// the parameter threshold (must be below zero), and the
	// ratio by which the amplitudes are compressed is specified
	// by the parameter ratio (must be at least one).
	// STILL GLITCHY
	void AudioOperations::compress(double threshold, double ratio);

  private:
	const char* originalFileName;
	int riffStart;
	int fileSize;
	int numChannels;
	int bitsPerSample;
	int audioFormat;
	int sampleRate;
	int byteRate;
	int blockAlign;
	int dataSize;
	char *original;
	int16_t *samples;
	int fmtSize;
	int fmtStart;
	int dataStart;
	int numSamples;
	double runningTime;

	void AudioOperations::monoSplit(int16_t * samples, int16_t *left, int16_t *right);

	void AudioOperations::writeHeader(std::ofstream* out, int fileSize, int audioFormat,
		int numChannels, int sampleRate, int byteRate, int blockAlign, int bitsPerSample, int dataSize);

	void AudioOperations::write16BitSample(std::ofstream* outputFile, int16_t value);
};