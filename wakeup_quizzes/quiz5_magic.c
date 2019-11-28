#include<stdio.h>
#include<stdlib.h>
#include<signal.h>

unsigned num1 = 0, num2 = 5;

void sighandler(int signo)
{
    printf("My bad! I initialized wrongly\n");
    num1 = 1;
}

main()
{
   unsigned sum = 0;
   if(signal(SIGFPE, &sighandler) < 0)  // Register a signal handler for division-by-zero
      perror("signal");
   
   while(num1 != num2){
       asm volatile(
                    "xor %%rdx, %%rdx;"
                    "movl %1, %%eax;"
                    "divl %2;"
                    "movl %0, %%ecx;"
                    "add %%eax, %%ecx;"
                    "movl %%ecx, %0;"
                    "movl %2, %%eax;"
                    "add $1, %%eax;"
                    "movl %%eax, %2;"
                    : "+m" (sum)
                    : "m" (num2), "m"(num1)
                    :"rax", "rcx", "memory"
                   );
       //sum += num2/num1;
       //num1++;
   }
  printf("sum = %u\n", sum);
}

/*What will be the output of the program? Give an one line explanation.*/
