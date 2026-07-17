/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 2$ 
** $Date: 10/11/00 8:16:04 PM$ 
**
*/


#include <stdlib.h>
#include <stdio.h>
#include "3dfx.h"
#include "fxpci.h"
#include "version.h"


#include "errrpt.h"
#include "misc.h"
#include "init.h"


int main(int argc, char **argv) { 

  printf("[ 3dfx mfgdiag version %s, built on %s ]\n",
	mdc_version_str,
	mdc_builddate_str);

  mdc_init();
  /* add command line args to lua */

  mdc_args_to_lua(argc,argv);

  /* postinit modules, call lua... */

  mdc_run_luascript();
  mdc_exit();
  return 0;
}



