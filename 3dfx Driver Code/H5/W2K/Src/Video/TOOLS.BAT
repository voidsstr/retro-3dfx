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
REM Setup tools directory
REM -----------------------------------------------------------------------------

set DEVTOOLS=d:\devtools

REM -----------------------------------------------------------------------------
REM Setup debugger directory
REM -----------------------------------------------------------------------------

set SIW2K=d:\siw2k

REM -----------------------------------------------------------------------------
REM Setup strip symbols
REM -----------------------------------------------------------------------------

REM NOTE: Setting STRIP_SYMBOLS=1 will prevent SoftICE from generating a .NMS file!

set STRIP_SYMBOLS=0

