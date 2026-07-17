/* -*-c++-*- */
/* */
/* Copyright (c) 1997-1998, 3Dfx Interactive, Inc.
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

Module Name:

    h3modetab.c

Abstract:

    This module contains all the global data used by the H3 driver.

Environment:

    Kernel mode

Revision History:


--*/

#ifdef INCSTBCUST
#include "..\..\..\..\build\stbcust.inc"
#endif

#include "dderror.h"
#include "devioctl.h"
#include "miniport.h"

#include "ntddvdeo.h"
#include "video.h"
#include "h3.h"
#include "localpci.h"
#include "cmdcnst.h"

#define ENABLE_24BPP_MODES    0

/*****************************************************************************
 *
 * START OF PAGED DATA
 *
 * All of the data listed below is pageable.  Therefore the system can
 * swap the data out to disk when it needs to free some physical memory.
 *
 * Any data accessed while paging is unavailable should be placed above.
 *
 ****************************************************************************/

#if defined(ALLOC_PRAGMA)
#pragma data_seg("PAGE")
#endif

#ifdef DMT_ENABLED 


VIDEO_MODE_INFORMATION BasicModeInfo_8BPP = 
{
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          640,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
};

VIDEO_MODE_INFORMATION BasicModeInfo_16BPP = 
{
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
};

VIDEO_MODE_INFORMATION BasicModeInfo_32BPP = 
{                   	
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          2560,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
};

H3_VIDEO_MODES DefaultMode[] = 

{                   // 640x480x8bpp
      0x0101,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0101,           // 'Noncontiguous' Int 10 mode number
      640,              // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          640,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        },
};

ULONG NumH3VideoModes;

#else // #ifdef DMT_ENABLED 

///////////////////////////////////////////////////////////////////////////
// Video mode table - Lists the information about each individual mode.
//
// Note that any new modes should be added here and to the appropriate
// H3_VIDEO_FREQUENCIES tables.
//

H3_VIDEO_MODES H3Modes[] = {
    {                   // 320x200x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      320,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          320,                            // X Resolution, in pixels
          200,                            // Y Resolution, in pixels
          320,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 320x240x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      320,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          320,                            // X Resolution, in pixels
          240,                            // Y Resolution, in pixels
          320,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 400x300x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      400,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          400,                            // X Resolution, in pixels
          300,                            // Y Resolution, in pixels
          400,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 512x384x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      512,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          512,                            // X Resolution, in pixels
          384,                            // Y Resolution, in pixels
          512,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

	{                   // 640x400x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      640,              // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          640,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0						          // DriverSpecificAttributeFlags
        },
    },

#if VIDEO_720_MODES_SUPPORTED
	{                   // 720x400x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      720,              // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          720,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },
#endif

	{                   // 640x480x8bpp
      0x0101,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0101,           // 'Noncontiguous' Int 10 mode number
      640,              // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          640,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        },
    },

	{                   // 720x480x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      720,              // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          720,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

    {                   // 720x576x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      720,              // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          576,                            // Y Resolution, in pixels
          720,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
              VIDEO_MODE_MANAGED_PALETTE, // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

	{                   // 800x600x8bpp
      0x0103,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0103,           // 'Noncontiguous' Int 10 mode number
      800,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          800,                            // X Resolution, in pixels
          600,                            // Y Resolution, in pixels
          800,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

	{                   // 960x720x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      960,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          960,                            // X Resolution, in pixels
          720,                            // Y Resolution, in pixels
          960,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1024x768x8bpp
      0x0105,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0105,           // 'Noncontiguous' Int 10 mode number
      1024,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1024,                           // X Resolution, in pixels
          768,                            // Y Resolution, in pixels
          1024,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1152x864x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1152,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1152,                           // X Resolution, in pixels
          864,                            // Y Resolution, in pixels
          1152,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1280x960x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1280,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          960,                            // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

    {                   // 1280x1024x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1280,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1024x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1600,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          1600,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1200x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1600,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          1600,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1792x1344x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1792,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1792,                           // X Resolution, in pixels
          1344,                           // Y Resolution, in pixels
          1792,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if 0
    {                   // 1800x1440x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1800,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1800,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          1800,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 1856x1392x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1856,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1856,                           // X Resolution, in pixels
          1392,                           // Y Resolution, in pixels
          1856,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if 0
    {                   // 1900x1200x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1900,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1900,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          1900,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 1920x1080x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1920,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1080,                           // Y Resolution, in pixels
          1920,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1200x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1920,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          1920,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1440x8bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1920,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          1920,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 2046x1536x8bpp
	                    // Note: External to the miniport Windows believes this mode is 2046 pixels, internally it actually is
						//       2048 wide with 2 pixels left black.  This is a workaround for a Voodoo3 hardware limitation.
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2048,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          2048,                           // X Resolution, in pixels
          1536,                           // Y Resolution, in pixels
          2048,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          8,                              // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00000000,                     // Mask for Red Pixels in non-palette modes
          0x00000000,                     // Mask for Green Pixels in non-palette modes
          0x00000000,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS | VIDEO_MODE_PALETTE_DRIVEN |
          VIDEO_MODE_MANAGED_PALETTE,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)CAPS_SW_POINTER          // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 320x200x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      640,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          320,                            // X Resolution, in pixels
          200,                            // Y Resolution, in pixels
          640,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        }
    },

    {                   // 320x240x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      640,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          320,                            // X Resolution, in pixels
          240,                            // Y Resolution, in pixels
          640,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        }
    },

    {                   // 400x300x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      800,              // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          400,                            // X Resolution, in pixels
          300,                            // Y Resolution, in pixels
          800,                            // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        }
    },

    {                   // 512x384x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1024,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
          512,                            // X Resolution, in pixels
          384,                            // Y Resolution, in pixels
          1024,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        }
    },

	// It's a game mode...but wait -- we can support it without the overlay.
    {                   // 640x400x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1280,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

#if VIDEO_720_MODES_SUPPORTED
    {                   // 720x400x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1440,             // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          1440,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },
#endif

    {                   // 640x480x16bpp
      0x0111,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0111,           // 'Noncontiguous' Int 10 mode number
      1280,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

	{                   // 720x480x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1440,             // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          1440,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

    {                   // 720x576x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1440,             // 'Contiguous' screen stride (it's '1024' here merely
                        // because we don't do 640x480 in contiguous mode)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          576,                            // Y Resolution, in pixels
          1440,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

	{                   // 800x600x16bpp
      0x0114,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0114,           // 'Noncontiguous' Int 10 mode number
      1600,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          800,                            // X Resolution, in pixels
          600,                            // Y Resolution, in pixels
          1600,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },


	{                   // 960x720x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1920,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          960,                            // X Resolution, in pixels
          720,                            // Y Resolution, in pixels
          1920,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
    {                   // 1024x768x16bpp
      0x0117,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0117,           // 'Noncontiguous' Int 10 mode number
      2048,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1024,                           // X Resolution, in pixels
          768,                            // Y Resolution, in pixels
          2048,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1152x864x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2304,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1152,                           // X Resolution, in pixels
          864,                            // Y Resolution, in pixels
          2304,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1280x960x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2560,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          960,                            // Y Resolution, in pixels
          2560,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

    {                   // 1280x1024x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2560,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          2560,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1024x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3200,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          3200,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1200x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3200,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          3200,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1792x1344x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3584,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1792,                           // X Resolution, in pixels
          1344,                           // Y Resolution, in pixels
          3584,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if 0
    {                   // 1800x1440x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3600,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1800,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          3600,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 1856x1392x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3712,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1856,                           // X Resolution, in pixels
          1392,                           // Y Resolution, in pixels
          3712,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if 0
    {                   // 1900x1200x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3800,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1900,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          3900,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 1920x1080x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3840,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1080,                           // Y Resolution, in pixels
          3840,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1200x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3840,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          3840,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1440x16bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3840,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          3840,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 2046x1536x16bpp
	                    // Note: External to the miniport Windows believes this mode is 2046 pixels, internally it actually is
						//       2048 wide with 2 pixels left black.  This is a workaround for a Voodoo3 hardware limitation.
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      4096,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          2048,                           // X Resolution, in pixels
          1536,                           // Y Resolution, in pixels
          4096,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          16,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x0000f800,                     // Mask for Red Pixels in non-palette modes
          0x000007e0,                     // Mask for Green Pixels in non-palette modes
          0x0000001f,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)CAPS_SW_POINTER          // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if ENABLE_24BPP_MODES
    {                   // 640x400x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1920,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          1920,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

    {                   // 720x400x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2160,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          2160,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

    {                   // 640x480x24bpp
      0x0112,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0112,           // 'Noncontiguous' Int 10 mode number
      1920,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          1920,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

	{                   // 720x480x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2160,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          2160,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

    {                   // 720x576x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2160,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          576,                            // Y Resolution, in pixels
          2160,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },

	{                   // 800x600x24bpp
      0x0115,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0115,           // 'Noncontiguous' Int 10 mode number
      2400,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          800,                            // X Resolution, in pixels
          600,                            // Y Resolution, in pixels
          2400,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1024x768x24bpp
      0x0118,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3072,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1024,                           // X Resolution, in pixels
          768,                            // Y Resolution, in pixels
          3072,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1152x864x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3456,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1152,                           // X Resolution, in pixels
          864,                            // Y Resolution, in pixels
          3456,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1280x960x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3840,             // 'Contiguous' screen stride (1280 * 3 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          960,                            // Y Resolution, in pixels
          3840,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

    {                   // 1280x1024x24bpp
      0x011B,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x011B,           // 'Noncontiguous' Int 10 mode number
      3840,             // 'Contiguous' screen stride (1280 * 3 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          3840,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1024x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      4800,             // 'Contiguous' screen stride (1600 * 3 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          4800,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1200x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      4800,             // 'Contiguous' screen stride (1600 * 3 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          4800,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1792x1344x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5376,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1792,                           // X Resolution, in pixels
          1344,                           // Y Resolution, in pixels
          5376,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1800x1440x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5400,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1800,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          5400,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1856x1392x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5568,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1856,                           // X Resolution, in pixels
          1392,                           // Y Resolution, in pixels
          5568,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1900x1200x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5700,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1900,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          5700,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1080x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5760,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1080,                           // Y Resolution, in pixels
          5760,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1200x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5760,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          5760,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1440x24bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5760,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          5760,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 2046x1536x24bpp
	                    // Note: External to the miniport Windows believes this mode is 2046 pixels, internally it actually is
						//       2048 wide with 2 pixels left black.  This is a workaround for a Voodoo3 hardware limitation.
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      6144,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          2048,                           // X Resolution, in pixels
          1536,                           // Y Resolution, in pixels
          6144,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          24,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)CAPS_SW_POINTER          // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 320x200x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1280,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          320,                            // X Resolution, in pixels
          200,                            // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 320x240x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1280,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          320,                            // X Resolution, in pixels
          240,                            // Y Resolution, in pixels
          1280,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 400x300x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      1600,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          400,                            // X Resolution, in pixels
          300,                            // Y Resolution, in pixels
          1600,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 512x384x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2048,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          512,                            // X Resolution, in pixels
          384,                            // Y Resolution, in pixels
          2048,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)(CAPS_SCAN_LINE_DOUBLED | CAPS_SW_POINTER) // DriverSpecificAttributeFlags
        },
    },

    {                   // 640x400x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2560,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          2560,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

#if VIDEO_720_MODES_SUPPORTED
    {                   // 720x400x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2880,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          400,                            // Y Resolution, in pixels
          2880,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        },
    },
#endif

	{                   // 640x480x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2560,             // 'Contiguous' screen stride (640 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          640,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          2560,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

	{                   // 720x480x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2880,             // 'Contiguous' screen stride (640 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          480,                            // Y Resolution, in pixels
          2880,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 720x576x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      2880,             // 'Contiguous' screen stride (640 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          720,                            // X Resolution, in pixels
          576,                            // Y Resolution, in pixels
          2880,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 800x600x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3200,             // 'Contiguous' screen stride (800 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          800,                            // X Resolution, in pixels
          600,                            // Y Resolution, in pixels
          3200,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 960x720x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      3840,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          960,                            // X Resolution, in pixels
          720,                            // Y Resolution, in pixels
          3840,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 5:6:5
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1024x768x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      4096,             // 'Contiguous' screen stride (1024 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1024,                           // X Resolution, in pixels
          768,                            // Y Resolution, in pixels
          4096,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1152x864x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      4608,             // 'Contiguous' screen stride (1152 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1152,                           // X Resolution, in pixels
          864,                            // Y Resolution, in pixels
          4608,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1280x960x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5120,             // 'Contiguous' screen stride (1280 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          960,                            // Y Resolution, in pixels
          5120,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags
        }
    },

    {                   // 1280x1024x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      5120,             // 'Contiguous' screen stride (1280 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1280,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          5120,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1024x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      6400,             // 'Contiguous' screen stride (1600 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1024,                           // Y Resolution, in pixels
          6400,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1600x1200x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      6400,             // 'Contiguous' screen stride (1600 * 4 bytes)
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1600,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          6400,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // RGB 8:8:8
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1792x1344x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7168,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1792,                           // X Resolution, in pixels
          1344,                           // Y Resolution, in pixels
          7168,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if 0
    {                   // 1800x1440x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7200,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1800,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          7200,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 1856x1392x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7424,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1856,                           // X Resolution, in pixels
          1392,                           // Y Resolution, in pixels
          7424,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

#if 0
    {                   // 1900x1200x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7600,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1900,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          7600,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },
#endif

    {                   // 1920x1080x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7680,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1080,                           // Y Resolution, in pixels
          7680,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1200x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7680,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1200,                           // Y Resolution, in pixels
          7680,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 1920x1440x32bpp
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      7680,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          1920,                           // X Resolution, in pixels
          1440,                           // Y Resolution, in pixels
          7680,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          0                               // DriverSpecificAttributeFlags (filled in later)
        }
    },

    {                   // 2046x1536x32bpp
	                    // Note: External to the miniport Windows believes this mode is 2046 pixels, internally it actually is
						//       2048 wide with 2 pixels left black.  This is a workaround for a Voodoo3 hardware limitation.
      0x0000,           // 'Contiguous' Int 10 mode number (for high-colour)
      0x0000,           // 'Noncontiguous' Int 10 mode number
      8192,             // 'Contiguous' screen stride
        {
          sizeof(VIDEO_MODE_INFORMATION), // Size of the mode informtion structure
          0,                              // Mode index used in setting the mode
                                          // (filled in later)
          2048,                           // X Resolution, in pixels
          1536,                           // Y Resolution, in pixels
          8192,                           // 'Noncontiguous' screen stride,
                                          // in bytes (distance between the
                                          // start point of two consecutive
                                          // scan lines, in bytes)
          1,                              // Number of video memory planes
          32,                             // Number of bits per plane
          1,                              // Screen Frequency, in Hertz ('1'
                                          // means use hardware default)
          320,                            // Horizontal size of screen in millimeters
          240,                            // Vertical size of screen in millimeters
          8,                              // Number Red pixels in DAC
          8,                              // Number Green pixels in DAC
          8,                              // Number Blue pixels in DAC
                                          // palettized mode
          0x00ff0000,                     // Mask for Red Pixels in non-palette modes
          0x0000ff00,                     // Mask for Green Pixels in non-palette modes
          0x000000ff,                     // Mask for Blue Pixels in non-palette modes
          VIDEO_MODE_COLOR | VIDEO_MODE_GRAPHICS,     // Mode description flags.
          0,                              // Video Memory Bitmap Width (filled in later)
          0,                              // Video Memory Bitmap Height (filled in later)
          (ULONG)CAPS_SW_POINTER          // DriverSpecificAttributeFlags (filled in later)
        }
    }
};

ULONG NumH3VideoModes = sizeof(H3Modes) / sizeof(H3_VIDEO_MODES);

#endif // #ifdef DMT_ENABLED 


#if defined(ALLOC_PRAGMA)
#pragma data_seg()
#endif
