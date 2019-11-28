#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>

#define SZ (1 << 15)

extern char etext, edata, end; /*see man(3) end*/
char buf[8192];

int func(int val)
{
   if(val == 0){
       void *sp;
       asm volatile("mov %%rsp, %0;"
                     :"=r"(sp)
                     ::"memory");
       printf("SP = %p\n", sp);
       return 1;
   }
   val *= func(val-1);
   return val;
}

int main()
{
   int result;
   void *oldbrk, *address, *oldsp;
   oldbrk = sbrk(SZ);
   assert(oldbrk != (void *)(-1));
   address = oldbrk + SZ;
   address = &end;
   asm volatile("mov %%rsp, %0;"
                    "mov %1, %%rsp;"
                    :"=m" (oldsp)
                    :"m" (address)
                    : "memory", "rsp"
                    );
   // oldsp = SP, SP = address
   result = func(5);
   printf("%d\n", result);
   asm volatile("mov %0, %%rsp;"
                     :
                     :"r" (oldsp)
                     :"memory", "rsp"
                   );
   // SP = oldsp
}
/*
  In this program, the stack is changed before invoking func. Write a single line about the program behavior.  
*/
