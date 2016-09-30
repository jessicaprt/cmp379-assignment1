#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "memlayout.h"
#define REGIONS_SIZE 32
#define DIFF_SIZE 32

int PAGE_SIZE = 4096;

int main(int argc, char** argv){
	printf("*** mem_mod2 ***\n");
	struct memregion regions[REGIONS_SIZE];
	struct memregion thediff[DIFF_SIZE];

	//call get_mem_layout
	int numregions = get_mem_layout(regions, REGIONS_SIZE);
	print_regions(regions, numregions, REGIONS_SIZE); 

	//void *malloc(PAGE_SIZE);
	mmap(NULL, 3*PAGE_SIZE, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

	//call get_mem_diff
	int diffregions = get_mem_diff(regions, numregions, thediff, DIFF_SIZE);
	print_regions(thediff, diffregions, DIFF_SIZE);

	return 0;
}
