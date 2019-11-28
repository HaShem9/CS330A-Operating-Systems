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
   if(signal(SIGFPE, &sighandler) < 0)  //Division-by-zero sighandler
      perror("signal");
   
   while(num1 != num2){
       sum += num2/num1;
       num1++;
   }
  printf("sum = %u\n", sum);
}
/* 
   Given that this program prints "My bad! I initialized wrongly!" infinitely,write your justification for this behavior (2 to 3 lines).
   Hint: Think about the exception (div-by-zero) handling mechanism. 
*/
