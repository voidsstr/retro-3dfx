#include <stdio.h>
#include <time.h>

int main(void)
{
  char *dateString, *current;
  time_t t;
  
  t = time(NULL);
  dateString = ctime(&t);

  //Chop off the new-line from dateString
  current = dateString;
  while(*current != 0)
    {
      if(*current == '\n' || *current == '\r')
	*current=0;
      current++;
    }

  printf("#ifndef GENDATE_H\n");
  printf("#define GENDATE_H\n");	 
  printf("#define BUILD_DATE \"%s (from incsrc/gendate.h)\"\n", dateString);
  printf("#endif\n");

  return(0);
}
