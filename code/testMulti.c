#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<ucontext.h>
#include<unistd.h>
#include<sys/time.h>
#include<time.h>
#include "my_pthread.c"

my_pthread_mutex_t mutex;
int counter=0;

int func1(int y){
    long int avgA = 0;
	int a;
	for(a = 1 ; a <= 100 ; a++) {				//each workload is done 100 times
		//printf("a = %d\n",a);
		//struct timeval tv;
		//suseconds_t start;
		//gettimeofday(&tv , NULL);
		//start = tv.tv_usec;//the time when the workload starts is saved
		char* ptrs[200];
		int i;
		for(i = 0; i< 200;i++){
			ptrs[i] = malloc(1);
            if(ptrs[i] == NULL){
                printf("run #%d in test iteration %d of thread %d failed to malloc\n",i,a,y);
            }else{

            printf("mallocing %d for the %d time in iteration %d\n", y,i,a);
		}
    }

		for(i = 0; i < 200;i++){
            printf("freeing %dth pointer\n", i);
			free(ptrs[i]);
		}
		//char *p = (char*)myallocate(100*size,page);
		//*p='c';
		//printf ("%c\n", *p);
		//mydeallocate(p,page);
		//suseconds_t end;
		//gettimeofday(&tv , NULL);
		//end = tv.tv_usec;				//the end time is saved

		//avgA = (end - start) + avgA;			//this finds the total run time and saved to a total

	}

	//avgA = avgA / 100;					//this finds the average of each run of the workload

	printf("Average time of workload %d : %ld microseconds\n" ,y,avgA);
	counter++;
	return 0;
}



int main(int argc, char ** argv){
	//init_memory();
	my_pthread_t t1,t2,t3,t4,t5,t6;
	int a=1;
    int b=2;
    int c=3;
    int d=4;
    int e=5;
    int f=6;

	pthread_create(&t1,NULL,(void*)&func1,a);
	/*pthread_create(&t2,NULL,(void*)&func1,b);
	pthread_create(&t3,NULL,(void*)&func1,c);
    pthread_create(&t4,NULL,(void*)&func1,d);
	pthread_create(&t5,NULL,(void*)&func1,e);
	pthread_create(&t6,NULL,(void*)&func1,f);*/
	pthread_join(t1,NULL);
    	/*pthread_join(t2,NULL);
    	pthread_join(t3,NULL);
    	pthread_join(t4,NULL);
    	pthread_join(t5,NULL);
    	pthread_join(t6,NULL);*/

	printf("the counter is %d\n", counter);
	//printf("Param T0: %d %lx\n",*arr[0],arr[0]);
	//pthread_create(&t2,NULL,(void*)&func2,w);
}
