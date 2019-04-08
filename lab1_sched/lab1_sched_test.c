/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32161796
*	    Student name : Park_Jung_Sun
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm test code.
*
*/
#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "lab1_sched_types.h"

int main(int argc, char *argv[]){
	init_workload();
	workload_print();
	scheduling(FCFS_MODE);
	result_print("FCFS");
	scheduling(RR_MODE);
	result_print("RR");
	scheduling(SJF_MODE);
	result_print("SJF");
	scheduling(STCF_MODE);
	result_print("STCF");
	scheduling(MLFQ_MODE);
	result_print("MLFQ");
	scheduling(LOTTERY_MODE);
	result_print("LOTTERY");
	return 0;
}
