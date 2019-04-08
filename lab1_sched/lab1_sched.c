/*
*	DKU Operating System Lab
*	    Lab1 (Scheduler Algorithm Simulator)
*	    Student id : 32161796
*	    Student name : Park_Jung_Sun
*
*   lab1_sched.c :
*       - Lab1 source file.
*       - Must contains scueduler algorithm function'definition.
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

PCB* process;
QUEUE* ready_queue;
int set = 0;
int schedul_time = 0;
int schedul_mode;
int level_queue;
char **result_picture;
int draw_point = -1;
int total_time = 100;
int TIME_SLICE = 1;

void scheduling(int mode){
	int run = 0, count = 0; // run ->schedul entitiy, count-> time_slice check
	init_ready_queue(mode); //init
	init_PCB();
	init_result();
	while(!is_arrival_new()) schedul_time++; //first schedul
	run = output_ready_queue();
	while(1){
    input_result(run);
		if(process[run].response_time == -1){ // arrival_time check
			process[run].response_time = schedul_time - process[run].arrival_time;
		}
		process[run].remain_time--;
		schedul_time++; count++;
		is_arrival_new();
		if(process[run].remain_time ==0){ // complete process
			process[run].turnarround_time = schedul_time - process[run].arrival_time;
			process[run].is_complete = 1;
			if(!is_all_finished()){ // all process complete
				break;
			}
		  run = output_ready_queue();
			while(run==-1) run = queue_empty();
			count = 0;
		}else if(schedul_mode == MLFQ_MODE && count == TIME_SLICE){ // if MLFQ, increase level 
			if((!process_is_alone()) && process[run].level<MLFQ_LEVEL-1)
				process[run].level++;
		}
		if((schedul_mode >= 10)&&count == TIME_SLICE){ // if (RR,STCF,MLFQ,LOTTERY), time slice
			input_ready_queue(run);
			run = output_ready_queue();
			while(run==-1) run = queue_empty();
			count = 0;
		}
	}
	del_ready_queue();
}
void init_workload(){  // init workload
	int i=0;
	printf("please, sum of service_time's maximum is 100 (if total service >100, error)\n");
	printf("input number of processes : ");
	scanf("%d", &set);
	process = malloc(sizeof(PCB)*set);
	result_picture = (char**)malloc(sizeof(char*)*set);
	for(i=0;i<set;i++){
		printf("input %c process workload(arrival_time, service_time, ticket) : ", i+65);
		scanf("%d %d %d", &process[i].arrival_time,&process[i].service_time,&process[i].ticket);
	}
	printf("TIME_SLICE : ");
	scanf("%d",&TIME_SLICE);
	for(i=0;i<set;i++){
		result_picture[i] = (char*)malloc(sizeof(char)*total_time);
	}
}
void init_PCB(){ // init PCB
	int i =0;
	for(i=0;i<set;i++){
		process[i].remain_time = process[i].service_time;
		process[i].turnarround_time = 0;
		process[i].response_time = -1;
		process[i].is_complete =0;
		process[i].level = 0;
	}
}
void init_ready_queue(int mode){ // init queue
	int i =0,k;
	schedul_time = 0;
	schedul_mode = mode;
	if(schedul_mode == MLFQ_MODE){ // MLFQ MODE?
		level_queue = MLFQ_LEVEL;
	}else{
		level_queue = SINGLE_LEVEL;
	}
	ready_queue = malloc(sizeof(QUEUE)*level_queue);
	for(i =0;i<level_queue;i++){ // INIT QUEUE
		ready_queue[i].front = ready_queue[i].rear = 0;
		ready_queue[i].size = set+1;
		ready_queue[i].index = malloc(sizeof(int)*ready_queue[i].size);
		for(k=0;k<QUEUE_SIZE+1;k++){
			ready_queue[i].index[k] = -1;
		}
	}
}
void input_ready_queue(int index){ //INPUT SCHEDUL ENTITIY IN QUEUE
	int i, in_index;
	i = process[index].level;
	ready_queue[i].rear = (ready_queue[i].rear+1)%ready_queue[i].size;
	in_index = ready_queue[i].rear;
	ready_queue[i].index[in_index] = index;
}
int output_ready_queue(){ // OUTPUT SCHEDUL ENTITIY FROM  QUEUE
	int i = 0, out_index;
	if(schedul_mode==MLFQ_MODE){ // QUEUE LEVEL SELECT
		for(i=0;i<MLFQ_LEVEL;i++){
			if(!queue_is_empty_full(i)){
				break;
			}
		}
		if(i==MLFQ_LEVEL){
			i--;
		}
	}
	if(queue_is_empty_full(i)){ // IF QUEUE IS EMPTY
		return -1;
	}
	ready_queue[i].front = (ready_queue[i].front+1)%ready_queue[i].size;
	if((schedul_mode%10 == 1)&&(!queue_is_empty_full(i))){  // IF SJF, STCF SWAP SCHEDUL ENTITY
		int start, end, shortest,time1,time2,temp;
		start = shortest = ready_queue[i].front;
		end = ready_queue[i].rear;
		do{
			start=(start+1)%ready_queue[i].size;
			time1 = ready_queue[i].index[shortest];
			time2 = ready_queue[i].index[start];
			if(process[time1].remain_time > process[time2].remain_time){
				shortest = start;
			}
		}while(start!=end);
		temp = ready_queue[i].index[shortest];   // SHORTEST AND FRONT CHANGE
		ready_queue[i].index[shortest] = ready_queue[i].index[ready_queue[i].front];
		ready_queue[i].index[ready_queue[i].front] = temp;
	}
	if((schedul_mode==LOTTERY_MODE)&&(!queue_is_empty_full(i))){  // IF LOTTERY
		int total_ticket,select =0,start,end,index,temp;
		total_ticket = 0;
		start = ready_queue[i].front;
		end = (ready_queue[i].rear+1)%ready_queue[i].size;
		for(;start!=end;start=(start+1)%ready_queue[i].size){  // SUM TOTAL TICKET 
			index = ready_queue[i].index[start];
			total_ticket += process[index].ticket;
		}
		srand((unsigned)time(NULL));
		select = rand()%total_ticket;		// RANDOM SELECT
		total_ticket = 0;
		start = ready_queue[i].front;
		end = (ready_queue[i].rear+1)%ready_queue[i].size;
		for(;start!=end;start=(start+1)%ready_queue[i].size){ //LOTTERY 
			index = ready_queue[i].index[start];
			total_ticket += process[index].ticket;
			if(select < total_ticket){
				select = start;
				break;
			}
		}
		temp = ready_queue[i].index[select];
		ready_queue[i].index[select] = ready_queue[i].index[ready_queue[i].front];
		ready_queue[i].index[ready_queue[i].front] = temp;
		select = 0;
	}
	out_index = ready_queue[i].front;
	return ready_queue[i].index[out_index];
}
void del_ready_queue(){  // DEL QUEUE
	int i =0;
	for(i=0;i<level_queue;i++){
		free(ready_queue[i].index);
	}
}
int is_arrival_new(){ // IF NEW ENTITY ARRIVE, INPUT QUEUE
	int i = 0, count =0;
	for(i = 0;i<set;i++){
		if(process[i].arrival_time == schedul_time){
			input_ready_queue(i);
			count++;
		}
	}
	draw_point++;
	return count;
}
int is_all_finished(){ // IS ALL ENTITY FINISHED?
	int i=0;
	int count = 0;
	for(i=0;i<set;i++){
		if(process[i].is_complete == 1)
			count++;
	}
	if(count == set)
		return 0;
	return 1;
}
int queue_is_empty_full(int i){ // QUEUE IS EMPTY
	if(ready_queue[i].front == ready_queue[i].rear){
		return 1;
	}else{
		return 0;
	}
}
int process_is_alone(){ // PROCESS IS ALONE
	int i, count =0;
	for(i=0;i<level_queue;i++){
		if(queue_is_empty_full(i)){
			count++;
		}
	}
	if(count == i){
		return 1;
	}
	return 0;
}
void workload_print(){ // PRINT WORKLOAD
	int i = 0;
	printf("|------------workload-------------|\n");
	printf("|___|_arrival_time_|_service_time_|\n");
	for(i=0;i<set;i++){
		printf("|_%c_|_%12d_|_%12d_|\n", i+65, process[i].arrival_time, process[i].service_time);
	}
}
void result_print(char* text){ //PRINT PCB
	int i=0;
	printf("\n\n\n|%-41s|\n",text);
	printf("|___|_turnarround_time_|___respose_time___|\n");
	for(i=0;i<set;i++){
		printf("|_%c_|_%16d_|_%16d_|\n", i+65, process[i].turnarround_time, process[i].response_time);
	}
	picture_print();
}
void init_result(){ //INIT SCHEDULING PICTURE
	int i =0,k=0;
	draw_point =-1;
	for(k=0;k<set;k++){
		for(i=0;i<total_time;i++){
			result_picture[k][i]=45;
		}
	}
}
void input_result(int i){
	result_picture[i][draw_point]=i+65;
}
void picture_print(){
	int i=0;
	printf("|Scheduling Picture|\n");
	for(i=0;i<set;i++){
		result_picture[i][schedul_time] = '\0';
		printf("|_%c_| %s \n",i+65,result_picture[i]);
	}
}
int queue_empty(){ // SCHEDULING ENTITY IS NONE
	schedul_time++;
	is_arrival_new();
	return output_ready_queue();
}
