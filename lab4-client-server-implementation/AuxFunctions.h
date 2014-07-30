#include <iostream>
#include <fstream>
#include <ctime>
#ifndef AUX
#define AUX

using std::time;

// sleep function that is used to make the random and fixed delay.
void sleep(unsigned int mseconds)
{
	clock_t goal = mseconds + clock();
	while (goal > clock()); // simply keep looping until the time comes.
}

#endif