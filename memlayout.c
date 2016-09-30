#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <fcntl.h>

extern int PAGE_SIZE;

#include "memlayout.h"

// Boolean Type Defintion
typedef int bool;
#define true 1
#define false 0

sigjmp_buf env;

void sigsev_handler(int sig) {
	siglongjmp(env, 2);
}

int get_mem_layout (struct memregion * regions, unsigned int size) {
	int fd = open("/dev/zero", O_RDONLY);
	char sample;
	int curr_size = 0;
	int curr_mode = 0;
	char * prev = 0;
	char * curr = 0;

	struct sigaction act;
	struct sigaction prev_act;

	act.sa_handler = sigsev_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGSEGV, &act, &prev_act);

	regions[0].from=curr;

	bool check = true;

	if(sigsetjmp(env,1) == 2) {
		regions[0].mode = MEM_NO;
		curr_mode = MEM_NO;
		check = false;
	}

	if (check){
		sample = *curr;

		if (read(fd, curr, 1) == 1) {
			regions[0].mode = MEM_RW;
			curr_mode = MEM_RW;
			*curr = sample;
		} else {
			regions[0].mode = MEM_RO;
			curr_mode = MEM_RO;
		}
	}

	while(curr >= prev && (uintptr_t) curr < (uint64_t) 0xFFFFFFFF) {

		check = true;
		if(sigsetjmp(env,1) == 2) {
			if (curr_mode != MEM_NO) {
				curr_size = curr_size + 1;
				curr_mode = MEM_NO;

				if (curr_size - 1 < size) {
					regions[curr_size - 1].to = curr-1;

					if(curr_size < size) {
						regions[curr_size].from = curr;
						regions[curr_size].mode = MEM_NO;
					}
				}
			}

			check = false;
		}

		if(check){
			sample = *curr;

			if (read(fd, curr, 1) == 1) {
				// can read.. MEM_RW
				*curr = sample;
				if (curr_mode != MEM_RW) {
					curr_size = curr_size + 1;
					curr_mode = MEM_RW;

					if (curr_size < size) {
						regions[curr_size - 1].to = curr - 1;

						if (curr_size < size) {
							regions[curr_size].from = curr;
							regions[curr_size].mode = MEM_RW;
						}
					}
				}

			} else {
				// MEM_RO
				if (curr_mode != MEM_RO) {
					curr_size = curr_size + 1;
					curr_mode = MEM_RO;

					if (curr_size < size) {
						regions[curr_size - 1].to = curr - 1;

						if(curr_size < size) {
							regions[curr_size].from = curr;
							regions[curr_size].mode = MEM_RO;
						}
					}
				}
			}
		}

		prev = curr;
		curr = curr + PAGE_SIZE;
	}

	// Close last region
	if(curr_size < size) {
		regions[curr_size].to = curr - 1;
		curr_size = curr_size + 1;
	}

	close(fd);

	sigaction(SIGSEGV, &prev_act, 0);

	return curr_size;

}

int get_mem_diff (struct memregion * regions, unsigned int howmany,
		struct memregion * thediff, unsigned int diffsize) {
	// Write Testing Variables
	int fd = open("/dev/zero", O_RDONLY);
	char sample;

	int curr_size = 0;
	int curr_mode = 0;

	int roff = 0; // regions_offset
	bool clean = true;

	char * prev = 0;
	char * curr = 0;


	// Signal Handler
	struct sigaction act;
	struct sigaction prev_act;

	act.sa_handler = sigsev_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGSEGV, &act, &prev_act);

	// Does the page need to be read permission checked
	bool check = true;

	if(sigsetjmp(env,1) == 2) {
		curr_mode = MEM_NO;
		check = false;
	}

	if (check){
		sample = *curr;

		if (read(fd, curr, 1) == 1) {
			curr_mode = MEM_RW;
			*curr = sample;
		} else {
			curr_mode = MEM_RO;
		}
	}

	if (curr_mode != regions[0].mode){
		thediff[0].from = curr;
		thediff[0].mode = curr_mode;
		clean = false;
	}

	while(curr >= prev && (uintptr_t) curr < (uint64_t) 0xFFFFFFFF) {
		check = true;
		if(sigsetjmp(env,1) == 2) {
			if (clean && curr - 1 != regions[roff].to && curr_mode == MEM_NO && regions[roff].mode != MEM_NO){
				clean = false;
				curr_size = curr_size + 1;
				if(curr_size < diffsize){
					thediff[curr_size].from = regions[roff].from;
				}
			}

			if (curr_mode != MEM_NO) {
				while(roff + 1 < howmany 
						&& regions[roff].from < (void *) curr){
					roff = roff + 1;
				}

				if(regions[roff - 1].to == curr - 1 &&
						MEM_NO == regions[roff].mode){
					if(clean == false){
						clean = true;
                        if(curr_size < diffsize){
                            thediff[curr_size].to = curr - 1;
                            curr_size = curr_size + 1;
                        }
					} else {
						clean = true;
					}
				} else {
					thediff[curr_size].to = curr - 1;
					curr_size = curr_size + 1;
					if(curr_size < diffsize){
						thediff[curr_size].mode = MEM_NO;
						thediff[curr_size].from = curr;
						clean = false;
					}
				}
				curr_mode = MEM_NO;

			}
			check = false;
		}

		if(check){
			sample = *curr;

			if (read(fd, curr, 1) == 1) {
				*curr = sample;
				// can read.. MEM_RW
				if (clean && curr - 1 != regions[roff].to && curr_mode == MEM_RW && regions[roff].mode != MEM_RW){
					clean = false;
					curr_size = curr_size + 1;
					if(curr_size < diffsize){
						thediff[curr_size].from = regions[roff].from;
					}
				}

				if (curr_mode != MEM_RW) {
					while(roff + 1 < howmany 
							&& regions[roff].from < (void *) curr){
						roff = roff + 1;
					}

					if(regions[roff - 1].to == curr - 1 &&
							MEM_RW == regions[roff].mode){
						if(clean == false){
							clean = true;
                            if(curr_size < diffsize){
                                thediff[curr_size].to = curr - 1;
                                curr_size = curr_size + 1;
                            }
						} else {
							clean = true;
						}
					} else {
						thediff[curr_size].to = curr - 1;
						curr_size = curr_size + 1;
						if(curr_size < diffsize){
							thediff[curr_size].mode = MEM_RW;
							thediff[curr_size].from = curr;
							clean = false;
						}
					}
					curr_mode = MEM_RW;

				} 

			} else {
				// MEM_RO
				if (clean && curr - 1 != regions[roff].to && regions[roff].mode != MEM_RO){
					clean = false;
					if(curr_size < diffsize){
						thediff[curr_size].from = regions[roff].from;
					}
				}

				if (curr_mode != MEM_RO) {
					while(roff + 1 < howmany 
							&& regions[roff].from < (void *) curr){
						roff = roff + 1;
					}

					if(regions[roff - 1].to == curr - 1 &&
							MEM_RO == regions[roff].mode){
						if(clean == false){
							clean = true;
                            if(curr_size < diffsize){
                                thediff[curr_size].to = curr - 1;
                                curr_size = curr_size + 1;
                            }
						} else {
							clean = true;
						}
					} else {
						thediff[curr_size].to = curr - 1;
						curr_size = curr_size + 1;
						if(curr_size < diffsize){
							thediff[curr_size].mode = MEM_RO;
							thediff[curr_size].from = curr;
							clean = false;
						}
					}

					curr_mode = MEM_RO;

				}

			}

		}
		prev = curr;
		curr = curr + PAGE_SIZE;
	}

	// Close last region
	if(clean == false && curr_size < diffsize){
		thediff[curr_size].to = curr - 1;
		curr_size = curr_size + 1;
	}

	close(fd);

	sigaction(SIGSEGV, &prev_act, 0);

	return curr_size;
}

void print_regions (struct memregion * regions, int numregions, unsigned int size) {
	printf("There are %i regions\n", numregions);

	int i = 0;

	for (i = 0; i < numregions && i < size; i++) {
		printf("0x%08x - 0x%08x ", regions[i].from, regions[i].to);
		if (regions[i].mode == MEM_RO) {
			printf("RO\n");
		} else if (regions[i].mode == MEM_RW) {
			printf("RW\n");
		} else {
			printf("NO\n");
		}
	}
	printf("\n");
}
