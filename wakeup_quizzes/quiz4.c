#define SZ (1 << 15)

int func(int val)
{
   if(val == 0)
       return 1;
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
   save_and_update_sp(oldsp, address); // SP --> oldsp, address --> SP
   result = func(5);
   printf("%d\n", result);
   update_sp(oldsp); // oldsp --> SP
   
}
/*
  In this program, the stack is changed before invoking func. Write a single line about the program behavior.  
*/
