LCINCS  = -I$(BUILD_ROOT_ARCADE)\include -I$(BUILD_ROOT)\$(FX_GLIDE_HW)\include \
          -I$(BUILD_ROOT_SWLIBS)\include -I$(BUILD_ROOT_GCI)\include \
          -I$(BUILD_ROOT_ARCADE)\src\atd

!if "$(DEBUG)"=="1"
LCDEFS  = -DAT_DEBUGGING $(LCDEFS)
!else
LCDEFS  = -DAT_PRODUCTION $(LCDEFS)
!endif

!if "$(AT_NEW_ATS)" == "1"
LCDEFS  = -DNEWATS $(LCDEFS)
!endif

!if "$(AT_SOUND_DIRECTX)"=="1"
LCDEFS  = -DSOUND
!endif

!if "$(FX_GLIDE3)" != ""
LCDEFS  = -DGLIDE3 $(LCDEFS)
!endif

!if "$(FX_TARGET)" == "WIN32"
MAKEHELP = $(BUILD_ROOT_ARCADE)\build\makehelp.bat
ADDLIB = wsock32.lib
LCDEFS  = -DNETWORK $(LCDEFS)
!if "$(HWC_CSIM)"!="1" 
FX_TARGET_MINOR=WIN95
!endif
RCFILE  = $(BUILD_ROOT_ARCADE)\src\atd\atdemo.rc
!endif

LLDLIBS = $(BUILD_ROOT_ARCADE)\lib\atu.lib \
          $(BUILD_ROOT_ARCADE)\lib\ata.lib \
!if "$(FX_TARGET)" == "WIN32"
          $(BUILD_ROOT_ARCADE)\lib\atg.lib \
!endif
!if "$(AT_LOADER_MG)" == "1"
          $(BUILD_ROOT_ARCADE)\lib\atcmg.lib \
          $(MGSDK)\dist\lib\apilib.lib \
          $(MGSDK)\dist\lib\fltdata.lib \
!endif
          $(BUILD_ROOT_ARCADE)\lib\ati.lib \
          $(BUILD_ROOT_ARCADE)\lib\atd.lib \
          $(BUILD_ROOT_ARCADE)\lib\atm.lib \
!if "$(AT_NEW_ATS)" == "1"
          $(BUILD_ROOT_ARCADE)\lib\nats.lib \
!else
          $(BUILD_ROOT_ARCADE)\lib\ats.lib \
!endif
          $(BUILD_ROOT_ARCADE)\lib\atr.lib \
!if "$(XHEAD)"=="1" 
          $(BUILD_ROOT_ARCADE)\lib\bcstudp.lib \
!endif
!if "$(HWC_CSIM)"=="1" 
          $(BUILD_DRIVE)$(BUILD_ROOT)\$(FX_GLIDE_HW)\lib\hwc.lib
!endif

optlibs = $(ADDLIB)

TOBOF = $(BUILD_ROOT_ARCADE)\cmd\tools\tobof\tobof
