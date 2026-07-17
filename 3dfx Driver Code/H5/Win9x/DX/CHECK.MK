##########################################################################
#
#       Microsoft Confidential
#       Copyright (C) Microsoft Corporation 1991
#       All Rights Reserved.
#
##########################################################################

#
# Assume No Error
#
!UNDEF CHECK_ERROR

!ifndef MASM
!MESSAGE >
!MESSAGE > The Environment variable MASM must be defined in order to
!MESSAGE > to use this makefile. I.E. SET MASM=%DEVTOOLS%\masm611
!MESSAGE >
CHECK_ERROR=1
!endif

!ifndef COMP16
!MESSAGE >
!MESSAGE > The Environment variable COMP16 must be defined in order to
!MESSAGE > to use this makefile. I.E. SET COMP16=%DEVTOOLS%\msvc
!MESSAGE >
CHECK_ERROR=1
!endif

!ifndef COMP32
!MESSAGE >
!MESSAGE > The Environment variable COMP32 must be defined in order to
!MESSAGE > to use this makefile. I.E. SET COMP32=%DEVTOOLS%\msvc20
!MESSAGE >
CHECK_ERROR=1
!endif

!ifndef DDK9X
!MESSAGE >
!MESSAGE > The Environment variable DDK9X must be defined in order to
!MESSAGE > to use this makefile. I.E. SET DDK9X=%DEVTOOLS%\ddk
!MESSAGE >
CHECK_ERROR=1
!endif

!ifndef SDK32
!MESSAGE >
!MESSAGE > The Environment variable SDK32 must be defined in order to
!MESSAGE > to use this makefile. I.E. SET SDK32=%DEVTOOLS%\mstools
!MESSAGE >
CHECK_ERROR=1
!endif


#
# Was there an error?!?
#
!ifdef CHECK_ERROR
!error
!endif

