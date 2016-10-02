#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <fcntl.h>

// Boolean Type Defintion
typedef int bool;
#define true 1
#define false 0

#include "memlayout.h"

extern int PAGE_SIZE;

sigjmp_buf env;

void sigsev_handler(int sig) {
	siglongjmp(env, 2);
}

int get_mem_layout (struct memregion * regions, unsigned int size) {
	int fd = open("/dev/zero", O_RDONLY);
	char sample;
	
    int curr_size = 0;
    char * curr = 0;
    char * prev = 0;

    struct sigaction act;
    struct sigaction prev_act;

    struct memregion next_region;
    struct memregion curr_region;
    
	act.sa_handler = sigsev_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGSEGV, &act, &prev_act);

	bool check = true;
    bool closed_region = false;

	if(sigsetjmp(env,1) == 2) {
        curr_region.mode = MEM_NO;
        curr_region.from = 0;
		check = false;
	}

	if (check){
		sample = *curr;

		if (read(fd, curr, 1) == 1) {
			*curr = sample;

            curr_region.mode = MEM_RW;
            curr_region.from = 0;
		} else {
            curr_region.mode = MEM_RO;
            curr_region.from = 0;
		}
	}
    


    while(prev <= curr && (uintptr_t) curr < (uint64_t) 0xFFFFFFFF) {

        check = true;
        closed_region = false;

		if(sigsetjmp(env,1) == 2) {
			if (curr_region.mode != MEM_NO) {
                next_region.mode = MEM_NO;
                closed_region = true;
			}

			check = false;
		}

		if(check){
			sample = *curr;

			if (read(fd, curr, 1) == 1) {
				// can read.. MEM_RW
				*curr = sample;
				
				if (curr_region.mode != MEM_RW) {
                    next_region.mode = MEM_RW;
                    closed_region = true;
                }
			} else {
				// MEM_RO
                if (curr_region.mode != MEM_RO) {
                    next_region.mode = MEM_RO;
                    closed_region = true;
                }
			}
		}
        
        if(closed_region){
            curr_region.to = curr - 1;
            next_region.from = curr;

            if(curr_size < size){
                regions[curr_size] = curr_region;
            }

            curr_region = next_region;
            curr_size = curr_size + 1;
        }

        prev = curr;
        curr = curr + PAGE_SIZE;
    }

    // Close last region
    curr_region.to = curr - 1;
    next_region.from = curr;

    if(curr_size < size){
        regions[curr_size] = curr_region;
    }
    curr_size = curr_size + 1;

    // Clean Up
    close(fd);

	sigaction(SIGSEGV, &prev_act, 0);

	return curr_size;
}

int get_mem_diff (struct memregion * regions, unsigned int howmany,
		struct memregion * thediff, unsigned int diffsize) {

    int fd = open("/dev/zero", O_RDONLY);
	char sample;
	
    int curr_size = 0;
    char * curr = 0;
    char * prev = 0;

    struct sigaction act;
    struct sigaction prev_act;

    struct memregion next_region;
    struct memregion curr_region;
    
    int roff = 0;

	act.sa_handler = sigsev_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGSEGV, &act, &prev_act);

	bool check = true;
    bool closed_region = false;

	if(sigsetjmp(env,1) == 2) {
        curr_region.mode = MEM_NO;
        curr_region.from = 0;
		check = false;
	}

	if (check){
		sample = *curr;

		if (read(fd, curr, 1) == 1) {
			*curr = sample;

            curr_region.mode = MEM_RW;
            curr_region.from = 0;
		} else {
            curr_region.mode = MEM_RO;
            curr_region.from = 0;
		}
	}
    

    while(prev <= curr && (uintptr_t) curr < (uint64_t) 0xFFFFFFFF){

        closed_region = false;
		check = true;
		if(sigsetjmp(env,1) == 2) {
			if (curr_region.mode != MEM_NO) {
                next_region.mode = MEM_NO;
                closed_region = true;
			}

			check = false;
		}

		if(check){
			sample = *curr;

			if (read(fd, curr, 1) == 1) {
				// can read.. MEM_RW
				*curr = sample;
				
				if (curr_region.mode != MEM_RW) {
                    next_region.mode = MEM_RW;
                    closed_region = true;
                }
			} else {
				// MEM_RO
                if (curr_region.mode != MEM_RO) {
                    next_region.mode = MEM_RO;
                    closed_region = true;
                }
			}
		}
        
        if(closed_region){
            curr_region.to = curr - 1;
            next_region.from = curr;

            while(roff + 1 < howmany 
                    && regions[roff + 1].from - 1 < curr_region.from){
                roff = roff + 1;
            }

            if (regions[roff].from != curr_region.from
                    || regions[roff].to != curr_region.to
                    || regions[roff].mode != curr_region.mode){
                  
                if(curr_size < diffsize){
                    thediff[curr_size] = curr_region;
                }

                curr_size = curr_size + 1;
            }

            curr_region = next_region;
        }

        prev = curr;
        curr = curr + PAGE_SIZE;
    }

    // Close last region
    curr_region.to = curr - 1;
    next_region.from = curr;

    while(roff + 1 < howmany 
            && regions[roff + 1].from - 1  < curr_region.from){
        roff = roff + 1;
    }

    if (regions[roff].from != curr_region.from
            || regions[roff].to != curr_region.to
            || regions[roff].mode != curr_region.mode){

        if(curr_size < diffsize){
            thediff[curr_size] = curr_region;
        }

        curr_size = curr_size + 1;
    }

    // Clean Up
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
