#include <stdio.h>

#include "memlayout.h"

#define REGIONS_SIZE 4

int PAGE_SIZE = 4096;

int main(int argc, char** argv){
	struct memregion regions[REGIONS_SIZE];

	get_mem_layout(regions, REGIONS_SIZE);

	printf("Hello world\n");
	return 0;
}
