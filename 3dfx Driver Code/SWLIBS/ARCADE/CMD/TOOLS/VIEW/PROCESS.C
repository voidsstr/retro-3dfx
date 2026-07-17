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
** $Revision: 4$ 
** $Date: 10/11/00 7:30:31 PM$ 
**
*/

#include <windows.h>
#include <commctrl.h>
#include <glide.h>
#include <atutil.h>
#include <atrender.h>
#include <atscene.h>
#include <atinput.h>
#include <ataudio.h>
#include <atdemop.h>
#include <conio.h>
#include "resource.h"
#include "afxres.h"
#include "view.h"

void 
spawnAndWait(char *appName) {
    BOOL bWorked;
    STARTUPINFO suInfo;
    PROCESS_INFORMATION procInfo;

    memset(&suInfo, 0, sizeof(suInfo));
    suInfo.cb = sizeof(suInfo);

    bWorked = CreateProcess(NULL,
        appName,
        NULL,
        NULL,
        FALSE,
        NORMAL_PRIORITY_CLASS,
        NULL,
        NULL,
        &suInfo,
        &procInfo);

    if (bWorked == FALSE) {
        DWORD dwError = GetLastError();
        char buff[128];
        PVOID pstrError;
        DWORD dwResult;

        dwResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwError,
            LANG_NEUTRAL,
            (LPTSTR) &pstrError, 0, NULL);

        if (dwResult == 0)
            pstrError = "FormatMessage() failed!";

        sprintf(buff, "Failed!  %s\n(Code is %d)", pstrError, dwError);

        atuError(FXFALSE, buff);
    } else {
        HANDLE hArray[2];
        DWORD dwReturn ;
        hArray[0] = procInfo.hProcess;
        /* hArray[1] = pInfo->m_psemClosing->m_hObject; */

        dwReturn = WaitForMultipleObjects(1, hArray, FALSE, INFINITE);
    }
}

