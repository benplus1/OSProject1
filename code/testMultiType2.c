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

int func1(int y){
    printf("Test Case F----------------------------------------------------------------------\n");
    int f;
    long int average_F = 0;
    char* string;
    string = (char*)malloc(sizeof(char)*100);		// 10 byte memory space malloced in each of 10 char * pointer
    for(f = 1; f<=100000; f++){
        struct timeval tv;
        suseconds_t start;
        gettimeofday(&tv , NULL);
        start = tv.tv_usec;					 //save start of time

        //10 char * alloted
        int x;

        if(string == NULL) break;
        int i;
        char l = 'A' + (random() % 26);				// Random Uppercase letter generated
        for(i = 0; i<= 97;i++){
            *(string+i) = l;				// Random Uppercase letter placed in malloced memory 9 times.
        }
        // printf("here\n" );
        sprintf((string+98), "%d",y);
        *(string+99) = '\0';					// Null byte placed  at the end of string.
        printf("String is %s on iteration %d of thread %d\n", string,f,y);			//printing strings
        				//freeing strings
        //str = (char*)"abc56efg8\0";
        //printf("malloc'd and initialized strings\n");
        /*for (x=0; x<100; x++) {
        //printf("String is %s\n",strings[x]);  		//printing string stored
        free((void*)strings[x]);				//freeing string
    }*/
    //printf("printed and free'd strings\n");
    suseconds_t end;
    gettimeofday(&tv , NULL);
    end = tv.tv_usec;					// end of time interval

    average_F = (end - start) + average_F;				// summ of all time interval
}
free((void*)string);
average_F = average_F / 100000;			//calculate average time interval
printf("Average time of thread %d: %ld microseconds\n" ,y,average_F);
return 0;
}



int main(int argc, char ** argv){
    init_memory();
    my_pthread_t t1,t2,t3,t4,t5,t6;
    int a=1;
    int b=2;
    int c=3;
    int d=4;
    int e=5;
    int f=6;

    pthread_create(&t1,NULL,(void*)&func1,a);
    pthread_create(&t2,NULL,(void*)&func1,b);
    pthread_create(&t3,NULL,(void*)&func1,c);
    pthread_create(&t4,NULL,(void*)&func1,d);
    pthread_create(&t5,NULL,(void*)&func1,e);
    pthread_create(&t6,NULL,(void*)&func1,f);
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_join(t3,NULL);
    pthread_join(t4,NULL);
    pthread_join(t5,NULL);
    pthread_join(t6,NULL);
    //printf("Param T0: %d %lx\n",*arr[0],arr[0]);
    //pthread_create(&t2,NULL,(void*)&func2,w);
}
