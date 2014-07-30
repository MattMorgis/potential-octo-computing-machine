#include <stdio.h>
#include <stdlib.h>

int main() {
	int offset = rand() % 0xFFFFF;
	printf("offset = %d\n", offset);

	return 0;
}



