
ATB(tm) Release 0.2a1 README.TXT
==================================

This is a preliminary release of the 3DFX Interactive Arcade Toolbox.
ATB layers on top of Glide and provides an optimized transformation, 
lighting, clipping pipeline. This release includes the full source code for
ATB to enable you to make the most of Voodoo Graphics. ATB is continuing to
evolve based on developer experience and feedback, your comments and 
suggestions are most welcome. Please note that this release is being made on
an as is basis and is subject to change based on developer input.

We hope that this release will help you use Voodoo Graphics to
push the frontiers of game graphics!!!

_____________________________________________________________________________

Contents
========


Prebuilt versions of ATB are provided for several environments:
MS-DOS using DOS4GW protected mode runtime, and Windows 95 and Windows NT.

Libraries compatible with Watcom C for MS-DOS and Win32, and
Microsoft Visual C for Win32, are provided, as well as header file,
source file, demo programs and documentation.

The code provided with this release is arranged into a number of separate
libraries which provide a modular set of building blocks:

    ati:       the ATB input library supporting keyboard, joystick and GCI
  
    atm:       the ATB math and linear algebra library
 
    atr:       the ATB rendering library

    ats:       the ATB scene manager

    atu:       the ATB utility library used by the higher level libraries

Preliminary documentation for these libraries is included with this release.
Some other lower level libraries are also included as they are needed to support
ATB and are not intended to be called directly. These libraries will be integrated 
into ATB in a future release. The 3d studio library is used by the ATB scene manager 
for loading 3d studio files the joystick and GCI libraries provide low level device 
access.

    3dstudio:  a library for loading 3d studio files

    gci:       a low  level input library for the GCI hardware. In the absence of this
               hardware GCI can provide input via the windows input drivers. The GCI
               functionality 

    joy:       joystick input library.

To install the software simply copy it from the CD into a convenient directory.

_____________________________________________________________________________

Running Demos
=============

Three demos are provided with this initial release fight, atsflip and wiz. 
Batch files are provided to run these demos.

_____________________________________________________________________________

Building Source
===============

For this initial release sources and a skeleton makefile are provided which
list the c files, header files and libraries used in building the targets.

_____________________________________________________________________________

Support
=======

If you have any questions on this release, there are currently two ways
to get assistance:
	e-mail          devsupport@3dfx.com
	phone           1-800-400-3235

_____________________________________________________________________________

The ATB Development Team
