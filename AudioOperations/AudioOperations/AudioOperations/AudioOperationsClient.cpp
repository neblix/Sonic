// AudioOperations.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "AudioOperations.h"

int _tmain(int argc, _TCHAR* argv[])
{
	AudioOperations *op = new AudioOperations("EpicDubstepExplosion.wav");
	op->fadeOut(10);

	std::cin.ignore();
	return 0;
}