NTRemap is only needed by people with S3 868/968 boards that reserve less
address space than they actually occupy, sometimes conflicting with your
3Dfx board.

NTRemap is a device driver that remaps the 3Dfx board out of the way of
the S3 board.

First copy the file NTREMAP.SYS to \WINNT\SYSTEM32\DRIVERS

We need to edit the Windows NT registry to have NT start our driver.
If you are familiar with the registry, the NTREMAP.INI file will show you
the keys and values to add.  Here are step-by-step directions to help you:

  1. Open a DOS shell.

  2. type regedt32 at the DOS prompt.
     This should start up the registry editor.

  3. Select the HKEY_LOCAL_MACHINE window.

  4. Open the SYSTEM folder.

  5. Open the CURRENT CONTROL SET folder.

  6. Open the SERVICES folder.

  7. From the top menubar , select EDIT>ADD KEY and enter NTRemap in the
      KEY NAME field, leaving the CLASS field blank.  Press Enter.

  8. Open the NTRemap folder you just created.

  9. From the top menubar, select EDIT>ADD VALUE.  Enter ErrorControl in
      the VALUE NAME field.  Select the REG_DWORD in the DATA TYPE popup
      menu at the bottom.  Press Enter.  In the new dialog box, type in 01,
      a hexadecimal value, and press Enter.

  10. From the top menubar, select EDIT>ADD VALUE.  Enter Start in the
      VALUE NAME field.  Select the REG_DWORD in the DATA TYPE popup menu
      at the bottom.  Press Enter.  In the new dialog box, type in 02, a
      hexadecimal value, and press Enter.

  11. From the top menubar, select EDIT>ADD VALUE.  Enter Type in the
      VALUE NAME field.  Select the REG_DWORD in the DATA TYPE popup menu
      at the bottom.  Press Enter.  In the new dialog box, type in 01, a
      hexadecimal value, and press Enter.

  12. From the top menubar, select EDIT>ADD VALUE.  Enter Group in the
      VALUE NAME field.  Select the REG_SZ in the DATA TYPE popup menu at
      the bottom.  Press Enter.  In the new dialog box, type in
      Extended base , and press Enter.

  13. Open the \HK_LOCAL_MACHINE\SOFTWARE\3Dfx Interactive
      folder.  

  14. From the top menubar , select EDIT>ADD KEY and enter NTRemap in the
      KEY NAME field, leaving the CLASS field blank.  Press Enter.

  15. Open the NTRemap folder you just created.

  16. From the top menubar, select EDIT>ADD VALUE.  Enter EnableNTRemap in
      the VALUE NAME field.  Select the REG_DWORD in the DATA TYPE popup
      menu at the bottom.  Press Enter.  In the new dialog box, type in 01,
      a hexadecimal value, and press Enter.

  17. Shutdown your system and reboot.


Check the operation of the 3Dfx card.


IF YOU STILL HAVE PROBLEMS
==========================

If you still have problems it may be that NTRemap thinks it does not need to
remap your board or the address range it remaps to does not work on your
system.  You can force a remap to the address of your choice by opening the
registry folder

    \HK_LOCAL_MACHINE\SOFTWARE\3Dfx Interactive\NTRemap

    From the top menubar, select EDIT>ADD VALUE.  Enter ForceAddress in
    the VALUE NAME field.  Select the REG_DWORD in the DATA TYPE popup
    menu at the bottom.  Press Enter.  In the new dialog box, type in
    a hexadecimal value you want to map the board at (i.e. F9000000)
    and press Enter.



