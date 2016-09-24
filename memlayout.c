#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <fcntl.h>

extern PAGE_SIZE;

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

	while((uint32_t) curr >= (uint32_t) prev) {
		check = true;
		if(sigsetjmp(env,1) == 2) {
			if (curr_mode != MEM_NO) {
				curr_size = curr_size + 1;
				curr_mode = MEM_NO;
				
				if (curr_size - 1 < size) {
					regions[curr_size].to = curr-1;

					if(curr_size < size) {
						regions[curr_size].to = curr;
						regions[curr_size].mode = MEM_NO;
					}
				}
			}

			check = false;
		}

		if(check){
			check = false;
			sample = *curr;

			if (read(fd, curr, 1) == 1) {
				// can read.. MEM_RW
				*curr = sample;
				if (curr_mode != MEM_RW) {
					curr_size = curr_size + 1;
					curr_mode = MEM_RW;

					if (curr_size < size) {
						regions[curr_size].to = curr - 1;

						if (curr_size < size) {
							regions[curr_size].to = curr;
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
						regions[curr_size].to = curr - 1;

						if(curr_size < size) {
							regions[curr_size].to = curr;
							regions[curr_size].mode = MEM_RO;
						}
					}
				}
			}
		}

		prev = curr;
		curr = curr + PAGE_SIZE;
	}
	close(fd);

	sigaction(SIGSEGV, &prev_act, 0);

	return curr_size;

}

int get_mem_diff (struct memregion * regions, unsigned int howmany,
					struct memregion * thediff, unsigned int diffsize) {
}

void print_regions (struct memregion * regions, int numregions, unsigned int size) {
	printf("There are %i many regions\n", numregions);

	for (int i = 0; i < numregions || i < size; i++) {
		printf("0x%p - 0x%p ", regions[i].from, regions[i].to);
		if (regions[i].mode == MEM_RO) {
			printf("RO\n");
		} else if (regions[i] == MEM_RW) {
			printf("RW\n");
		} else {
			printf("NO\n");
		}
	}
}