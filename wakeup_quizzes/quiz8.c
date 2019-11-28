#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
sem_t s1, s2;
int counter = 0;

void* thfunc(void *arg)
{
     sem_post(&s1);
     sleep(1);
     counter++;
     printf("T1: %d\n", counter);
     sem_wait(&s2);
     printf("T2: %d\n", counter);
     return NULL;
}
int main(void)
{
   pthread_t tid;
   sem_init(&s1, 0, 0);
   sem_init(&s2, 0, 0);
   if(pthread_create(&tid, NULL, thfunc, NULL) != 0){
              perror("pthread_create");
              exit(-1);
   }
   sem_post(&s2); 
   counter++;
   printf("M1: %d\n", counter);
   sem_wait(&s1);  
   printf("M2: %d\n", counter);
   pthread_join(tid, NULL);
}
/*Write down all possible outputs of the above program?*/
