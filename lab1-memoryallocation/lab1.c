/* Matt Morgis - 917152237
 * Temple University 
 * CIS 3207 - Lab 1
 *
 * Program will determine a random number of records
 * to read in from a text file.
 * Generates a random starting point, reads records to memory,
 * sleeps, frees memory.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DELAY_SECONDS 10
#define RECORD_SIZE 1298
#define NUM_OF_ITERATIONS 3


// --------------------------------------

/*
 * Function determines the delay or sleep time
 */
void sleep(time_t delay)
{
	time_t t0, t1;
	
	time(&t0);
	
	do
	time(&t1);
	while(t1-t0 < delay);
}

// --------------------------------------

int main()
{
	FILE * file;
	size_t result;
	char * buffer;
	int offset, n, i;
	
	// initialize random seed
	srand(time(NULL));
	
	// open file
	file = fopen("records.txt", "rb");
	if (file == NULL)
	{
		fputs("File error", stderr);
		exit(1);
	}
	
	// reading loop
	for (i=0; i < NUM_OF_ITERATIONS; i++)
	{
		// random offset
		// where in file it will start reading from
		offset = rand() % 0xFFFFF;
		printf("offset = %d\n", offset);
		
		// random number of records to read
		n = rand() % 5000 + 1;
		printf("n = %d\n", n);
	
		// allocate memory to contain n number of records
		buffer = (char*) malloc(sizeof(char) * RECORD_SIZE * n);
		if (buffer == NULL)
		{
			fputs("Memory error", stderr);
			exit(2);
		}
	
		// set stream position
		fseek(file, offset, SEEK_SET);
	
		// read n records from file
		result = fread(buffer, RECORD_SIZE, n, file);
		if (result != n)
		{
			fputs("Reading error", stderr);
			exit(3);
		}
	
		// delay
		sleep(DELAY_SECONDS);
	
		// free allocated memory
		free(buffer);
	}
	
	// close file
	fclose(file);
	
	return 0;
}
