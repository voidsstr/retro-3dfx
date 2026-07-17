#include <stdio.h>
#include <unistd.h>

int main(void)
{
  printf("This isn't a real diag. It's just a \"diag\" for run_top's sake\n");

  while(1)
    sleep(10000);

  return(0);	
}
