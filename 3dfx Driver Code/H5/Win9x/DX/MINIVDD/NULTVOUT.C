

/*----------------------------------------------------------------------
Function name:  NullTV_GetStatus

Description:    Return the status of the BT868.

Information:

Return:         INT     The status word or,
                        -1 for failure.
----------------------------------------------------------------------*/
int NullTV_GetStatus (PDEVTABLE pDev, int estatus)
{
   return -1;
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetPosition

Description:    Get current horizontal/vertical position.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetPosition (PTVCURPOS pTvCurPos, void *tvOutData)
{
	pTvCurPos->dwCurLeft = 0;
	pTvCurPos->dwCurTop = 0;
	pTvCurPos->dwCurRight = 0;
	pTvCurPos->dwCurBottom = 0;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  NullTV_SetStandard

Description:    Set TV type.

Information:

Return:         INT     retult of the NullTV_Enable call.
----------------------------------------------------------------------*/
int NullTV_SetStandard (PTVSETSTANDARD pTvSetStandard, PDEVTABLE pDev)
{
   return FXFALSE;
}


/*----------------------------------------------------------------------
Function name:  NullTV_SetPicControl

Description:    Set the picture contorl (Brightness, Stautation, or
                Flicker).
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_SetPicControl (PTVSETCAP pTvSetPicControl, PDEVTABLE pDev)
{
   return TV_CONTROL_UNSUPPORTED;
}


/*----------------------------------------------------------------------
Function name:  NullTV_SetPosition

Description:    Set position.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_SetPosition(PTVSETPOS pTvSetPosition, PDEVTABLE pDev)
{
	return (-1); // original function always returned 0, so return -1 here.
}


/*----------------------------------------------------------------------
Function name:  NullTV_SetSize

Description:    Set the picture size.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_SetSize(PTVSETSIZE pTvSetSize, PDEVTABLE pDev)
{
	return (-1); // original function always returned 0, so return -1 here.
}


/*----------------------------------------------------------------------
Function name:  NullTV_SetSpecial

Description:    Set the TV as a special device (ie. LCD panel).

Information:

Return:         INT     0 is always returned
----------------------------------------------------------------------*/
int NullTV_SetSpecial(PTVSETSPECIAL pTvSetSpecial, DWORD RegBase)
{
   // have no idea what this does, so just return 0
   return(0);
}


/*----------------------------------------------------------------------
Function name:  NullTV_Disable

Description:    Disable the TV device.

Information:

Return:         INT     0 is always returned
----------------------------------------------------------------------*/
int NullTV_Disable(PDEVTABLE pDev)
{
   return(0); // let them disable it, coz it ain't really there anyway
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetPicControl

Description:    Get the picture contorl (Brightness, Stautation, or
                Flicker).
Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetPicControl (PTVCURCAP tvPicValue, void *tvOutData)
{
	return (-1); // gotta return != 0 because the data would be bogus
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetFilterControl

Description:    Get the filter contorl.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetFilterControl (PTVCURCAP tvPicValue, void *tvOutData)
{
	return (-1); // gotta return != 0 because the data would be bogus
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetSizeControl

Description:    Get the horizontal/vertical contorl.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetSizeControl (PTVCURSIZE tvPicValue, void *tvData)
{
	return (-1); // gotta return != 0 because the data would be bogus
}


/*----------------------------------------------------------------------
Function name:  NullTV_Enable

Description:    Enable the TV device.

Information:

Return:         INT     FXTRUE if success,
                        FXFALSE if failure.
----------------------------------------------------------------------*/
int NullTV_Enable (PDEVTABLE pDev, int vgaMode)
{
	return (FXFALSE); // can't enable what doesn't exist.
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetStandard

Description:    Get the TV type.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
#ifdef STU_ORIGINAL
int NullTV_GetStandard (PDEVTABLE pDev, LPTVGETSTANDARD lpOutput)
#else
int NullTV_GetStandard (PDEVTABLE pDev, LPTVSETSTANDARD lpOutput)
#endif
{
   return (-1); // gotta return != 0 because the data would be bogus
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetSizeCap

Description:    Get the Min/Max size capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetSizeCap (PDEVTABLE pDev, LPTVSIZECAP lpOutput)
{
   // go ahead and init the structure, because I'm afraid bad things
   // would happen if I didn't.  but report error nonetheless.
	lpOutput->dwMaxHorInput = 800;
	lpOutput->dwMaxVerInput = 600;
	lpOutput->dwMaxHorOutput = 800;
	lpOutput->dwMaxVerOutput = 600;
	lpOutput->dwMinHorInput = 640;
	lpOutput->dwMinVerInput = 480;
	lpOutput->dwMinHorOutput = 640;
	lpOutput->dwMinVerOutput = 480;
	lpOutput->dwHorStepSize = 1;
	lpOutput->dwVerStepSize = 1;
	return (-1); // non-zero means error
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetPosCap

Description:    Get the position capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetPosCap (PDEVTABLE pDev, LPTVPOSCAP lpOutput)
{
	lpOutput->dwMaxLeft = 0;
	lpOutput->dwMaxRight = 0;
	lpOutput->dwHorGranularity = 0;
	lpOutput->dwMaxTop = 0;
	lpOutput->dwMaxBottom = 0;
	lpOutput->dwVGAGranularity = 8;
	return (0);
}


/*----------------------------------------------------------------------
Function name:  NullTV_GetFilterCap

Description:    Get the filter capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetFilterCap (PDEVTABLE pDev, LPTVCAPDATA lpOutput)
{
   lpOutput->dwNumSteps = 0;
	return (0);
}



/*----------------------------------------------------------------------
Function name:  NullTV_GetPicCap

Description:    Get the picture capabilities.

Information:

Return:         INT     0 is always returned.
----------------------------------------------------------------------*/
int NullTV_GetPicCap (PDEVTABLE pDev, LPTVCAPDATA lpOutput)
{
   lpOutput->dwNumSteps = 0;
	return (0);
}



/*----------------------------------------------------------------------
Function name:  NullTV_CopyProtect

Description:    Enable or Disable Macrovision encoding on bt869
Information:    setting = 1 means on

Return:         INT     1 or 0
----------------------------------------------------------------------*/
int NullTV_CopyProtect (PDEVTABLE pDev, int setting)
{
   return 0;
}



/*----------------------------------------------------------------------
Function name:  NullTV_FixupVGA

Description:    Set VGA CRTC registers for full screen DOS mode 3

Information:    Returns 0

Return:         int
----------------------------------------------------------------------*/

int NullTV_FixupVGA (PDEVTABLE pDev, int unused)
{
	return (0);
}