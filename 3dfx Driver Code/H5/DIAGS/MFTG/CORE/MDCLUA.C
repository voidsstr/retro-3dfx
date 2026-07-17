/*
 * mdclua.c
 *
 * These are core functions exported to lua
 *
 */
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

#include <3dfx.h>
#include <fxpci.h>

#include "lua.h"
#include "luadebug.h"
#include "lualib.h"
#include "lauxlib.h"
#include "mdclua.h"
#include "init.h"
// #include <env.h>

//#include "pcibrd.h"


#include "misc.h" /* printPciInfo() */

// #include "banshee.h"
#include "errrpt.h"
#include "version.h"

#include <time.h>
#include <stdlib.h>


/* lua CFunction prototypes */

/* generic */
void mdcl_init_default_board(void);
void mdcl_pci_dev_info(void);
void mdcl_signalFailure(void);
void mdcl_command_prompt(void);
void mdcl_fbwr8(void); void mdcl_fbwr16(void); void mdcl_fbwr32(void);
void mdcl_fbrd8(void); void mdcl_fbrd16(void); void mdcl_fbrd32(void);
void mdcl_kbhit(void);
void mdcl_getch(void);
void mdcl_set_vmode(void);
void mdcl_printPciInfo(void);
void mdcl_printBoardInitFns(void);
void mdcl_flush(void);
void mdcl_restoreText(void);
void mdcl_textOutputEnable(void);
void mdcl_pciGetConfigData(void);
void mdcl_time(void);
void mdcl_system(void);
void mdcl_logError(void);
void mdcl_strtol(void);
void mdcl_strtoul(void);
void mdcl_bitwise_or(void);
void mdcl_break_requested(void);
void mdcl_logDestination(void);

static struct luastr_reg_struct luastr_list[] = {
  { "mdc_version", &mdc_version_str },
  { "mdc_builddate", &mdc_builddate_str },
  { NULL, NULL }};

static struct luafn_reg_struct luafn_list[] = {
  /* General */
  { "signalFailure", mdcl_signalFailure },
  { "pci_dev_info", mdcl_pci_dev_info },
  { "init_default_board",mdcl_init_default_board},
  { "command_prompt",mdcl_command_prompt},
  { "printPciInfo", mdcl_printPciInfo},
  { "printBoardInitFns", mdcl_printBoardInitFns},
  { "time", mdcl_time },
  { "system", mdcl_system },

  { "pciGetConfigData", mdcl_pciGetConfigData },
  { "bitor", mdcl_bitwise_or },

  { "fbwr8", mdcl_fbwr8},
  { "fbwr16", mdcl_fbwr16},
  { "fbwr32", mdcl_fbwr32},
  { "fbrd8", mdcl_fbrd8},  
  { "fbrd16", mdcl_fbrd16},
  { "fbrd32", mdcl_fbrd32},

  { "kbhit", mdcl_kbhit},
  { "getch", mdcl_getch},
  { "flush", mdcl_flush},
  { "logError", mdcl_logError } ,
  { "restoreText", mdcl_restoreText },
  { "textOutputEnable", mdcl_textOutputEnable },
  { "strtol", mdcl_strtol },
  { "strtoul", mdcl_strtoul },

  { "break_requested", mdcl_break_requested },
  { "logDestination", mdcl_logDestination },


  { NULL, NULL} // end record 
};

LPCARDINFO card = NULL;

lua_Object lua_callback_fn = 0;

void mdcl_logDestination(void) {
  char *filename = luaL_check_string(1);

  /* redirect standard output to a log  
   *
   * if we could we should grab printf output, but we'll see how
   * possible this is under DOS 
   */

    stderr_file = stdout_file = fopen(filename, "w" );

    if (!stdout_file) {
      printf("Couldn't open stdout.log!\n");
      printf("Couldn't open stderr.log!\n");
    }
}

void mdcl_bitwise_or(void) {
  unsigned long int a = luaL_check_number(1);
  unsigned long int b = luaL_check_number(2);


  lua_pushnumber((double)(a | b));
}

void mdcl_break_requested(void) {
  if (lua_isnumber(lua_getparam(1))) {
    break_requested = lua_getnumber(lua_getparam(1));
  } else {
    if (break_requested != 0) {
      lua_pushnumber(break_requested);
    } else {
      lua_pushnil();
    }
  }
}

void mdcl_time(void) {
  lua_pushnumber(time(NULL));
}


void mdcl_system(void) {
  char *s = luaL_check_string(1);

  system(s);
}


/*
 * pciConfig access
 */

struct {
  char *reg_name;
  const PciRegister *reg;
} lua_pciregs[] = {
  { "PCI_VENDOR_ID", &PCI_VENDOR_ID },
  { "PCI_DEVICE_ID", &PCI_DEVICE_ID },
  { "PCI_COMMAND", &PCI_COMMAND},
  { "PCI_STATUS", &PCI_STATUS },
  { "PCI_REVISION_ID", &PCI_REVISION_ID},
  { "PCI_CLASS_CODE", &PCI_CLASS_CODE},
  { "PCI_CACHE_LINE_SIZE", &PCI_CACHE_LINE_SIZE},
  { "PCI_LATENCY_TIMER", &PCI_LATENCY_TIMER},
  { "PCI_HEADER_TYPE", &PCI_HEADER_TYPE},
  { "PCI_BIST", &PCI_BIST},
  { "PCI_BASE_ADDRESS_0", &PCI_BASE_ADDRESS_0},
  { "PCI_BASE_ADDRESS_1", &PCI_BASE_ADDRESS_1},
  { "PCI_IO_BASE_ADDRESS", &PCI_IO_BASE_ADDRESS},
  { "PCI_SUBVENDOR_ID", &PCI_SUBVENDOR_ID},
  { "PCI_SUBSYSTEM_ID", &PCI_SUBSYSTEM_ID},
  { "PCI_ROM_BASE_ADDRESS", &PCI_ROM_BASE_ADDRESS},
  { "PCI_CAP_PTR", &PCI_CAP_PTR},
  { "PCI_INTERRUPT_LINE", &PCI_INTERRUPT_LINE},
  { "PCI_INTERRUPT_PIN", &PCI_INTERRUPT_PIN},
  { "PCI_MIN_GNT", &PCI_MIN_GNT},
  { "PCI_MAX_LAT", &PCI_MAX_LAT},
  { "PCI_FAB_ID", &PCI_FAB_ID},
  { "PCI_CONFIG_STATUS", &PCI_CONFIG_STATUS},
  { "PCI_CONFIG_SCRATCH", &PCI_CONFIG_SCRATCH},
  { "PCI_AGP_CAP_ID", &PCI_AGP_CAP_ID},
  { "PCI_AGP_STATUS", &PCI_AGP_STATUS },
  { "PCI_AGP_CMD", &PCI_AGP_CMD},
  { "PCI_ACPI_CAP_ID", &PCI_ACPI_CAP_ID},
  { "PCI_CNTRL_STATUS", &PCI_CNTRL_STATUS },
  { NULL, NULL }};

void mdcl_pciGetConfigData(void) {
  FxU32 tmp;
  int i = 0;
  char *this_name;
  char *name;

  name = luaL_check_string(1);
  mdc_l_check_card(card);

  while (this_name = lua_pciregs[i].reg_name) {
     if (!strcmp(name,this_name)) {
       pciGetConfigData( *(lua_pciregs[i].reg), card->pciDevNum, &tmp);
       lua_pushnumber(tmp);
       return;
     }
     i++;
  }

  // failure!

  {
   char s[100];
   sprintf(s,"pciGetConfigData(): no such pciregister '%s'",name);
   lua_error(s); // return
  }
}

void mdcl_strtol(void) {
  char *str = luaL_check_string(1);
  int base = luaL_check_number(3);
  
  lua_pushnumber((double)strtol(str,NULL,base));
}

void mdcl_strtoul(void) {
  char *str = luaL_check_string(1);
  int base = luaL_check_number(3);
  
  lua_pushnumber((double)strtoul(str,NULL,base));
}

void mdcl_textOutputEnable(void) {
  int new_state = 0, old_state;
  lua_Object obj;

  obj = lua_getparam(1);
  if (lua_isnil(obj)) {
    new_state = 0;
  } else {
    if (lua_isnumber(obj)) {
      new_state = lua_getnumber(obj);
    } else {
      lua_error("textOutputEnable() needs a number or nil as arg1");
      return;
    }
  }
  
  old_state = mdc_set_output_enable(new_state);
  
  if (old_state) {
    lua_pushnumber(old_state);
  } else {
    lua_pushnil();
  }
}

void mdcl_restoreText(void) {
  mdc_restore_text();
}

void mdcl_signalFailure(void) { /* void */
  /* alert the error reporting code so it can report the failure via something */
  signalFailure();
  
}


void mdcl_logError(void) {
  ERR_EVENT err_event = luaL_check_number(1);
  DEBUG_LVL debug_level = luaL_check_number(2);
  char *string = luaL_check_string(3);

  lua_pushnumber(logError(err_event, debug_level, "%s", string));
}


/* 
 * void mdc_register_luafns(void)
 *
 */

void mdc_register_luafns() {
  // register the prompt...
  lua_pushstring("> "); lua_setglobal("_PROMPT");

   mdcl_register_fns(luafn_list);
   mdcl_register_strs(luastr_list);
};

void mdcl_register_fns(struct luafn_reg_struct *walker) {
  // register functions
  while (walker && walker->name) {
    lua_register(walker->name,walker->fn);
    walker++;
  }
}

void mdcl_register_strs(struct luastr_reg_struct *walker2) {
  // register strings
  while(walker2 && walker2->name) {
    lua_pushstring(*(walker2->value));
    lua_setglobal(walker2->name);
    walker2++; 
  }
}

void mdcl_register_dbls(struct luadbl_reg_struct *walker) {
  // register doubles
  while(walker && walker->name) {
    lua_pushnumber((walker->value));
    lua_setglobal(walker->name);
    walker++; 
  }
}



void mdc_l_push_fxbool(FxBool value) {
  if (value == FXTRUE) {
    lua_pushnumber(1);
  } else {
    lua_pushnil();
  }
}



void mdcl_init_default_board(void) {
  if ((card = mdc_init_default_board()) != NULL) {
    lua_pushnumber(1);
  }
  return;
}


void mdcl_printPciInfo(void) {
  printPciInfo();
}
void mdcl_printBoardInitFns(void) {
  mdc_print_boardinitfns();
}

void mdc_l_check_card(LPCARDINFO cd) {
  if (cd == NULL) {
    lua_error("Invalid card!");
    /* return to lua */
  }
}


void mdcl_kbhit(void) {
  lua_pushnumber(kbhit());
  return;
}

void mdcl_getch(void) {
  lua_pushnumber(getch());
  return;
}

void mdcl_flush(void) {
  fflush(stdout);
}

void mdcl_fbwr8(void) {
  FxU32 loc;
  FxU8 color;
  lua_Object locObj,cObj;
  FxU8 *fbAddr;

  mdc_l_check_card(card);

  locObj = lua_getparam(1);
  cObj = lua_getparam(2);

  if (lua_isnumber(locObj) && lua_isnumber(cObj)) {
    loc = lua_getnumber(locObj);
    color = lua_getnumber(cObj);

    if (loc < card->NatMem1.MappedSize) {
      fbAddr = (FxU8 *)(card->NatMem1.MappedAddr + loc);
      
      *fbAddr = color;
    } else {
      lua_error("location out of range");
    }
    
  } else {
    lua_error("incorrect arguments to mdcl_fbwr");
  }
}

void mdcl_fbwr16(void) {
  FxU32 loc;
  FxU16 color;
  lua_Object locObj,cObj;
  FxU16 *fbAddr;
  char s[100];

  mdc_l_check_card(card);


  locObj = lua_getparam(1);
  cObj = lua_getparam(2);

  if (lua_isnumber(locObj) && lua_isnumber(cObj)) {
    loc = lua_getnumber(locObj);
    color = lua_getnumber(cObj);

    if (loc < card->NatMem1.MappedSize) {
      fbAddr = (FxU16 *)(card->NatMem1.MappedAddr + loc);
      
      *fbAddr = color;
    } else {
      sprintf(s,"location out of range, card->Natmem1.MappedSize = %lu",card->NatMem1.MappedSize);
      lua_error(s);
    }

  } else {
    lua_error("incorrect arguments to mdcl_fbwr");
  }
}

void mdcl_fbwr32(void) {
  FxU32 loc;
  FxU32 color;
  lua_Object locObj,cObj;
  FxU32 *fbAddr;

  mdc_l_check_card(card);


  locObj = lua_getparam(1);
  cObj = lua_getparam(2);

  if (lua_isnumber(locObj) && lua_isnumber(cObj)) {
    loc = lua_getnumber(locObj);
    color = lua_getnumber(cObj);

    if (loc < card->NatMem1.MappedSize) {
      fbAddr = (FxU32 *)(card->NatMem1.MappedAddr + loc);
      
      *fbAddr = color;
    } else {
      lua_error("location out of range");
    }
    
  } else {
    lua_error("incorrect arguments to mdcl_fbwr");
  }
}



void mdcl_fbrd8(void) {
  FxU32 loc;
  lua_Object locObj;
  FxU8 *fbAddr;

  mdc_l_check_card(card);

  locObj = lua_getparam(1);

  if (lua_isnumber(locObj)) {
    loc = lua_getnumber(locObj);

    if (loc < card->NatMem1.MappedSize) {
      fbAddr = (FxU8 *)(card->NatMem1.MappedAddr + loc);

      lua_pushnumber(*fbAddr);
      return;
    } else {
      lua_error("location out of range");
    }
    
  } else {
    lua_error("incorrect arguments to mdcl_fbwr");
  }
}

void mdcl_fbrd16(void) {
  FxU32 loc;
  lua_Object locObj;
  FxU16 *fbAddr;

  mdc_l_check_card(card);

  locObj = lua_getparam(1);

  if (lua_isnumber(locObj)) {
    loc = lua_getnumber(locObj);

    if (loc < card->NatMem1.MappedSize) {
      fbAddr = (FxU16 *)(card->NatMem1.MappedAddr + loc);
      
      lua_pushnumber(*fbAddr);
      return;
    } else {
      lua_error("location out of range");
    }
    
  } else {
    lua_error("incorrect arguments to mdcl_fbwr");
  }
}

void mdcl_fbrd32(void) {
  FxU32 loc;
  lua_Object locObj;
  FxU32 *fbAddr;

  mdc_l_check_card(card);

  locObj = lua_getparam(1);

  if (lua_isnumber(locObj)) {
    loc = lua_getnumber(locObj);

    if (loc < card->NatMem1.MappedSize) {
      fbAddr = (FxU32 *)(card->NatMem1.MappedAddr + loc);
      
      lua_pushnumber(*fbAddr);
      return;
    } else {
      lua_error("location out of range");
    }
      
  } else {
    lua_error("incorrect arguments to mdcl_fbwr");
  }
}


FxBool mdc_get_yesno(char *prompt) {
  printf("%s",prompt);
  fflush(stdout);

  while (1) {
    switch (toupper(getch())) {
    case 'Y':
      printf("YES\n");
      return FXTRUE;
    case 'N':
      printf("NO\n");
      return FXFALSE;
    }
  }
}



#define LUA_BUF_SIZE  2048

void mdcl_command_prompt(void) {
  int cont = 1;
  int prompt = 1;

  while (cont) {
    char buffer[LUA_BUF_SIZE];
    int i = 0;
    lua_beginblock();
    if (prompt) {
      printf("%s",lua_getstring(lua_getglobal("_PROMPT")));
    }
    for (;;) {
      int c = getchar();
      if (c == EOF) {
	cont = 0;
	break;
      } else if (c == '\n') {
	if (i>0 && buffer[i-1] == '\\') {
	  buffer[i-1] = '\n';
	} else {
	  break;
	} 


      } else if (i >= LUA_BUF_SIZE-1) {
	fprintf(stderr, "lua: argument line too long\n");
	break;
      } else {
	buffer[i++] = c;
      }
    }
    buffer[i] = 0;
    if (!strcmp(buffer,"q")) {
      printf("quit...");
      cont = 0;
    } else {
      lua_dostring(buffer);
      lua_endblock();
    }
  }
  printf("\n");
}

/*
 * void mdcl_pci_dev_info(void)
 *
 * This function takes an integer as its argument (the pci device number) and returns
 * either nil (if no such device exists) or a table containing the device info.
 */
 
void mdcl_pci_dev_info(void) {
  int dev_num;
  lua_Object arg1 = lua_getparam(1);
  
  if (!lua_isnumber(arg1)) {
    lua_error("mdcl_pci_dev_info(): arg1 should be a number");
    return; 
  }
  
  dev_num = lua_getnumber(arg1);

  if ((dev_num > MAX_PCI_DEVICES) || !pciDeviceExists(dev_num)) {
    lua_pushnil(); /* return nil */
    return;
  } 

  { /* query the PCI config space for the data and build a lua table */
    FxU32 pciData = 0;
    lua_Object ret_table;
    struct pci_field_cnv_struct {
      const PciRegister *reg;
      const char *name;
    } device_data_info[] = {
      { &PCI_DEVICE_ID, "deviceID" },
      { &PCI_VENDOR_ID, "vendorID" },
      { &PCI_BASE_ADDRESS_0, "baseAddress0"},
      { &PCI_BASE_ADDRESS_1, "baseAddress1"},
      { &PCI_COMMAND, "pciCommand"},
      { &PCI_CLASS_CODE, "pciClassCode"},
      { 0, 0}}; /* check the NAME! not the loc */
    struct pci_field_cnv_struct *walker = device_data_info;

    ret_table = lua_createtable();

    while (walker->name) {
      pciData = 0;
      pciGetConfigData( *(walker->reg), dev_num, &pciData );
      lua_pushobject(ret_table); /* table */
      lua_pushstring(walker->name); /* index */
      lua_pushnumber((double)pciData); /* data */
      lua_settable();
      walker++;
    }


    lua_pushobject(ret_table); /* return the table we created */
    return;
  }
  
}

