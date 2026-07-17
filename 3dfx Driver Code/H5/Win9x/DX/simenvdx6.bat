@echo off
REM -----------------------------------------------------------------------------
REM Copyright (c) 1997, 3Dfx Interactive, Inc.
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
REM  SETENV.BAT SET UP 3DFX BUILD ENVIRONMENT FOR COMPILING BANSHEE DRIVERS
REM  FOR WINDOWS 95
REM      updated 4/15/98 gsanders
REM -----------------------------------------------------------------------------

REM -----------------------------------------------------------------------------
REM set the following variables for your particular environment
REM -----------------------------------------------------------------------------

REM Complier tools required by this build are -
REM
REM COMP32 - Msvc6_0
REM COMP16 - Msvc1_52
REM DDKDX  - Dx6ddk
REM MASM   - Masm614
REM DDK9X  - Win95ddk
REM SDK32  - Win32sdk

REM Required when debug symbols are needed.
REM NOTE: Setting this when building the retail
REM driver will result in debug information
REM within the retail build.
SET SIW95=%DEVTOOLS%\SIW95

REM Environment variables to all the required tools
REM The environment variable DEVTOOLS should point to
REM the tools directory.
SET COMP16=%DEVTOOLS%\msvc1_52
SET COMP32=%DEVTOOLS%\msvc6_0\vc98
SET MASM=%DEVTOOLS%\masm614
SET DDK9X=%DEVTOOLS%\win95ddk
SET SDK32=%DEVTOOLS%\win32sdk
SET DDKDX=%DEVTOOLS%\dx6ddk

REM -----------------------------------------------------------------------------
REM # Setup PATH to point to a copy of nmake
REM -----------------------------------------------------------------------------
PATH=%COMP32%\bin;%PATH%

REM ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
REM Build Flags
REM ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

REM SW_TNL=0 use the software transform and light code
set SW_TNL=0

REM STBKNI=1 use the STB Katmai KNI optimizations
set STBKNI=1

REM DTD=1 use Deferred Texture Downloading optimization
set DTD=0

REM VC6=1 use the VC6 complilier optimizations for DD & D3D
set VC6=1

REM TU=1 use one texture unit (Banshee)
REM TU=2 use two texture units (Voodoo3)
set TU=2

REM HP=H3 use for Banshee specific hw and driver names
REM HP=H4 use for Voodoo3 specific hw and driver names
set HP=H5

REM The following enables STB Display driver optimizations
set BRDCFLAGS=-DINCSTBPERF
set BRDAFLAGS=-DINCSTBPERF

REM K6=1 use the AMD K6 optimizations
set K6=1

REM ACF=1 use AGP CMD FIFO, 0 for no AGP CMD FIFO
set ACF=0

REM -----------------------------------------------------------------------------
REM  You should NOT have to alter the following variables.
REM -----------------------------------------------------------------------------

REM Turn off Verbose Mode
set VERBOSE=

REM CF=1 is use CMDFIFO, else use Direct Write
set CF=1

REM DF=1 is use DEBUG CMDFIFO macros
set DF=0

REM CT=1 use CRASH TEST -- puts in lots of FXWAITFORIDLES
set CT=0

REM SSB=1 use SaveScreenBitmaps
set SSB=1

REM IRQ=1 Interrupts On or OFF
set IRQ=0

REM ND=1 for Null Driver
set ND=0

REM PN=1 for Performance NOP
set PN=0

REM DX= Level
set DX=6

REM Set SE=1 for Schematic Error
set SE=0

REM Set FP=1 for Flavor Profile
set FP=0

REM Set SD=1 for SDRAM Support
set SD=0

REM Set MMI=1 for Memory Manager Integrity Check
set MMI=0

REM Set IFB=1 for Infinite Frame Buffer
set IFB=0

REM Set DDR=1 for Direct Draw Refresh rates
set DDR=0

REM Set HR=B0 for Hardware Revision -- Hold Over from Banshee B0
set HR=B0

REM MT=1 for Mode Type -- This is a OEM version of Mode Behavior
set MT=0

REM V3TV -- V3 Television
set V3TV=0

REM WCS=1 Windows C Simulator
set WCS=1

REM SA=1 for SLI/AA Support
set SA=1

REM NEWASMTRI=1 New 3DNow and KNI Assembler Triangle Rendering routines
set NEWASMTRI=1
