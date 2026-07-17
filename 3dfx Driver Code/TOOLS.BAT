@echo off

REM -----------------------------------------------------------------------------
REM Copyright (c) 1999, 3Dfx Interactive, Inc.
REM All Rights Reserved.
REM 
REM This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
REM the contents of this file may not be disclosed to third parties, copied or
REM duplicated in any form, in whole or in part, without the prior written
REM permission of 3Dfx Interactive, Inc.
REM 
REM RESTRICTED RIGHTS LEGEND:
REM Use, duplication or disclosure by the Government is subject to restrictions
REM as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
REM and Computer Software clause at DFARS 252.227-7013, and/or in similar or
REM successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
REM rights reserved under the Copyright Laws of the United States.
REM 
REM -----------------------------------------------------------------------------

REM -----------------------------------------------------------------------------
REM Clean up environment
REM -----------------------------------------------------------------------------

SET PATH=%systemroot%;%systemroot%\system32
SET LIB=
SET INCLUDE=

REM -----------------------------------------------------------------------------
REM Setup tools directory
REM -----------------------------------------------------------------------------

set DEVTOOLS=c:\3dfxtools


set Q3DSDK=C:\Q3DTools\SwStore\NetUse\Quantum3D_DriverExtensionsSdk_Private\1.5