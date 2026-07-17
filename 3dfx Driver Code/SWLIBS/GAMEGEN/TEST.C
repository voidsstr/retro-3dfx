#include <3dfx.h>
#include <stdlib.h>
#include <stdio.h>
#include "gamegen.h"

void main(void)
{
   if (gg_read_db("c:\\dev3\\swlibs\\obsolete\\gw\\data\\models\\truck.gam")!=SUCCESS)
	   return;
   gg_convert_db();
}