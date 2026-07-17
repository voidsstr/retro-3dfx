
#include "pcibrd.h"


struct luastr_reg_struct {
  char *name;
  char **value;
};

struct luadbl_reg_struct {
  char *name;
  double value;
};

struct luafn_reg_struct {
  char *name;
  void (*fn)(void);
};

void mdc_register_luafns(void);
void mdc_l_check_card(LPCARDINFO cd);
int mdc_set_output_enable(int);

extern lua_Object lua_callback_fn;
extern LPCARDINFO card;

void mdcl_register_fns(struct luafn_reg_struct *walker);
void mdcl_register_strs(struct luastr_reg_struct *walker2);
void mdcl_register_dbls(struct luadbl_reg_struct *walker);

void mdcl_chksumCallback(int frame);

void mdc_l_push_fxbool(FxBool);
