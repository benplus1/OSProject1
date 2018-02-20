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

char *my_itoa(int num, char *str)
{
    if(str == NULL)
    {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

int func1(){
    my_pthread_mutex_lock(&mutex2);
    FILE *fp, *fp2;
    char buff[255];
    fp = fopen("test.txt", "r");
    fp2 = fopen("test_copy.txt", "w");
    fscanf(fp, "%s", buff);
    printf("value %s\n", buff);
    int val = atoi(buff);
    val++;
    delay(100000);
    my_itoa(val,buff);
    fprintf(fp2,"%s\n",buff);
    fclose(fp);
    fclose(fp2);
    remove("test.txt");
    rename( "test_copy.txt", "test.txt" );
    my_pthread_mutex_unlock(&mutex2);
}

void delay(int milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
    now = clock();
}

int main(int argc, char ** argv){
    init();
    //my_pthread_mutex_init(&mutex, NULL);
    my_pthread_mutex_init(&mutex2, NULL);
    int k;
    //my_pthread_t * arr=(my_pthread_t *)malloc(sizeof(my_pthread_t)*50);
    //my_pthread_mutex_lock(&mutex);
    for (k=1;k<=100;k++){
        my_pthread_t  thread;
        //int * i=12;
        my_pthread_create(&thread,NULL,(void*)&func1,NULL);
        my_pthread_join(thread, NULL);
    }
    printf("HELLO THERE!\n");

}
