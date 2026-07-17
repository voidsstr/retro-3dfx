/*
 * Name:        Debug.c
 *
 * Description: This module contains the Napalm debug functions. This module
 *              is called to handle user debug requests.
 *
 * Date:        August 1, 1998
 *
 * Version:     1.00
 *
 * Author:      Randy Spurlock
 *
 **********************
 *
 * Changes:     Date            Version Description
 *
 *              08/01/1998      1.00    Original version
 *              09/30/1998      1.01    Converted to C from debug.asm
 *
 */
#define WANTVXDWRAPS

#define WIN40COMPAT
#include "h3vdd.h"
#include "h3.h"

#define VDDONLY
#include "h3g.h"
#include "h3cinitdd.h"
#include "devtable.h"
#include "h3irq.h"
#ifndef WINNT
#include "shared.h"		// NEW CPUID CODE
#endif
#undef  VDDONLY
#include "ascii.h"
#include "sliaa.h"
#include "h3cinit.h"
#include "p6stuff.h"
#include "gpio.h"

/*
 * Local Defines
 */
#define         HEX                     16
#define         DECIMAL                 10
#define         OCTAL                   8
#define         BINARY                  2
#define         PAGE_SIZE               0x1000
#define         TAB_END                 ASCII_EOF

#define         Print(f, a)             _Debug_Vprintf_Service(f, a)
#define         PF                      _Debug_Printf_Service





/*
 * Local declarations
 */
#pragma VxD_DEBUG_ONLY_CODE_SEG
void Napalm_Debug();
BOOL Debug_Input(DWORD, char *, DWORD *);
BOOL Check_Abort();

typedef BOOL (* COMMANDFN)(BYTE);
BOOL PCI_Command(BYTE);
BOOL WR_Command(BYTE);
BOOL SLI_Command(BYTE);
BOOL Write_Command(BYTE);
BOOL Exit_Command(BYTE);
BOOL Help_Command(BYTE);
BOOL Quit_Command(BYTE);
BOOL Reg_Command(BYTE);
BOOL Mode_Command(BYTE);
BOOL Mtrr_Command(BYTE);
BOOL AGP_Command(BYTE);
BOOL Clock_Command(BYTE);

/*
 * Local data
 */
#pragma VxD_DEBUG_ONLY_CODE_SEG
#pragma VxD_DEBUG_ONLY_DATA_SEG

// Data for this module
#define DEBUG_LENGTH (0x80)
BYTE            Debug_Buffer[DEBUG_LENGTH];
BYTE            Parm_Buffer[DEBUG_LENGTH];

// Messages
char            Debug_Prompt[] =        "\nNapalm>";
char *          Help_Fmt[] =            {"\nNapalm Debug Command Summary\n\n",
                                        "P[CI] <Bus#> <Dev#>              - Display PCI Info for Device\n",
                                        "W[r] Bus# Dev# Func# Reg# Value  - Toggles int1 in the code\n",
                                        "S[LI] <Bus#> <Dev#>              - Display PCI SLI Info for Device\n",
                                        "E[xit]                           - Exits back to debugger\n",
                                        "H[elp] [Command]                 - Prints Help Information\n",
                                        "Q[uit]                           - Quits back to debugger\n",
                                        "R[eg]                            - Dumps VGA registers\n",
                                        "M[ode]                           - Resets the last mode\n",
                                        "A[gp]                            - Dump the AGP registers\n",
                                        "C[lock] s v r                    - Program the ICS 307 Clock\n",
                                         };
                                        

BYTE*           Unknown_Arg[] =         {
                                                Parm_Buffer
                                        };
char            Unknown_Fmt[] =         "\nUnknown command \"%s\"!";

char            PCI_Hlp[] =             "\nPci <Bus#> <Dev#>           - Dump PCI Info";
char            WR_Hlp[] =              "\nWr Bus# Dev# Func# Reg# Value             - Wr PCI Info";
char            SLI_Hlp[] =             "\nSLI <Bus#> <Dev#> - Dump the hidden SLI Config Register for the Device";
char            Exit_Hlp[] =            "\nExit           - Exits control back to the debugger";
char            Help_Hlp[] =            "\nHelp           - Prints the general help message"
                                        "\nHelp Command   - Prints command specific help";
char            Quit_Hlp[] =            "\nQuit           - Quits back to the debugger";
char            Reg_Hlp[] =             "\nReg            - Dump VGA Registers";
char            Mode_Hlp[] =            "\nMode           - Resets the last mode";
char            Mtrr_Hlp[] =            "\nMtrr           - Print MTRRs";
char            Agp_Hlp[] =             "\nAgp            - Print AGP Registers";
char            Clock_Hlp[] =           "\nClock s v r    - Reprogram the ICS 307";

BYTE            Delimiter_Table[] =     {
                                                ASCII_SPACE, ASCII_HT, ASCII_COLON, ASCII_SEMI_COLON,
                                                ASCII_COMMA, ASCII_NULL, ASCII_EOF
                                        };

BYTE            Command_Table[] =       {
                                                8,
                                                1, 'P', 'C', 'I', ' ', ' ', ' ', ' ',
                                                1, 'W', 'R', ' ', ' ', ' ', ' ', ' ',
                                                1, 'S', 'L', 'I', ' ', ' ', ' ', ' ',
                                                1, 'E', 'X', 'I', 'T', ' ', ' ', ' ',
                                                1, 'H', 'E', 'L', 'P', ' ', ' ', ' ',
                                                1, 'Q', 'U', 'I', 'T', ' ', ' ', ' ',
                                                1, 'R', 'E', 'G', 'S', ' ', ' ', ' ',
                                                2, 'M', 'O', 'D', 'E', ' ', ' ', ' ',
                                                2, 'M', 'T', 'R', 'R', ' ', ' ', ' ',
                                                1, 'A', 'G', 'P', ' ', ' ', ' ', ' ',
                                                1, 'C', 'L', 'O', 'C', 'K', ' ', ' ',
                                                ASCII_EOF
                                        };

COMMANDFN       Jump_Table[] =          {
                                                PCI_Command,
                                                WR_Command,
                                                SLI_Command,
                                                Exit_Command,
                                                Help_Command,
                                                Quit_Command,
                                                Reg_Command,
                                                Mode_Command,
                                                Mtrr_Command,
                                                AGP_Command,
                                                Clock_Command,
                                        };

char*           Help_Table[] =          {
                                                PCI_Hlp,
                                                WR_Hlp,
                                                SLI_Hlp,
                                                Exit_Hlp, 
                                                Help_Hlp,
                                                Quit_Hlp,
                                                Reg_Hlp,
                                                Mode_Hlp,
                                                Mtrr_Hlp,
                                                Agp_Hlp,
                                        };


// Global data


#pragma VxD_DEBUG_ONLY_CODE_SEG
#pragma VxD_DEBUG_ONLY_DATA_SEG

#define GET_INPUT               (0x01)
#define CHECK_INPUT             (0x03)
#define DEBUG_KERNEL            (0x41)

/*----------------------------------------------------------------------
Function name:  Get_Input

Description:    
                
Information:    uses in-line asm.

Return:         returns input
----------------------------------------------------------------------*/
DWORD VXDINLINE Get_Input()
{
   DWORD dwReturn;

   __asm mov     eax,GET_INPUT
   __asm int     DEBUG_KERNEL
   __asm mov     dwReturn, eax
   return (dwReturn);
}

/*----------------------------------------------------------------------
Function name:  Check_Input

Description:    
                
Information:    uses in-line asm.

Return:         returns input
----------------------------------------------------------------------*/
DWORD VXDINLINE Check_Input()
{
   DWORD dwReturn;

   __asm mov     eax,CHECK_INPUT
   __asm int     DEBUG_KERNEL
   __asm mov     dwReturn, eax
   return (dwReturn);
}

/*----------------------------------------------------------------------
Function name:  Out_Debug_Chr

Description: Give the user some feedback    
                
Information:    uses in-line asm.

Return:         returns nothing
----------------------------------------------------------------------*/
void VXDINLINE Output_Debug_Chr(BYTE bCh)
{
   __asm mov     al, bCh
   VMMCall (Out_Debug_Chr);
   return;
}

/*----------------------------------------------------------------------
Function name:  Upper_Case

Description: Convert a string to upper case
                
Information:    Nothing

Return:         
----------------------------------------------------------------------*/
#define TOUPPER(ch) (ch) = (((ch >= 'a') && (ch <= 'z')) ? (ch) - 'a' + 'A' : (ch))
void Upper_Case(BYTE bLength, BYTE * pStr)
{
   for (;bLength != 0; bLength--)
      {
      TOUPPER(*pStr);
      pStr++;
      }
}

/*******************************************************************************
*
*       Get_Parameter
*
*       Input:
*               //TODO: Input arguments
*
*       Output:
*               //TODO: Output arguments
*
*       //TODO: Pseudocode
*
*******************************************************************************/
BYTE Get_Parameter(BYTE* input, BYTE* inOffset, BYTE  paramTerm, BYTE* delimTable, BYTE  buffTerm, BYTE* output, BYTE  outBuffLen, BYTE  maxParamLen, BYTE* actualDelim)
{
   BYTE *pIn;
   BYTE *pOut;
   BYTE *pDelim;

   pIn = input + *inOffset;
   pOut = output;
        
   if(outBuffLen > 1)
      {
      while((*pIn == ASCII_SPACE) || (*pIn == ASCII_HT))
         pIn++;

      if(maxParamLen == 0)
         maxParamLen = outBuffLen;

      while(*input != buffTerm)
         {
         pDelim = delimTable;
         while((*pDelim != TAB_END) && (*pDelim != *pIn))
            pDelim++;

         if(*pDelim == TAB_END)
            {
            if((*pIn != ASCII_SPACE) && (*pIn != ASCII_HT))
               {
               if(outBuffLen > 1)
                  {
                  *pOut++ = *pIn;
                  outBuffLen--;
                  if(--maxParamLen == 0)
                     break;
                  }
               }
                                
            pIn++;
            }
         else
            break;
         }
                
         *pOut = buffTerm;
         if(actualDelim != NULL)
            *actualDelim = *input;
         *inOffset = (BYTE)((DWORD)pIn - (DWORD)input);
         return (BYTE)((DWORD)pOut - (DWORD)output);
      }
        
   if(outBuffLen > 0)
      *output = buffTerm;
   if(actualDelim != NULL)
      *actualDelim = buffTerm;
   *inOffset = (BYTE)((DWORD)pIn - (DWORD)input);
   return 0;
}

/*******************************************************************************
*
*       Match
*
*       Input:
*               terminator      -       Terminator byte
*               table           -       Pointer to the match table
*               str             -       Pointer to the string to match
*
*       Output:
*               len             -       Pointer to a byte to return the
*                                       match length used in
*               Return          -       The match table entry on success or
*                                       0xff on failure
*
*       Find the string length
*       Get the table entry count
*       Initialize the match entry
*       Get a pointer to the first table entry
*       DO
*               Initialize this entry as matching
*               FOREACH character in str until the string length or
*                                              the match length is reached
*                       IF the string character doesn't match the match character
*                               Flag the entry as not matching
*                               End the loop
*                       ENDIF
*               ENDFOREACH
*               IF the entry matched
*                       IF the return match length isn't null
*                               Copy the length
*                       ENDIF
*                       RETURN the match entry index
*               ENDIF
*               Goto the next entry
*               Get a pointer to the next entry
*       WHILE the table entry pointer doesn't point to the end of the table
*       RETURN failure
*
*******************************************************************************/
BYTE Match(BYTE terminator, BYTE* table, BYTE* str, BYTE *len)
{
   BYTE* pEntry;
   BYTE nEntry;
   DWORD strLen;
   DWORD i;
   BOOL bMatch;
   BYTE size;

   for(strLen=0; str[strLen]!=terminator; strLen++)
      ;

   size = table[0];
   nEntry = 0;
   pEntry = table + 1;
   do
      {
      bMatch = TRUE;
      for(i=0; (i<strLen)&&(i<pEntry[0]); i++)
         {
         if(pEntry[i+1] != str[i])
            {
            bMatch = FALSE;
            break;
            }
         }

      if(bMatch)
         {
         if(len != NULL)
            *len = pEntry[0];

         return nEntry;
         }

      nEntry++;
      pEntry = table + (nEntry * size) + 1;
      }

   while(*pEntry != ASCII_EOF)
      ;

   return 0xff;
}

/*******************************************************************************
*
*       Napalm_Debug
*
*       Input:
*
*       Output:
*
*       WHILE 1
*               Print the debug prompt
*               Call to get debug input
*               IF the call succeedes
*                       IF the length greater than zero
*                               Set the offset to zero
*                               Get the first parameter
*                               IF the parameter length is greater than zero
*                                       Convert the parameter to upper case
*                                       Find a match in the Command_Table
*                                       IF there was a match
*                                               Jump to the appropriate command
*                                       ELSE
*                                               Print an error message
*                                       ENDIF
*                               ENDIF
*                       ENDIF
*               ELSE
*                       RETURN
*               ENDIF
*       ENDWHILE
*
*******************************************************************************/
void Napalm_Debug()
{
   DWORD length;
   DWORD paramLen;
   BYTE offset;
   BYTE entry;
        
   while(1)
      {
      PF(Debug_Prompt);
      if(Debug_Input(DEBUG_LENGTH, Debug_Buffer, &length))
         {
         if(length > 0)
            {
            offset = 0;
                                
            paramLen = Get_Parameter(Debug_Buffer, &offset, 0, Delimiter_Table, 0, Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);
            if(paramLen > 0)
               {
               Upper_Case((BYTE)paramLen, Parm_Buffer);
               entry = Match(0, Command_Table, Parm_Buffer, NULL);
               if(entry != 0xff)
                  {
                  if(Jump_Table[entry](offset))
                     return;
                  }
               else
                  {
                  PF(Unknown_Fmt, *Unknown_Arg);
                  }
               }
            }
         }
      else
         return;
      }
}

/*******************************************************************************
*
*       Debug_Input
*
*       Input:
*               bufflen         -       Buffer length
*               pBuff           -       Buffer to recieve debug input
*               pInLen          -       Pointer to dword to recieve input length
*
*       Output:
*               Return          -       TRUE to indicate success
*
*       Set the shadow input length to zero
*       DO
*               Get a character
*               SWITCH on the character
*               null:   IF the input length pointer isn't null
*                               Set it to the local shadow
*                       RETURN false
*               escape: Set the input length to zero
*                       Set the input char to null
*               cr:     Set the input char to null
*               bs:     IF the input length is greater than 1
*                               Decrement the input length
*                               Increment the buffer size
*                               Output the backspace char
*                       ENDIF
*               default:IF the buffer size is greater than one
*                               Store the character
*                               Increment the input length
*                               Decrement the buffer size
*                               Output the character
*                       ENDIF
*               ENDSWITCH
*       WHILE the character isn't null
*       Terminate the input string with a null
*       IF the input length pointer isn't null
*               Copy the shadow input to the pointer
*       ENDIF
*       RETURN true
*
*******************************************************************************/
BOOL Debug_Input(DWORD buffLen, char *pBuff, DWORD *pInLen)
{
   DWORD inLen;
   char c;

   inLen = 0;

   do
      {
      c = (BYTE)Get_Input();
      switch(c)
         {
         case ASCII_NULL:
            if(pInLen != NULL)
               *pInLen = inLen;
            return FALSE;
      
         case ASCII_ESCAPE:
            inLen = 0;
            c = ASCII_NULL;
            break;

         case ASCII_CR:
            c = ASCII_NULL;
            break;

         case ASCII_BS:
            if(inLen > 0)
               {
               inLen--;
               buffLen++;
               Output_Debug_Chr(c);
               }
            break;

         default:
            if(buffLen > 0)
               {
               pBuff[inLen] = c;
               inLen++;
               buffLen--;
               Output_Debug_Chr(c);
               }
         }
      }
   while(c != ASCII_NULL);

   pBuff[inLen] = ASCII_NULL;
   if(pInLen != NULL)
      *pInLen = inLen;

   return TRUE;
}

/*******************************************************************************
*
*       Check_Abort
*
*       Input:
*
*       Output:
*               Return          -       TRUE if escape has been psuhed
*
*       Check the input character
*       IF the character if the escape character
*               RETURN true
*       ELSE
*               RETURN false
*       ENDIF
*
*******************************************************************************/
BOOL Check_Abort()
{
   return Check_Input() == ASCII_ESCAPE;
}

/*******************************************************************************
*
*       Exit_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL Exit_Command(BYTE inOffset)
{
   return TRUE;
}

/*******************************************************************************
*
*       Help_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       Get the next parameter
*       IF the parameter length is greater than zero
*               Convert the parameter to upper case
*               Find a match in Command_Table
*               IF a match was found
*                       Print the appropriate string out of the help table
*               ELSE
*                       Print an error message
*               ENDIF
*       ELSE
*               Print the help table
*       ENDIF
*       RETURN false
*
*******************************************************************************/
BOOL Help_Command(BYTE inOffset)
{
   BYTE length;
   BYTE entry;
   int i;         

   length = Get_Parameter(Debug_Buffer, &inOffset, 0, Delimiter_Table,
                          0,
                          Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);

   if (length > 0)
      {
      Upper_Case(length, Parm_Buffer);
      entry = Match(0, Command_Table, Parm_Buffer, &length);
      if (entry != 0xff)
         {
         PF(Help_Table[entry]);
         }
      else
         {
         PF(Unknown_Fmt, *Unknown_Arg);
         }
      }
   else
      {
      for (i=0; i<sizeof(Help_Fmt)/sizeof(char *); i++)
         PF("%s", Help_Fmt[i]);
      }

   return FALSE;
}

/*******************************************************************************
*
*       Quit_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL Quit_Command(BYTE inOffset)
{
   return TRUE;
}

/*----------------------------------------------------------------------
Function name:  ConvertHex

Description: Convert a string to upper case
                
Information:    Nothing

Return:         
----------------------------------------------------------------------*/
#define RANGE(lo, val, hi) ((val >= lo) && (val <= hi))
DWORD ConvertHex(BYTE *pBuffer, DWORD dwLength)
{
   DWORD dwReturn;
   for (dwReturn = 0; dwLength!=0; dwLength--)
      {
      dwReturn <<= 4;

      if (RANGE('a', *pBuffer, 'f'))
         dwReturn += (*pBuffer - 'a' + 10);
      else if (RANGE('A', *pBuffer, 'F'))
         dwReturn += (*pBuffer - 'A' + 10);
      else if (RANGE('0', *pBuffer, '9'))
         dwReturn += (*pBuffer - '0');
      else
         {
         dwReturn = 0xFFFFFFFF;
         dwLength = 1;
         }

      pBuffer++;
      }

   return (dwReturn);
}


DWORD PCIOffset[] = {
   0x00,
   0x04,
   0x08,
   0x0c,
   0x10,
   0x14,
   0x18,
   0x2C,
   0x30,
   0x34,
   0x3C,
   0x40,
   0x44,
   0x48,
   0x4C,
   0x50,
   0x54,
   0x58,
   0x5C,
   0x60,
   0x64,
   0x80,
   0x84,
   0x88,
   0x8c,
   0x90,
   0x94,
   0x98,
   0x9C,
   0xA0,
   0xA4,
   0xA8,
   0xAC,
   };

char * ppDeviceStr[] = {
   "Device ID/Vendor ID %08x\n",    // Offset 0x00
   "Status/Command %08x\n",         // Offset 0x04
   "Class_Code/Revision ID %08x\n",         // Offset 0x08
   "BIST/Header Type/Latency Timer/Cache_line_size %08x\n",         // Offset 0x0c
   "memBaseAddr0 %08x\n",         // Offset 0x10
   "memBaseAddr1 %08x\n",         // Offset 0x14
   "IoBaseAddr %08x\n",         // Offset 0x18
   "SubSystemID/SubVendorID %08x\n",         // Offset 0x2C
   "RomBaseAddr %08x\n",         // Offset 0x30
   "Cap Ptr %08x\n",         // Offset 0x34
   "Max_lat/Min_gnt/Interrupt_Pin/Interrupt_Line %08x\n",         // Offset 0x3C
   "cfgInitEnable/FABID %08x\n",         // Offset 0x40
   "ACPI Reset %08x\n",         // Offset 0x44
   "cfgPciDecode %08x\n",         // Offset 0x48
   "CfgStatus %08x\n",         // Offset 0x4C
   "CfgScratch %08x\n",         // Offset 0x50
   "AGP CAP_ID %08x\n",         // Offset 0x54
   "AGP status %08x\n",         // Offset 0x58
   "AGP_Cmd %08x\n",         // Offset 0x5C
   "ACPI Cap ID %08x\n",         // Offset 0x60
   "ACPI cntrl/status %08x\n",         // Offset 0x64
   "cfgVideoCtrl0 %08x\n",         // Offset 0x80
   "cfgVideoCtrl1 %08x\n",         // Offset 0x84
   "cfgVideoCtrl2  %08x\n",         // Offset 0x88
   "cfgSliLfbCtrl %08x\n",         // Offset 0x8c
   "cfgAaDepthBufferAperture %08x\n",         // Offset 0x90
   "cfgAALfbCtrl %08x\n",         // Offset 0x94
   "agpTestCtrl %08x\n",         // Offset 0x98
   "agpTestData0 %08x\n",         // Offset 0x9C
   "agpTestData1 %08x\n",         // Offset 0xA0
   "agpTestData2 %08x\n",         // Offset 0xA4
   "agpTestData3 %08x\n",         // Offset 0xA8
   "cfgSliAAmisc %08x\n",         // Offset 0xAC
   };

/*----------------------------------------------------------------------
Function name:  DevicePrint

Description: 
                
Information:    Nothing

Return:         
----------------------------------------------------------------------*/
void DevicePrint(DWORD dwBusNum, DWORD dwDevNum)
{
   DWORD dwFunc;
   DWORD dwDevID;
   DWORD dwReg;
   DWORD dwDevFunc;
   DWORD i;


   for (dwFunc = 0; dwFunc < 8; dwFunc++)
      {
      dwDevID = PCI_Read_Config(dwBusNum, (dwDevNum << 3) | dwFunc, 0x0);
      if (IS_NAPALM(dwDevID) || IS_VOODOO3(dwDevID))
         {
         dwDevFunc = (dwDevNum << 3) | dwFunc;
         for (i=0; i<sizeof(PCIOffset)/sizeof(DWORD); i++)
            {
            dwReg = PCI_Read_Config(dwBusNum, (dwDevNum << 3) | dwFunc, PCIOffset[i]);
            PF(ppDeviceStr[i], dwReg);
            }
         }
      }
}

/*----------------------------------------------------------------------
Function name:  PCIPrint

Description: 
                
Information:    Nothing

Return:         
----------------------------------------------------------------------*/
void PCIPrint(DWORD dwBusNum, DWORD dwDevNum)
{
   DWORD dwBus;
   DWORD dwDevFunc;
   DWORD dwDevID;

   if ((0xFFFFFFFF != dwBusNum) && (0xFFFFFFFF != dwDevNum))
      {
      DevicePrint(dwBusNum, dwDevNum);
      }
  else
      {
      for (dwBus = 0; dwBus < 256; dwBus++)
         for (dwDevFunc = 0; dwDevFunc<0x100; dwDevFunc += 0x08)
            {
            dwDevID = PCI_Read_Config(dwBus, dwDevFunc, 0x0);
            if (IS_NAPALM(dwDevID) || IS_VOODOO3(dwDevID))
               DevicePrint(dwBus, dwDevFunc >> 3);
            }
      }
}     

/*******************************************************************************
*
*       PCI_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL PCI_Command(BYTE inOffset)
{
   DWORD length;
   DWORD BusNum;
   DWORD DevNum;

   BusNum = 0xFFFFFFFF;
   DevNum = 0xFFFFFFFF;
   length = Get_Parameter( Debug_Buffer, &inOffset, 0, Delimiter_Table,
                                0,
                                Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);

   if (length > 0)
      {
      BusNum = ConvertHex(Parm_Buffer, length);

      length = Get_Parameter( Debug_Buffer, &inOffset, 0, Delimiter_Table,
                                   0,
                                   Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);

      if (length > 0)
         {
         DevNum = ConvertHex(Parm_Buffer, length);
         }
      }
      
    PCIPrint(BusNum, DevNum);
      
    return FALSE;
}



/*******************************************************************************
*
*       WR_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL WR_Command(BYTE inOffset)
{
   DWORD length;
   DWORD BusNum;
   DWORD DevNum;
   DWORD FuncNum;
   DWORD RegNum;
   DWORD Value;
   DWORD dwWrite;
   DWORD dwReg1;
   DWORD dwReg2;
   DWORD i;
   DWORD * dwP[5] = {&BusNum, &DevNum, &FuncNum, &RegNum, &Value};

   dwWrite = 1;
   for (i=0; i<5; i++)
      {
      *dwP[i] = 0xFFFFFFFF;
      length = Get_Parameter( Debug_Buffer, &inOffset, 0, Delimiter_Table,
                                0,
                                Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);
      if (length > 0)
         *dwP[i] = ConvertHex(Parm_Buffer, length);
      if (0xFFFFFFFF == *dwP[i])
         dwWrite = 0x0;
      }

   if (dwWrite)
      {
      dwReg1 = PCI_Read_Config(BusNum, (DevNum << 3) | FuncNum, RegNum);
      PCI_Write_Config(BusNum, (DevNum << 3) | FuncNum, RegNum, Value);
      dwReg2 = PCI_Read_Config(BusNum, (DevNum << 3) | FuncNum, RegNum);
      PF("\nPCI Bus %08x Device %08x Func %08x Register %08x was %08x set to %08x is %08x\n",
         BusNum, DevNum, FuncNum, RegNum, dwReg1, Value, dwReg2);
      }
   else
      {
      PF("\nError in Parameter PCI Bus %08x Device %08x Func %08x Register %08x\n",
         BusNum, DevNum, FuncNum, RegNum);
      }
    return FALSE;
}

/*----------------------------------------------------------------------
Function name:  SLIDevicePrint

Description: 
                
Information:    Nothing

Return:         
----------------------------------------------------------------------*/
#define FIELD(val, pos, size) (((val) >> pos) & size)
void SLIDevicePrint(DWORD dwBusNum, DWORD dwDevNum)
{
   DWORD dwFunc;
   DWORD dwDevID;
   DWORD dwReg;
   DWORD dwDevFunc;

   for (dwFunc = 0; dwFunc < 8; dwFunc++)
      {
      dwDevID = PCI_Read_Config(dwBusNum, (dwDevNum << 3) | dwFunc, 0x0);
      if (IS_NAPALM(dwDevID) || IS_VOODOO3(dwDevID))
         {
         dwDevFunc = (dwDevNum << 3) | dwFunc;
         PF("\nBus %d Device %d Function %d\n", dwBusNum, dwDevNum, dwFunc);
         PF("Memory Config Base0 %08x Base1 %08x IoBase %08x\n", PCI_Read_Config(dwBusNum, dwDevFunc, 0x10), PCI_Read_Config(dwBusNum, dwDevFunc, 0x14), PCI_Read_Config(dwBusNum, dwDevFunc, 0x18));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_INIT_ENABLE);
         PF("cfgInitEnabled %08x\n", dwReg);
         PF("Enable HW Init Writes %x PCI Fifo %x MemSize %x Snooped HW Init %x Disable LFB read Cache %x\n", FIELD(dwReg,8,1) , FIELD(dwReg,9,1), FIELD(dwReg,10,1), FIELD(dwReg,30,1), FIELD(dwReg,29,1));
         PF("Address Snoop Enable %x MemBase0 %x MemBase1 %x Master/Slave %x\n", FIELD(dwReg, 11, 1), FIELD(dwReg, 12, 1), FIELD(dwReg, 13, 1), FIELD(dwReg, 14, 1));
         PF("Snoop Address 0 %08x\n", FIELD(dwReg, 15, 0x3FF));         
         PF("Swap Buffer Algorithm %x Swap Master %x Quick Sample %x PCI Multi Func bit %x\n", FIELD(dwReg, 25, 1), FIELD(dwReg, 26, 1), FIELD(dwReg, 27, 1), FIELD(dwReg, 28, 1));         
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_PCI_DECODE);
         PF("cfgPciDecode %08x\nSize: MemBase0 %x MemBase1 %x IOBase %x\nSnoop Size: Membase0 %x Membase1 %x\nSnoop Address 1 %x\n", dwReg,
            FIELD(dwReg, 0, 0x0F), FIELD(dwReg, 4, 0x0F), FIELD(dwReg, 8, 0x07), FIELD(dwReg, 10, 0x0F), FIELD(dwReg, 14, 0x0F), 
            FIELD(dwReg, 18, 0x3FF));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_VIDEO_CTRL0);
         PF("cfgVideoCtrl0 %08x\nEnhanced Video Enable %x Slave %x TV Output %x Local Mux Sel %x\n",
            dwReg, FIELD(dwReg, 0, 1), FIELD(dwReg, 1, 1), FIELD(dwReg, 2, 1), FIELD(dwReg, 3, 1));
         PF("OtherMux Sel True %x False %x\nSLI Compare Invert Fetch %x CRT %x AAFifo %x VidPll %x\n",
            FIELD(dwReg, 4, 3), FIELD(dwReg, 6, 3), FIELD(dwReg, 8, 1), FIELD(dwReg, 9, 1), FIELD(dwReg, 10, 1), FIELD(dwReg, 11, 1));
         PF("Divide Video %x Always Drive AABus %x Vsync Ref %x DAC TriState Vsync %x Hsync %x\n",
            FIELD(dwReg, 12, 0x7), FIELD(dwReg, 15, 1), FIELD(dwReg, 16, 0x0F), FIELD(dwReg, 24, 1), FIELD(dwReg, 25, 1));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_VIDEO_CTRL1);
         PF("cfgVideoCtrl1 %08x\nSLI Fetch RenderMask %x CompareMask %x\nSLI CRT RenderMask %x CompareMask %x\n", dwReg, FIELD(dwReg, 0, 0xFF), FIELD(dwReg, 8, 0xFF), FIELD(dwReg, 16, 0xFF), FIELD(dwReg, 24, 0xFF));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_VIDEO_CTRL2);
         PF("cfgVideoCtrl2 %08x\nSLI Fetch AAFifo RenderMask %x CompareMask %x\n", dwReg, FIELD(dwReg, 0, 0xFF), FIELD(dwReg, 8, 0xFF));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_SLI_LFB_CTRL);
         PF("cfgSliLfbCtrl %08x\nSLI Lfb Masks Render %x Compare %x Scan %x NumChips Log2 %x\nSLI LFB Enable CPU %x Dispatch %x Rd %x\n",
            dwReg, FIELD(dwReg, 0, 0xFF), FIELD(dwReg, 16, 0xFF), FIELD(dwReg, 24, 0xFF), FIELD(dwReg, 24, 3), FIELD(dwReg, 1, 26), FIELD(dwReg, 27, 1), FIELD(dwReg, 28, 1));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_AA_ZBUFF_APERTURE);
         PF("cfgAaDepthBufferAperature %08x\nBegin %x End %x\n", dwReg, FIELD(dwReg, 0, 0x7FFF), FIELD(dwReg, 16, 0xFFFF));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_AA_LFB_CTRL);
         PF("cgfAALfbCtrl %08x\nBase Addr %x\nWrite Enable CPU %x Dispatch %x Rd %x\nFormat %x Divide By 4 %x\n",
            dwReg, FIELD(dwReg, 3, 0x3FFFFF), FIELD(dwReg, 26, 1), FIELD(dwReg, 27, 1), FIELD(dwReg, 28, 1), FIELD(dwReg, 29, 3), FIELD(dwReg, 31, 1));
         dwReg = PCI_Read_Config(dwBusNum, dwDevFunc, CFG_SLI_AA_MISC);
         PF("cfgSliAaMisc %08x\nVga Vsync Offset %x\nHot Plug %x HotPlug Pin %x Wait for AA %x\n",
            dwReg, FIELD(dwReg, 0, 0x1FF), FIELD(dwReg, 9, 3), FIELD(dwReg, 11, 1), FIELD(dwReg, 12, 1));
         }
      }
}

/*----------------------------------------------------------------------
Function name:  SLIPrint

Description: 
                
Information:    Nothing

Return:         
----------------------------------------------------------------------*/
void SLIPrint(DWORD dwBusNum, DWORD dwDevNum)
{
   DWORD dwBus;
   DWORD dwDevFunc;
   DWORD dwDevID;

   if ((0xFFFFFFFF != dwBusNum) && (0xFFFFFFFF != dwDevNum))
      {
      SLIDevicePrint(dwBusNum, dwDevNum);
      }
  else
      {
      for (dwBus = 0; dwBus < 256; dwBus++)
         for (dwDevFunc = 0; dwDevFunc<0x100; dwDevFunc += 0x08)
            {
            dwDevID = PCI_Read_Config(dwBus, dwDevFunc, 0x0);
            if (IS_NAPALM(dwDevID) || IS_VOODOO3(dwDevID))
               SLIDevicePrint(dwBus, dwDevFunc >> 3);
            }
      }
}     

/*******************************************************************************
*
*       SLI_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL SLI_Command(BYTE inOffset)
{
   DWORD length;
   DWORD BusNum;
   DWORD DevNum;

   BusNum = 0xFFFFFFFF;
   DevNum = 0xFFFFFFFF;
   length = Get_Parameter( Debug_Buffer, &inOffset, 0, Delimiter_Table,
                                0,
                                Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);

   if (length > 0)
      {
      BusNum = ConvertHex(Parm_Buffer, length);

      length = Get_Parameter( Debug_Buffer, &inOffset, 0, Delimiter_Table,
                                   0,
                                   Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);

      if (length > 0)
         {
         DevNum = ConvertHex(Parm_Buffer, length);
         }
      }
      
    SLIPrint(BusNum, DevNum);
      
    return FALSE;
}


/*----------------------------------------------------------------------
Function name:  our_outpb

Description:    write a byte

Information:    

Return:         write a byte
                        
----------------------------------------------------------------------*/
void our_outpb(DWORD addr, DWORD dwData)
{
   _asm {mov   edx, addr}
   _asm {mov   eax, dwData}
   _asm {out   dx, al}
}

/*----------------------------------------------------------------------
Function name:  our_inpb

Description:    read a byte

Information:    

Return:         read a byte  
                        
----------------------------------------------------------------------*/
BYTE our_inpb(DWORD Addr) 
{
   BYTE bReturn;

   _asm {mov   edx, Addr}
   _asm {xor   eax, eax}
   _asm {in    al, dx}
   _asm {mov   bReturn, al}

   return bReturn;
}


/*******************************************************************************
*
*       Reg_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL Reg_Command(BYTE inOffset)
{
   int i;
   BYTE iByte;

   PF("\nCRT Registers\n");
   for (i=0; i<0x1C; i++)
      {
      if (0x19 == i)
         continue;
      our_outpb(0x3d4, i);
      iByte = our_inpb(0x3d5);
      PF("CR%02x=%02x%c", i, iByte, ((i+1)%11==0)? '\n':' ');
      }

   PF("\nSR Registers\n");
   for (i=0; i<0x4; i++)
      {
      our_outpb(0x3c4, i);
      iByte = our_inpb(0x3c5);
      PF("SR%02x=%02x%c", i, iByte, ((i+1)%11==0)? '\n':' ');
      }

   PF("\nGR Registers\n");
   for (i=0; i<0x8; i++)
      {
      our_outpb(0x3ce, i);
      iByte = our_inpb(0x3cf);
      PF("GR%02x=%02x%c", i, iByte, ((i+1)%11==0)? '\n':' ');
      }

   PF("\nAR Registers\n");
   for (i=0; i<0x14; i++)
      {
      our_inpb(0x3da);
      our_outpb(0x3c0, i);
      iByte = our_inpb(0x3c1);
      PF("AR%02x=%02x%c", i, iByte, ((i+1)%11==0)? '\n':' ');
      }

   PF("\nMisc Output %02x\n", our_inpb(0x3cc));

   return FALSE;
}

/*******************************************************************************
*
*       Mode_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
#ifdef SLI_AA
void SetVideoMode(VidProcConfig *pVpc, PDEVTABLE pDev);
#endif
BOOL Mode_Command(BYTE inOffset)
{
#ifdef SLI_AA
   int i;
   DWORD vidProcCfg;
   DWORD Palette[256];
   DWORD dacData;
   DWORD dacAddr;
   SstIORegs *lpIORegs = DevNodeandUnitNumtoIORegs(DevTable[0].dwDevNode, 0x0);
   
   vidProcCfg = lpIORegs->vidProcCfg;

   for (i=0; i<256; i++)
      {
      lpIORegs->dacAddr = i;
      dacAddr = lpIORegs->dacAddr;
      Palette[i] = lpIORegs->dacData;
      dacData = lpIORegs->dacData;
      }
   
   h3InitSetVideoMode(&DevTable[0], 640, 480, 60, 0x0, 0);
   SetVideoMode(&DevTable[0].Vpc, &DevTable[0]);
   lpIORegs->vidProcCfg = vidProcCfg;

   for (i=0; i<256; i++)
      {
      lpIORegs->dacAddr = i;
      dacAddr = lpIORegs->dacAddr;
      lpIORegs->dacData = Palette[i];
      dacData = lpIORegs->dacData;
      }
 #endif
   return FALSE;
}

/*******************************************************************************
*
*       Mtrr_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
#ifdef WINNT
extern FxU32 isP6;	// OLD CPUID CODE
#else
extern CPU_FEATURES cpuFeatures;
#endif

BOOL Mtrr_Command(BYTE inOffset)
{
   DWORD Mtrr;
   DWORD mtrr1Lo;
   DWORD mtrr1Hi;
   DWORD mtrr2Lo;
   DWORD mtrr2Hi;

#ifdef WINNT	// OLD CPUID CODE
   if (!isP6)
      {
      PF("Processor is not a P6 or greater\n");
      }
   else if (isP6 == P6_AMDK6_MTRRS)
      {
      PF("Processor is not a Genuine Intel or greater\n");
      }
#else
   if (! (cpuFeatures.dwCpuFlags & CPU_FEATURE_P6_MTRR))
      {
      PF("Processor does not support P6-style MTRRs\n");
	  }
#endif
   else
      {
      PF("\n");
      for (Mtrr = 0x200; Mtrr <=0x20e; Mtrr+=2)
         {
		   GetMTRR(Mtrr, &mtrr1Lo, &mtrr1Hi);
		   GetMTRR(Mtrr + 1, &mtrr2Lo, &mtrr2Hi);    
         PF("%08x %08x %08x\n%08x %08x %08x\n", Mtrr, mtrr1Lo, mtrr1Hi, Mtrr+1, mtrr2Lo, mtrr2Hi);
         }
      }
   return FALSE;
}
/*******************************************************************************
*
*       AGP_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL AGP_Command(BYTE inOffset)
{
   PDEVTABLE pDev;
   DWORD dwData;
   int i;
   
   PF("\nHost Chip Set %08x\n", PCI_Read_Config(0x0, 0x0,  0x0));
   PF("AGP Cap %08x\n", PCI_Read_Config(0x0, 0x0,  0xA0));
   PF("AGP Status %08x\n", PCI_Read_Config(0x0, 0x0,  0xA4));
   PF("AGP Command %08x\n", PCI_Read_Config(0x0, 0x0,  0xA8));

   for (i=0; i<MAX_BANSHEE_DEVICES; i++)
      {
      pDev = &DevTable[i];
      if (0x0 != pDev->dwDevNode)
         {
         if (0x0 == pDev->dwPCI)
            {
            PF("AGP Card Found Bus %x DevFunc %x\n", pDev->dwBus, pDev->dwDevFunc);
            dwData = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc,  0x58);
            PF("AGP Status %x\n", dwData);
            dwData = PCI_Read_Config(pDev->dwBus, pDev->dwDevFunc,  0x5c);
            PF("AGP Command %x\n", dwData);
            }
         }
      }
   return FALSE;
}

/*******************************************************************************
*
*       Clock_Command
*
*       Input:
*               inOffset        -       Offset into the input buffer to
*                                       start searching for the next parameter
*
*       Output:
*               Return          -       TRUE to end the debug loop and
*                                       return to the debugger
*
*       RETURN true
*
*******************************************************************************/
BOOL Clock_Command(BYTE inOffset)
{
   DWORD length;
   DWORD dwWrite;
   DWORD s;
   DWORD v;
   DWORD r;
   DWORD i;
   DWORD * dwP[3] = {&s, &v, &r};
   extern DWORD S2S1S0[];

   dwWrite = 1;
   for (i=0; i<3; i++)
      {
      *dwP[i] = 0xFFFFFFFF;
      length = Get_Parameter( Debug_Buffer, &inOffset, 0, Delimiter_Table,
                                0,
                                Parm_Buffer, DEBUG_LENGTH - 1, 0, NULL);
      if (length > 0)
         *dwP[i] = ConvertHex(Parm_Buffer, length);
      if (0xFFFFFFFF == *dwP[i])
         dwWrite = 0x0;
      }

   if (dwWrite)
      {
      Output_Clock(&DevTable[0], 0x0, 0x1, 0x0, S2S1S0[s], v, r);
      PF("\nNew Clock s=%x v=%x r=%x\n", S2S1S0[s], v, r);
      }
   else
      {
      PF("\nError in Parameter Clock s=%08x v=%08x r=%08x\n",
         s, v, r);
      }
    return FALSE;
}









