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
my_pthread_mutex_t mutex2;
int test=0;


int func2(int num){
  int i;
  for(i = 0; i<num; i++){
    int z;
    for (z = 0; z < 50000000*num; z++){}
    printf("currently %d is doing %d\n",num,i);


  }
  printf("%d is done\n",num);
  return i;
}

int main(int argc, char ** argv){

    my_pthread_t  thread1;
    my_pthread_t  thread2;
    my_pthread_t  thread3;
    my_pthread_t  thread4;
    my_pthread_t  thread5;
    my_pthread_t  thread6;
    my_pthread_t  thread7;
    my_pthread_t  thread8;
    my_pthread_t  thread9;
    my_pthread_t  thread10;
    my_pthread_t  thread11;
    my_pthread_t  thread12;
    my_pthread_t  thread13;
    my_pthread_t  thread14;
    my_pthread_t  thread15;
    my_pthread_t  thread16;
    my_pthread_t  thread17;
    my_pthread_t  thread18;

    pthread_create(&thread1,NULL,(void*)&func2,1);
    pthread_create(&thread2,NULL,(void*)&func2,2);
    pthread_create(&thread3,NULL,(void*)&func2,3);
    pthread_create(&thread4,NULL,(void*)&func2,4);
    pthread_create(&thread5,NULL,(void*)&func2,5);
    pthread_create(&thread6,NULL,(void*)&func2,6);
    pthread_create(&thread7,NULL,(void*)&func2,7);
    pthread_create(&thread8,NULL,(void*)&func2,8);
    pthread_create(&thread9,NULL,(void*)&func2,9);
    pthread_create(&thread10,NULL,(void*)&func2,10);
    pthread_create(&thread11,NULL,(void*)&func2,11);
    pthread_create(&thread12,NULL,(void*)&func2,12);
    pthread_create(&thread13,NULL,(void*)&func2,13);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    pthread_join(thread5, NULL);
    pthread_join(thread6, NULL);
    pthread_join(thread7, NULL);
    pthread_join(thread8, NULL);
    pthread_join(thread9, NULL);
    pthread_join(thread10, NULL);
    pthread_join(thread11, NULL);
    pthread_join(thread12, NULL);
    pthread_join(thread13,NULL);



    printf("HELLO THERE!\n");
    //while(1==1);

}
