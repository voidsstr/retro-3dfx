/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** get env vars from prefs
**
*/


#pragma optimize ("",off)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hrm_3d_envvars.h"

#include <3dfx.h>

#define kMaxEnvVarLen 100
#define kMaxEnvValLen 256
typedef struct {
    char envVariable[kMaxEnvVarLen];
    char envValue[kMaxEnvValLen];
    void *nextVar;
} sst1InitEnvVarStruct;

sst1InitEnvVarStruct *envVarsBase = (sst1InitEnvVarStruct *) NULL;

static int bufferFgets(char *, char **);
static int bufferFgetc(char **);
static int parseFieldCfg(char *);
static void stringToLower(char *string);


static FxBool checkedPrefs = FXFALSE;







/*______________________________________________________________________________

	getpreferences
	
	Request the env vars from the HRM, using a preference text stream. The
	variables are stored localy, but without any real parsing. They are
	available for future referencing (using getenv).

*/

FX_ENTRY FxBool FX_CALL getpreferences()
{
  static FxBool retVal = FXFALSE;
  char * thePrefsStream;
  char buffer[1024];
  int inCfg, inDac;
  
  if (checkedPrefs) goto __errExit;
  
  hrm_Register3DClient( kHRM_Glide_Client );
  hrm_GetGlideEnvVars( 0, &thePrefsStream );

  inCfg = inDac = 0;
  while( bufferFgets( buffer, &thePrefsStream ) ) {
    buffer[strlen(buffer)-1] = (char) NULL;
    if(!strcmp(buffer, "[VOODOO]")) {
      inCfg = 1; inDac = 0;
      continue;
    } else if(buffer[0] == '[') {
      inCfg = 0; inDac = 0;
      continue;
    }

    if(inCfg) {
      if(!parseFieldCfg(buffer)) {
        retVal = FXFALSE;
        break;
      }
    } 
  }
 
__errExit:
  checkedPrefs = FXTRUE;
  return retVal;
}





/*______________________________________________________________________________

	bufferFgets
	
	sets the string pointer to the next valid line. comments are skipped.

*/

static int bufferFgets(char *string, char **stream)
{
  int validChars = 0;
  char *ptr = string;
  int charRead;

  while(0 != ((charRead = bufferFgetc(stream))))
  {
    *ptr++ = (char) charRead;
    validChars++;
    if(charRead == '\n' || charRead == '\r')
    {
      *ptr++ = (char) NULL;
      break;
    }
  }
  return(validChars);
}





/*______________________________________________________________________________

	bufferFgetc
	
	sets the char pointer to the next valid char. comments are skipped.

*/

static int bufferFgetc(char ** stream)
{
  static int column = 0;
  static int validChars = 0;
  int charRead, charReadL;
  int inComment;

  inComment = 0;
  while(1)
  {
    charRead = **stream;
    (*stream)++;
    
    if(inComment == 1)
    {
      if(charRead <= 0)
        return(0);
      else if(charRead == '\n' || charRead == '\r')
        inComment = 0;
      column = 0;
      validChars = 0;
      continue;
    }
    else if(column == 0 && charRead == '#')
    {
      /* Comment line */
      inComment = 1;
      column = 0;
      validChars = 0;
    }
    else if(charRead <= 0)
    {
      return(0);
    }
    else
    {
      if(charRead == '\n' || charRead == '\r')
      {
        if(validChars > 0)
        {
          validChars = 0;
          column = 0;
          return(charRead);
        }
        else
          continue;
      }
      else
      {
        if(isspace(charRead))
          continue;
        validChars++;
        column++;
        charReadL = (islower(charRead)) ? toupper(charRead) : charRead;
        return(charReadL);
      }
    }
  }
}





/*______________________________________________________________________________

	parseFieldCfg
	
	.

*/

static int parseFieldCfg(char *string)
{
  char *envName, *envVal;
  sst1InitEnvVarStruct *envVarsPtr;

  if((envName = strtok(string, "=")) == NULL)
    return(0);
  if((envVal = strtok((char *) NULL, "=")) == NULL)
  /* Valid environment variable, NULL value */
    return(1);

  /* canonical form is now lower case */
  stringToLower(envName);

  if(envVarsBase == (sst1InitEnvVarStruct *) NULL)
  {
    if((envVarsPtr = malloc(sizeof(sst1InitEnvVarStruct))) ==
                                                 (sst1InitEnvVarStruct *) NULL)
      return(0);
    envVarsBase = envVarsPtr;
  }
  else
  {
    envVarsPtr = envVarsBase;
    while(1)
    {
      if(envVarsPtr->nextVar == (sst1InitEnvVarStruct *) NULL)
        break;
      else
        envVarsPtr = envVarsPtr->nextVar;
    }
        
    if((envVarsPtr->nextVar = malloc(sizeof(sst1InitEnvVarStruct))) ==
                                                  (sst1InitEnvVarStruct *) NULL)
      return(0);
    envVarsPtr = envVarsPtr->nextVar;
  }
  
  envVarsPtr->nextVar = (sst1InitEnvVarStruct *) NULL;
  strcpy(envVarsPtr->envVariable, envName);
  strcpy(envVarsPtr->envValue, envVal);

  return(1);
}






/*______________________________________________________________________________

	stringToLower
	
	converts string to lower case.

*/

static void stringToLower(char *string)
{
  char *ptr = string;

  while(*ptr)
  {
    *ptr = (isupper(*ptr)) ? tolower(*ptr) : *ptr;
    ptr++;
  }
}





/*______________________________________________________________________________

	getenv
	
	If the actual environment variable exists (determined by a call to
	the system getenv() routine), then that pointer is returned.  Otherwise,
	if the variable is defined preferences from the HRM.  Otherwise,
	NULL is returned

*/

FX_ENTRY char* FX_CALL getenv(const char *string)
{
  const char* retVal = NULL;
  
  if (!checkedPrefs)
  {
    static FxBool inProc = FXFALSE;
  	
    if (!inProc)
    {
      inProc = FXTRUE;
      getpreferences();
      inProc = FXFALSE;
    }
  }
    
  {
    sst1InitEnvVarStruct *envVarsPtr = envVarsBase;
    char tempSearchString[kMaxEnvVarLen];

    /* Put the search into canonical form */
    strcpy(tempSearchString, string);
    stringToLower(tempSearchString);

    while(envVarsPtr)
    {
      if(!strcmp(tempSearchString, envVarsPtr->envVariable))
      {
        retVal = envVarsPtr->envValue;
        break;
      }
      envVarsPtr = (sst1InitEnvVarStruct *) envVarsPtr->nextVar;
    }
  }

  return (char*)retVal;
}


#pragma optimize ("",on)
