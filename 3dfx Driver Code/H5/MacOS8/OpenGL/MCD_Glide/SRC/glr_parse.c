#if SUPPORT_CONFIG_FILE

/*-*-c++-*-*/
/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:36:36 PM$ 
**
** Parsing code for grabbing information from "voodoo2.ini" initialization file
**
*/
#pragma optimize ("",off)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <MacTypes.h>
#include <Files.h>
#include <Folders.h>
#include <Processes.h>
#include <Strings.h>

#include "glr.h"

#if __WIN32__
#include <windows.h>

/* Path relative to HKEY_LOCAL_MACHINE */
#define REGSTR_PATH_3DFXSW              "Software\\3Dfx Interactive\\Voodoo2"
#define REGSTR_PATH_GLIDE               REGSTR_PATH_3DFXSW"\\Glide"
#endif /* __WIN32__ */

#include <3dfx.h>
#if 0
#include <cvgregs.h>
#include <cvgdefs.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <sst1vid.h>
#include <sst1init.h>
#endif

#define kMaxEnvVarLen 100
#define kMaxEnvValLen 256
typedef struct {
    char envVariable[kMaxEnvVarLen];
    char envValue[kMaxEnvValLen];
    void *nextVar;
} sst1InitEnvVarStruct;

sst1InitEnvVarStruct *envVarsBase = (sst1InitEnvVarStruct *) NULL;

static int sst1InitFgets(char *, short refNum);
static int sst1InitFgetc(short refNum);
static int sst1InitParseFieldCfg(char *);
static void sst1InitToLower(char *string);
static void sst1InitFixFilename(char *dst, char *src);

static FxBool checkedFileP = FXFALSE;

/*
** sst1InitVoodooFile():
**  Find and setup "voodoo2.ini" file if possible
**
**    Returns:
**      FxTRUE if "voodoo2.ini" file is found, opened with no errors, and
**             constains the dac programming data.
**      FXFALSE if cannot find file, error opening file, or has no dac data in it.
**
*/
FX_ENTRY FxBool FX_CALL sst1InitVoodooFile()
{
  static FxBool retVal = FXFALSE;

#ifndef DIRECTX
  int inCfg, inDac;
  //FILE *file = (FILE *) NULL;
  short refNum;
  char buffer[1024], filename[256];
  OSErr err = -1;

	filename[0] = '\0';
	if (checkedFileP) goto __errExit;
	
	{
		FSSpec iniSpec = {
			0, 0,
			"\p3dfxogl.var"
		};
		Boolean foundP = false;
		
		/* Check the app's directory */
		if (!foundP) {
			ProcessSerialNumber curApp;
			ProcessInfoRec appInfo;
			FSSpec appSpec;
			
			if (GetCurrentProcess(&curApp) != noErr) goto __errAppDir;

			/* We only care about the app's location */
			appInfo.processInfoLength = sizeof(ProcessInfoRec);
			appInfo.processName = NULL;
			appInfo.processAppSpec = &appSpec;
			if (GetProcessInformation(&curApp, &appInfo) != noErr) goto __errAppDir;
			
			{
				CInfoPBRec thePB;

				thePB.hFileInfo.ioCompletion = NULL;
				thePB.hFileInfo.ioNamePtr = iniSpec.name;
				thePB.hFileInfo.ioVRefNum = appSpec.vRefNum;
				thePB.hFileInfo.ioDirID = appSpec.parID;

				thePB.hFileInfo.ioFDirIndex = 0;

				foundP = ((PBGetCatInfoSync(&thePB) == noErr) &&
									((thePB.hFileInfo.ioFlAttrib & (0x01 << 4)) == 0));
				if (foundP) {
					iniSpec.vRefNum = appSpec.vRefNum;
					iniSpec.parID = appSpec.parID;
				}
			}			
			
		__errAppDir:
			;
		}
		
		/* Check the mac's version of the 'search path' */
		if (!foundP) {
			OSType folderList[] = { kPreferencesFolderType, kExtensionFolderType };
			int i;

			for(i = 0; i < sizeof(folderList) / sizeof(folderList[0]); i++) {
				short vRefNum;
				long dirId;
				
				if (FindFolder(kOnSystemDisk, folderList[i], false, 
											 &vRefNum, &dirId) == noErr) {
				
					CInfoPBRec thePB;
					
					thePB.hFileInfo.ioCompletion = NULL;
					thePB.hFileInfo.ioNamePtr = iniSpec.name;
					thePB.hFileInfo.ioVRefNum = vRefNum;
					thePB.hFileInfo.ioDirID = dirId;
					
					thePB.hFileInfo.ioFDirIndex = 0;
					
					foundP = ((PBGetCatInfoSync(&thePB) == noErr) &&
										((thePB.hFileInfo.ioFlAttrib & (0x01 << 4)) == 0));
					if (foundP) {
						iniSpec.vRefNum = vRefNum;
						iniSpec.parID = dirId;
						
						break;
					}
				}
			}
		}
		
		if (foundP) {
			short wdRefNum;
			long  wdDirId;
			
			/* Change working directories, just in case the app did something else */
			if (HGetVol(NULL, &wdRefNum, &wdDirId) != noErr) goto __errFile;
			if (HSetVol(NULL, iniSpec.vRefNum, iniSpec.parID) != noErr) goto __errFile;
			
			/* NB: We leave the name trashed after this */
			err = FSpOpenDF(&iniSpec, fsRdPerm, &refNum);
			glr_debug_printf("err: %d\n",err);
			//p2cstr(iniSpec.name);
			//err = fopen((const char*)iniSpec.name, "r");
			
			HSetVol(NULL, wdRefNum, wdDirId);
			
		__errFile:
			;
		}
	}

  //if(file == NULL) goto __errExit;
  if(err != 0) goto __errExit;
  
  inCfg = inDac = 0;
  while(sst1InitFgets(buffer, refNum)) {
    buffer[strlen(buffer)-1] = (char) NULL;
    glr_debug_printf("str: %s\n",buffer);
    if(!strcmp(buffer, "[OPENGL]")) {
      inCfg = 1; inDac = 0;
      continue;
    } else if(buffer[0] == '[') {
      inCfg = 0; inDac = 0;
      continue;
    }

    if(inCfg) {
      if(!sst1InitParseFieldCfg(buffer)) {
        retVal = FXFALSE;
        break;
      }
    } 
  }
  FSClose(refNum);
  //if (file != NULL) fclose(file);

__errExit:
	checkedFileP = FXTRUE;
#endif /* !DIRECTX */

  return retVal;
}

static void sst1InitFixFilename(char *dst, char *src)
{
    while(*src) {
        *dst++ = *src;
        if(*src == '\\')
            *dst++ = *src;
        src++;
    }
    *dst = (char) NULL;
}


static int sst1InitFgets(char *string, short refNum)
{
    int validChars = 0;
    char *ptr = string;
    int charRead;

    while(0 != ((charRead = sst1InitFgetc(refNum)))) {
        *ptr++ = (char) charRead;
        validChars++;
        if(charRead == '\n' || charRead == '\r') {
            *ptr++ = (char) NULL;
            break;
        }
    }
    return(validChars);
}

static int sst1InitFgetc(short refNum)
{
    static int column = 0;
    static int validChars = 0;
    int charRead, charReadL;
    int inComment;
    char c;
    long count;
    OSErr err;
    
    inComment = 0;
    while(1) {
        count = 1;
        err = FSRead(refNum,&count,&c);
        if(count == 1) {
          charRead = c;
        } else {
          charRead = -1;
        }    
        //charRead = fgetc(stream);
        if(inComment == 1) {
            if(charRead <= 0)
                return(0);
            else if(charRead == '\n' || charRead == '\r')
                inComment = 0;
            column = 0;
            validChars = 0;
            continue;
        } else if(column == 0 && charRead == '#') {
            /* Comment line */
            inComment = 1;
            column = 0;
            validChars = 0;
        } else if(charRead <= 0) {
                return(0);
        } else {
            if(charRead == '\n' || charRead == '\r') {
                if(validChars > 0) {
                    validChars = 0;
                    column = 0;
                    return(charRead);
                } else
                    continue;
            } else {
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

static int sst1InitParseFieldCfg(char *string)
{
    char *envName, *envVal;
    sst1InitEnvVarStruct *envVarsPtr;

    if((envName = strtok(string, "=")) == NULL)
        return(0);
    if((envVal = strtok((char *) NULL, "=")) == NULL)
        /* Valid environment variable, NULL value */
        return(1);

    /* .ini canonical form is now lower case */
    sst1InitToLower(envName);
    sst1InitToLower(envVal);

    if(envVarsBase == (sst1InitEnvVarStruct *) NULL) {
        if((envVarsPtr = glmMalloc(sizeof(sst1InitEnvVarStruct))) ==
          (sst1InitEnvVarStruct *) NULL)
            return(0);
        envVarsBase = envVarsPtr;
    } else {
        envVarsPtr = envVarsBase;
        while(1) {
            if(envVarsPtr->nextVar == (sst1InitEnvVarStruct *) NULL)
                break;
            else
               envVarsPtr = envVarsPtr->nextVar;
        }
        if((envVarsPtr->nextVar = glmMalloc(sizeof(sst1InitEnvVarStruct))) ==
           (sst1InitEnvVarStruct *) NULL)
            return(0);
        envVarsPtr = envVarsPtr->nextVar;
    }
    envVarsPtr->nextVar = (sst1InitEnvVarStruct *) NULL;
    strcpy(envVarsPtr->envVariable, envName);
    strcpy(envVarsPtr->envValue, envVal);

    return(1);
}


static void sst1InitToLower(char *string)
{
    char *ptr = string;

    while(*ptr) {
        *ptr = (isupper(*ptr)) ? tolower(*ptr) : *ptr;
        ptr++;
    }
}

/*
** sst1InitGetenv():
**  Getenv() for INIT routines.
**
**  If the actual environment variable exists (determined by a call to
**  the system getenv() routine), then that pointer is returned.  Otherwise,
**  if the variable is defined in the [CFG] section of "voodoo2.ini", then
**  a pointer to the value defined in "voodoo2.ini" is returned.  Otherwise,
**  NULL is returned
**
*/
FX_ENTRY char* FX_CALL getenv(const char *string)
{
  const char* retVal = NULL;
  
  /* Does the requested environment variable exist in "voodoo2.ini"? */
  /* Dump CFG Data... */
  if (!checkedFileP) {
  	static FxBool inProc = FXFALSE;
  	
  	if (!inProc) {
  		inProc = FXTRUE;
     	sst1InitVoodooFile();
      inProc = FXFALSE;
   }
  }
    
  {
    sst1InitEnvVarStruct *envVarsPtr = envVarsBase;
    char tempSearchString[kMaxEnvVarLen];

    /* Put the search into canonical form */
    strcpy(tempSearchString, string);
    sst1InitToLower(tempSearchString);

    while(envVarsPtr) {
      if(!strcmp(tempSearchString, envVarsPtr->envVariable)) {
        retVal = envVarsPtr->envValue;
        break;
      }
      envVarsPtr = (sst1InitEnvVarStruct *) envVarsPtr->nextVar;
    }
  }

  return (char*)retVal;
}

#pragma optimize ("",on)

#endif
