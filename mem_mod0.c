#include <stdio.h>

#include "memlayout.h"

#define REGIONS_SIZE 32

int PAGE_SIZE = 4096;

int main(int argc, char** argv){
	struct memregion regions[REGIONS_SIZE];

	int numregions = get_mem_layout(regions, REGIONS_SIZE);
	print_regions(regions, numregions, REGIONS_SIZE); 

	return 0;
}
