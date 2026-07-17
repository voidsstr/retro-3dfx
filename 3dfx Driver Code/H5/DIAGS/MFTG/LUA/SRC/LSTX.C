
/*  A Bison parser, made from lua.stx with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse luaY_parse
#define yylex luaY_lex
#define yyerror luaY_error
#define yylval luaY_lval
#define yychar luaY_char
#define yydebug luaY_debug
#define yynerrs luaY_nerrs
#define	WRONGTOKEN	258
#define	NIL	259
#define	IF	260
#define	THEN	261
#define	ELSE	262
#define	ELSEIF	263
#define	WHILE	264
#define	DO	265
#define	REPEAT	266
#define	UNTIL	267
#define	END	268
#define	RETURN	269
#define	LOCAL	270
#define	FUNCTION	271
#define	DOTS	272
#define	NUMBER	273
#define	NAME	274
#define	STRING	275
#define	AND	276
#define	OR	277
#define	EQ	278
#define	NE	279
#define	LE	280
#define	GE	281
#define	CONC	282
#define	UNARY	283
#define	NOT	284

#line 1 "lua.stx"

/*
** $Id: lstx.c,v 1.1 1998/01/14 21:20:23 lhf Exp $
** Syntax analizer and code generator
** See Copyright Notice in lua.h
*/


#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "ldo.h"
#include "lfunc.h"
#include "llex.h"
#include "lmem.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "lua.h"
#include "luadebug.h"
#include "lzio.h"


int luaY_parse (void);


#define MES_LIM(x)	"(limit=" x ")"


/* size of a "normal" jump instruction: OpCode + 1 byte */
#define JMPSIZE	2

/* maximum number of local variables */
#define MAXLOCALS 32
#define SMAXLOCALS "32"

#define MINGLOBAL (MAXLOCALS+1)

/* maximum number of variables in a multiple assignment */
#define MAXVAR 32
#define SMAXVAR "32"

/* maximum number of nested functions */
#define MAXSTATES  6
#define SMAXSTATES  "6"

/* maximum number of upvalues */
#define MAXUPVALUES 16
#define SMAXUPVALUES "16"



/*
** Variable descriptor:
** if 0<n<MINGLOBAL, represents local variable indexed by (n-1);
** if MINGLOBAL<=n, represents global variable at position (n-MINGLOBAL);
** if n<0, indexed variable with index (-n)-1 (table on top of stack);
** if n==0, an indexed variable (table and index on top of stack)
** Must be long to store negative Word values.
*/
typedef long vardesc;

#define isglobal(v)	(MINGLOBAL<=(v))
#define globalindex(v)	((v)-MINGLOBAL)
#define islocal(v)	(0<(v) && (v)<MINGLOBAL)
#define localindex(v)	((v)-1)
#define isdot(v)	(v<0)
#define dotindex(v)	((-(v))-1)

/* state needed to generate code for a given function */
typedef struct FuncState {
  TProtoFunc *f;  /* current function header */
  int pc;  /* next position to code */
  TaggedString *localvar[MAXLOCALS];  /* store local variable names */
  int stacksize;  /* number of values on activation register */
  int maxstacksize;  /* maximum number of values on activation register */
  int nlocalvar;  /* number of active local variables */
  int nupvalues;  /* number of upvalues */
  int nvars;  /* number of entries in f->locvars */
  int maxcode;  /* size of f->code */
  int maxvars;  /* size of f->locvars (-1 if no debug information) */
  int maxconsts;  /* size of f->consts */
  vardesc varbuffer[MAXVAR];  /* variables in an assignment list */
  vardesc upvalues[MAXUPVALUES];  /* upvalues */
} FuncState;



#define YYPURE	1


void luaY_syntaxerror (char *s, char *token)
{
  if (token[0] == 0)
    token = "<eof>";
  luaL_verror("%.100s;\n  last token read: \"%.50s\" at line %d in file %.50s",
           s, token, L->lexstate->linenumber, L->mainState->f->fileName->str);
}


void luaY_error (char *s)
{
  luaY_syntaxerror(s, luaX_lasttoken());
}


static void check_pc (int n)
{
  FuncState *fs = L->currState;
  if (fs->pc+n > fs->maxcode)
    fs->maxcode = luaM_growvector(&fs->f->code, fs->maxcode,
                                  Byte, codeEM, MAX_INT);
}


static void code_byte (Byte c)
{
  check_pc(1);
  L->currState->f->code[L->currState->pc++] = c;
}


static void deltastack (int delta)
{
  FuncState *fs = L->currState;
  fs->stacksize += delta;
  if (fs->stacksize > fs->maxstacksize) {
    if (fs->stacksize > 255)
      luaY_error("function/expression too complex");
    fs->maxstacksize = fs->stacksize;
  }
}


static int code_oparg_at (int pc, OpCode op, int builtin, int arg, int delta)
{
  Byte *code = L->currState->f->code;
  deltastack(delta);
  if (arg < builtin) {
    code[pc] = op+1+arg;
    return 1;
  }
  else if (arg <= 255) {
    code[pc] = op;
    code[pc+1] = arg;
    return 2;
  }
  else if (arg <= MAX_WORD) {
    code[pc] = op+1+builtin;
    code[pc+1] = arg&0xFF;
    code[pc+2] = arg>>8;
    return 3;
  }
  else luaY_error("code too long " MES_LIM("64K"));
  return 0;   /* to avoid warnings */
}


static int fix_opcode (int pc, OpCode op, int builtin, int arg)
{
  FuncState *fs = L->currState;
  if (arg < builtin) {  /* close space */
    luaO_memdown(fs->f->code+pc+1, fs->f->code+pc+2, fs->pc-(pc+2));
    fs->pc--;
  }
  else if (arg > 255) {  /* open space */
    check_pc(1);
    luaO_memup(fs->f->code+pc+1, fs->f->code+pc, fs->pc-pc);
    fs->pc++;
  }
  return code_oparg_at(pc, op, builtin, arg, 0) - 2;
}


static void code_oparg (OpCode op, int builtin, int arg, int delta)
{
  check_pc(3);  /* maximum code size */
  L->currState->pc += code_oparg_at(L->currState->pc, op, builtin, arg, delta);
}


static void code_opcode (OpCode op, int delta)
{
  deltastack(delta);
  code_byte(op);
}


static void code_pop (OpCode op)
{
  code_opcode(op, -1);
}

/* binary operations get 2 arguments and leave one, so they pop one */
#define code_binop(op)	code_pop(op)


static void code_neutralop (OpCode op)
{
  code_opcode(op, 0);
}

/* unary operations get 1 argument and leave one, so they are neutral */
#define code_unop(op)	code_neutralop(op)


static void code_constant (int c)
{
  code_oparg(PUSHCONSTANT, 8, c, 1);
}


static int next_constant (FuncState *cs)
{
  TProtoFunc *f = cs->f;
  if (f->nconsts >= cs->maxconsts) {
    cs->maxconsts = luaM_growvector(&f->consts, cs->maxconsts, TObject,
                                    constantEM, MAX_WORD);
  }
  return f->nconsts++;
}


static int string_constant (TaggedString *s, FuncState *cs)
{
  TProtoFunc *f = cs->f;
  int c = s->constindex;
  if (!(c < f->nconsts &&
      ttype(&f->consts[c]) == LUA_T_STRING && tsvalue(&f->consts[c]) == s)) {
    c = next_constant(cs);
    ttype(&f->consts[c]) = LUA_T_STRING;
    tsvalue(&f->consts[c]) = s;
    s->constindex = c;  /* hint for next time */
  }
  return c;
}


static void code_string (TaggedString *s)
{
  code_constant(string_constant(s, L->currState));
}


#define LIM 20
static int real_constant (real r)
{
  /* check whether 'r' has appeared within the last LIM entries */
  TObject *cnt = L->currState->f->consts;
  int c = L->currState->f->nconsts;
  int lim = c < LIM ? 0 : c-LIM;
  while (--c >= lim) {
    if (ttype(&cnt[c]) == LUA_T_NUMBER && nvalue(&cnt[c]) == r)
      return c;
  }
  /* not found; create a luaM_new entry */
  c = next_constant(L->currState);
  cnt = L->currState->f->consts;  /* 'next_constant' may reallocate this vector */
  ttype(&cnt[c]) = LUA_T_NUMBER;
  nvalue(&cnt[c]) = r;
  return c;
}


static void code_number (real f)
{
  int i;
  if (f >= 0 && f <= (real)MAX_WORD && (real)(i=(int)f) == f)
    code_oparg(PUSHNUMBER, 3, i, 1);  /* f has an (short) integer value */
  else
    code_constant(real_constant(f));
}


static void flush_record (int n)
{
  if (n > 0)
    code_oparg(SETMAP, 1, n-1, -2*n);
}

static void flush_list (int m, int n)
{
  if (n == 0) return;
  code_oparg(SETLIST, 1, m, -n);
  code_byte(n);
}


static void luaI_registerlocalvar (TaggedString *varname, int line)
{
  FuncState *fs = L->currState;
  if (fs->maxvars != -1) {  /* debug information? */
    if (fs->nvars >= fs->maxvars)
      fs->maxvars = luaM_growvector(&fs->f->locvars, fs->maxvars,
                                    LocVar, "", MAX_WORD);
    fs->f->locvars[fs->nvars].varname = varname;
    fs->f->locvars[fs->nvars].line = line;
    fs->nvars++;
  }
}


static void luaI_unregisterlocalvar (int line)
{
  luaI_registerlocalvar(NULL, line);
}


static void store_localvar (TaggedString *name, int n)
{
  if (L->currState->nlocalvar+n < MAXLOCALS)
    L->currState->localvar[L->currState->nlocalvar+n] = name;
  else
    luaY_error("too many local variables " MES_LIM(SMAXLOCALS));
  luaI_registerlocalvar(name, L->lexstate->linenumber);
}

static void add_localvar (TaggedString *name)
{
  store_localvar(name, 0);
  L->currState->nlocalvar++;
}


/* 
** dotted variables <a.x> must be stored like regular indexed vars <a["x"]>
*/
static vardesc var2store (vardesc var)
{
  if (isdot(var)) {
    code_constant(dotindex(var));
    var = 0;
  }
  return var;
}


static void add_varbuffer (vardesc var, int n)
{
  if (n >= MAXVAR)
    luaY_error("variable buffer overflow " MES_LIM(SMAXVAR));
  L->currState->varbuffer[n] = var2store(var);
}


static int aux_localname (TaggedString *n, FuncState *st)
{
  int i;
  for (i=st->nlocalvar-1; i >= 0; i--)
    if (n == st->localvar[i]) return i;  /* local var index */
  return -1;  /* not found */
}


static vardesc singlevar (TaggedString *n, FuncState *st)
{
  int i = aux_localname(n, st);
  if (i == -1) {  /* check shadowing */
    int l;
    for (l=1; l<=(st-L->mainState); l++)
      if (aux_localname(n, st-l) >= 0)
        luaY_syntaxerror("cannot access a variable in outer scope", n->str);
    return string_constant(n, st)+MINGLOBAL;  /* global value */
  }
  else return i+1;  /* local value */
}


static int indexupvalue (TaggedString *n)
{
  vardesc v = singlevar(n, L->currState-1);
  int i;
  for (i=0; i<L->currState->nupvalues; i++) {
    if (L->currState->upvalues[i] == v)
      return i;
  }
  /* new one */
  if (++(L->currState->nupvalues) > MAXUPVALUES)
    luaY_error("too many upvalues in a single function " MES_LIM(SMAXUPVALUES));
  L->currState->upvalues[i] = v;  /* i = L->currState->nupvalues - 1 */
  return i;
}


static void pushupvalue (TaggedString *n)
{
  int i;
  if (L->currState == L->mainState)
    luaY_error("cannot access upvalue in main");
  if (aux_localname(n, L->currState) >= 0)
    luaY_syntaxerror("cannot access an upvalue in current scope", n->str);
  i = indexupvalue(n);
  code_oparg(PUSHUPVALUE, 2, i, 1);
}


void luaY_codedebugline (int line)
{
  if (lua_debug && line != L->lexstate->lastline) {
    code_oparg(SETLINE, 0, line, 0);
    L->lexstate->lastline = line;
  }
}


static void adjuststack (int n)
{
  if (n > 0)
    code_oparg(POP, 2, n-1, -n);
  else if (n < 0)
    code_oparg(PUSHNIL, 1, (-n)-1, -n);
}


static long adjust_functioncall (long exp, int nresults)
{
  if (exp <= 0)
    return -exp; /* exp is -list length */
  else {
    int temp = L->currState->f->code[exp];
    int nparams = L->currState->f->code[exp-1];
    exp += fix_opcode(exp-2, CALLFUNC, 2, nresults);
    L->currState->f->code[exp] = nparams;
    if (nresults != MULT_RET)
      deltastack(nresults);
    deltastack(-(nparams+1));
    return temp+nresults;
  }
}


static void adjust_mult_assign (int vars, long exps)
{
  if (exps > 0) { /* must correct function call */
    int diff = L->currState->f->code[exps] - vars;
    if (diff < 0)
      adjust_functioncall(exps, -diff);
    else {
      adjust_functioncall(exps, 0);
      adjuststack(diff);
    }
  }
  else adjuststack((-exps)-vars);
}


static void code_args (int nparams, int dots)
{
  L->currState->nlocalvar += nparams;  /* "self" may already be there */
  nparams = L->currState->nlocalvar;
  if (!dots) {
    L->currState->f->code[1] = nparams;  /* fill-in arg information */
    deltastack(nparams);
  }
  else {
    L->currState->f->code[1] = nparams+ZEROVARARG;
    deltastack(nparams+1);
    add_localvar(luaS_new("arg"));
  }
}


static void lua_pushvar (vardesc var)
{
  if (isglobal(var))
    code_oparg(GETGLOBAL, 8, globalindex(var), 1);
  else if (islocal(var))
    code_oparg(PUSHLOCAL, 8, localindex(var), 1);
  else if (isdot(var))
    code_oparg(GETDOTTED, 8, dotindex(var), 0);
  else
    code_pop(GETTABLE);
}


static void storevar (vardesc var)
{
  if (var == 0)  /* indexed var */
    code_opcode(SETTABLE0, -3);
  else if (isglobal(var))
    code_oparg(SETGLOBAL, 8, globalindex(var), -1);
  else  /* local var */
    code_oparg(SETLOCAL, 8, localindex(var), -1);
}


/* returns how many elements are left as 'garbage' on the stack */
static int lua_codestore (int i, int left)
{
  if (L->currState->varbuffer[i] != 0 ||  /* global or local var or */
      left+i == 0) {  /* indexed var without values in between */
    storevar(L->currState->varbuffer[i]);
    return left;
  }
  else {  /* indexed var with values in between*/
    code_oparg(SETTABLE, 0, left+i, -1);
    return left+2;  /* table/index are not popped, since they are not on top */
  }
}


static int fix_jump (int pc, OpCode op, int n)
{
  /* jump is relative to position following jump instruction */
  return fix_opcode(pc, op, 0, n-(pc+JMPSIZE));
}


static void fix_upjmp (OpCode op, int pos)
{
  int delta = L->currState->pc+JMPSIZE - pos;  /* jump is relative */
  if (delta > 255) delta++;
  code_oparg(op, 0, delta, 0);
}


static void codeIf (int thenAdd, int elseAdd)
{
  int elseinit = elseAdd+JMPSIZE;
  if (L->currState->pc == elseinit) {  /* no else part */
    L->currState->pc -= JMPSIZE;
    elseinit = L->currState->pc;
  }
  else
    elseinit += fix_jump(elseAdd, JMP, L->currState->pc);
  fix_jump(thenAdd, IFFJMP, elseinit);
}


static void code_shortcircuit (int pc, OpCode op)
{
  fix_jump(pc, op, L->currState->pc);
}


static void codereturn (void)
{
  code_oparg(RETCODE, 0, L->currState->nlocalvar, 0);
  L->currState->stacksize = L->currState->nlocalvar;
}


static void func_onstack (TProtoFunc *f)
{
  int i;
  int nupvalues = (L->currState+1)->nupvalues;
  int c = next_constant(L->currState);
  ttype(&L->currState->f->consts[c]) = LUA_T_PROTO;
  L->currState->f->consts[c].value.tf = (L->currState+1)->f;
  if (nupvalues == 0)
    code_constant(c);
  else {
    for (i=0; i<nupvalues; i++)
      lua_pushvar((L->currState+1)->upvalues[i]);
    code_constant(c);
    code_oparg(CLOSURE, 2, nupvalues, -nupvalues);
  }
}


static void init_state (TaggedString *filename)
{
  TProtoFunc *f = luaF_newproto();
  FuncState *fs = L->currState;
  fs->stacksize = 0;
  fs->maxstacksize = 0;
  fs->nlocalvar = 0;
  fs->nupvalues = 0;
  fs->f = f;
  f->fileName = filename;
  fs->pc = 0;
  fs->maxcode = 0;
  f->code = NULL;
  fs->maxconsts = 0;
  if (lua_debug) {
    fs->nvars = 0;
    fs->maxvars = 0;
  }
  else
    fs->maxvars = -1;  /* flag no debug information */
  code_byte(0);  /* to be filled with stacksize */
  code_byte(0);  /* to be filled with arg information */
  L->lexstate->lastline = 0;  /* invalidate it */
}


static void init_func (void)
{
  if (L->currState-L->mainState >= MAXSTATES-1)
    luaY_error("too many nested functions " MES_LIM(SMAXSTATES));
  L->currState++;
  init_state(L->mainState->f->fileName);
  luaY_codedebugline(L->lexstate->linenumber);
  L->currState->f->lineDefined = L->lexstate->linenumber;
}

static TProtoFunc *close_func (void)
{
  TProtoFunc *f = L->currState->f;
  code_neutralop(ENDCODE);
  f->code[0] = L->currState->maxstacksize;
  f->code = luaM_reallocvector(f->code, L->currState->pc, Byte);
  f->consts = luaM_reallocvector(f->consts, f->nconsts, TObject);
  if (L->currState->maxvars != -1) {  /* debug information? */
    luaI_registerlocalvar(NULL, -1);  /* flag end of vector */
    f->locvars = luaM_reallocvector(f->locvars, L->currState->nvars, LocVar);
  }
  L->currState--;
  return f;
}


/*
** Parse Lua code.
*/
TProtoFunc *luaY_parser (ZIO *z)
{
  struct LexState lexstate;
  FuncState state[MAXSTATES];
  L->currState = L->mainState = &state[0];
  L->lexstate = &lexstate;
  luaX_setinput(z);
  init_state(luaS_new(zname(z)));
  if (luaY_parse()) lua_error("parse error");
  return close_func();
}



#line 634 "lua.stx"
typedef union
{
  int vInt;
  real vReal;
  char *pChar;
  long vLong;
  TaggedString *pTStr;
  TProtoFunc *pFunc;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		181
#define	YYFLAG		-32768
#define	YYNTBASE	49

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 93)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,    43,     2,     2,    41,
    42,    32,    30,    48,    31,    39,    33,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    40,    37,    26,
    38,    25,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    44,     2,    45,    36,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    46,     2,    47,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    27,
    28,    29,    34,    35
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     4,     8,     9,    11,    19,    23,    30,    36,
    40,    42,    46,    50,    51,    54,    56,    60,    64,    66,
    68,    74,    75,    78,    85,    86,    90,    91,    92,    94,
    96,    99,   101,   105,   109,   113,   117,   121,   125,   129,
   133,   137,   141,   145,   149,   153,   156,   159,   161,   163,
   165,   167,   169,   170,   174,   179,   184,   186,   188,   191,
   193,   195,   200,   204,   206,   211,   214,   216,   220,   224,
   226,   228,   229,   231,   233,   234,   239,   240,   242,   244,
   248,   250,   254,   255,   257,   259,   260,   262,   265,   268,
   270,   274,   278,   282,   284,   286,   290,   292,   296,   298,
   302,   303
};

static const short yyrhs[] = {    50,
    60,     0,     0,    50,    52,    51,     0,     0,    37,     0,
     5,    65,     6,    53,    62,    59,    13,     0,    10,    53,
    13,     0,     9,    61,    65,    10,    53,    13,     0,    11,
    61,    53,    12,    66,     0,    90,    38,    78,     0,    74,
     0,    15,    91,    92,     0,    16,    55,    58,     0,     0,
    54,    49,     0,    72,     0,    56,    39,    57,     0,    56,
    40,    57,     0,    72,     0,    19,     0,    41,    80,    42,
    49,    13,     0,     0,     7,    53,     0,     8,    65,     6,
    53,    62,    59,     0,     0,    14,    77,    51,     0,     0,
     0,    62,     0,    62,     0,    66,    63,     0,    67,     0,
    41,    67,    42,     0,    66,    23,    66,     0,    66,    26,
    66,     0,    66,    25,    66,     0,    66,    24,    66,     0,
    66,    27,    66,     0,    66,    28,    66,     0,    66,    30,
    66,     0,    66,    31,    66,     0,    66,    32,    66,     0,
    66,    33,    66,     0,    66,    36,    66,     0,    66,    29,
    66,     0,    31,    66,     0,    35,    66,     0,    70,     0,
    73,     0,    18,     0,    20,     0,     4,     0,     0,    16,
    68,    58,     0,    66,    21,    63,    66,     0,    66,    22,
    63,    66,     0,    70,     0,    71,     0,    43,    19,     0,
    74,     0,    72,     0,    69,    44,    66,    45,     0,    69,
    39,    19,     0,    19,     0,    46,    64,    81,    47,     0,
    75,    76,     0,    69,     0,    69,    40,    19,     0,    41,
    77,    42,     0,    73,     0,    20,     0,     0,    78,     0,
    67,     0,     0,    78,    48,    79,    67,     0,     0,    17,
     0,    91,     0,    91,    48,    17,     0,    82,     0,    82,
    37,    82,     0,     0,    84,     0,    85,     0,     0,    48,
     0,    86,    83,     0,    89,    83,     0,    87,     0,    86,
    48,    87,     0,    88,    38,    66,     0,    44,    66,    45,
     0,    57,     0,    66,     0,    89,    48,    66,     0,    71,
     0,    90,    48,    71,     0,    19,     0,    91,    48,    19,
     0,     0,    38,    78,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   679,   681,   682,   685,   685,   687,   689,   691,   703,   709,
   719,   721,   727,   730,   731,   738,   739,   744,   752,   754,
   756,   758,   759,   760,   763,   764,   771,   773,   780,   782,
   784,   786,   788,   789,   790,   791,   792,   793,   794,   795,
   796,   797,   798,   799,   800,   801,   802,   803,   804,   805,
   806,   807,   808,   808,   809,   810,   813,   815,   816,   817,
   820,   821,   822,   825,   827,   829,   838,   839,   846,   847,
   848,   851,   852,   855,   856,   857,   866,   867,   868,   869,
   872,   873,   881,   882,   883,   886,   886,   888,   895,   902,
   903,   911,   913,   914,   917,   918,   926,   927,   930,   931,
   934,   935
};

static const char * const yytname[] = {   "$","error","$undefined.","WRONGTOKEN",
"NIL","IF","THEN","ELSE","ELSEIF","WHILE","DO","REPEAT","UNTIL","END","RETURN",
"LOCAL","FUNCTION","DOTS","NUMBER","NAME","STRING","AND","OR","EQ","NE","'>'",
"'<'","LE","GE","CONC","'+'","'-'","'*'","'/'","UNARY","NOT","'^'","';'","'='",
"'.'","':'","'('","')'","'%'","'['","']'","'{'","'}'","','","chunk","statlist",
"sc","stat","block","@1","funcname","fvarname","fname","body","elsepart","ret",
"GetPC","SaveWord","SaveWordPop","SaveWordPush","cond","expr1","expr","@2","sexp1",
"sexp","var","varname","table","functioncall","funcvalue","funcParams","exprlist",
"exprlist1","@3","parlist","fieldlist","part","lastcomma","ffieldlist","lfieldlist",
"ffieldlist1","ffield","ffieldkey","lfieldlist1","varlist1","localnamelist",
"decinit",""
};
#endif

static const short yyr1[] = {     0,
    49,    50,    50,    51,    51,    52,    52,    52,    52,    52,
    52,    52,    52,    54,    53,    55,    55,    55,    56,    57,
    58,    59,    59,    59,    60,    60,    61,    62,    63,    64,
    65,    66,    67,    67,    67,    67,    67,    67,    67,    67,
    67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
    67,    67,    68,    67,    67,    67,    69,    70,    70,    70,
    71,    71,    71,    72,    73,    74,    75,    75,    76,    76,
    76,    77,    77,    78,    79,    78,    80,    80,    80,    80,
    81,    81,    82,    82,    82,    83,    83,    84,    85,    86,
    86,    87,    88,    88,    89,    89,    90,    90,    91,    91,
    92,    92
};

static const short yyr2[] = {     0,
     2,     0,     3,     0,     1,     7,     3,     6,     5,     3,
     1,     3,     3,     0,     2,     1,     3,     3,     1,     1,
     5,     0,     2,     6,     0,     3,     0,     0,     1,     1,
     2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     2,     2,     1,     1,     1,
     1,     1,     0,     3,     4,     4,     1,     1,     2,     1,
     1,     4,     3,     1,     4,     2,     1,     3,     3,     1,
     1,     0,     1,     1,     0,     4,     0,     1,     1,     3,
     1,     3,     0,     1,     1,     0,     1,     2,     2,     1,
     3,     3,     3,     1,     1,     3,     1,     3,     1,     3,
     0,     2
};

static const short yydefact[] = {     2,
    25,     0,    27,    14,    27,    72,     0,     0,    64,     0,
     4,     1,    67,    57,    58,    61,    11,     0,     0,    52,
    53,    50,    51,     0,     0,     0,    28,     0,    28,    32,
    48,    58,    49,    60,     0,     0,     2,    14,     0,    74,
     4,    73,    99,   101,     0,     0,    19,    59,     5,     3,
     0,     0,     0,    71,    72,    70,    66,     0,     0,     0,
    46,    47,    32,    30,    83,    14,    28,    28,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    29,    31,     0,     7,    15,     0,    26,    75,     0,     0,
    12,    77,    13,     0,     0,    63,    68,     0,     0,    10,
    58,    54,    33,    64,     0,    94,    95,     0,    81,    84,
    85,    86,    90,     0,    86,    28,     0,     0,    34,    37,
    36,    35,    38,    39,    45,    40,    41,    42,    43,    44,
    14,     0,     0,   102,   100,    78,     0,    79,    20,    17,
    18,    62,    69,     0,    65,    83,    87,    88,     0,    87,
    89,    22,    55,    56,     0,     9,    76,     2,     0,    93,
    82,    91,    92,    96,    14,     0,     0,     8,     0,    80,
    23,     0,     6,    21,    14,    28,    22,    24,     0,     0,
     0
};

static const short yydefgoto[] = {    85,
     1,    50,    11,    36,    37,    45,    46,   106,    93,   167,
    12,    35,    81,    82,    65,    28,    39,    30,    60,    13,
    31,    32,    16,    33,    34,    18,    57,    41,    42,   133,
   137,   108,   109,   148,   110,   111,   112,   113,   114,   115,
    19,    44,    91
};

static const short yypact[] = {-32768,
   147,    94,-32768,-32768,-32768,    94,    19,    34,-32768,    39,
   -19,-32768,     2,-32768,   -29,-32768,   -10,    -6,   -27,-32768,
-32768,-32768,-32768,    94,    94,    94,-32768,    14,   196,-32768,
     4,-32768,-32768,-32768,    94,    49,-32768,-32768,   196,   212,
   -19,    18,-32768,   -22,    38,    21,    40,-32768,-32768,-32768,
    68,    71,    94,-32768,    94,-32768,-32768,    94,     9,    38,
    57,    57,    62,-32768,    76,-32768,-32768,-32768,    94,    94,
    94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
-32768,-32768,    96,-32768,-32768,   106,-32768,-32768,    94,   102,
-32768,    -4,-32768,   109,   109,-32768,-32768,   146,    91,    18,
   -21,-32768,-32768,    98,    94,-32768,   196,    92,   105,-32768,
-32768,    90,-32768,   107,    95,-32768,    94,    94,    53,    53,
    53,    53,    53,    53,    69,    23,    23,    57,    57,    57,
-32768,    94,    94,    18,-32768,-32768,   104,   112,-32768,-32768,
-32768,-32768,-32768,   171,-32768,    76,    -7,-32768,    94,    94,
-32768,   101,   226,   226,   136,   196,   212,-32768,    30,-32768,
-32768,-32768,   196,   196,-32768,    94,   137,-32768,   140,-32768,
-32768,   148,-32768,-32768,-32768,-32768,   101,-32768,   155,   165,
-32768
};

static const short yypgoto[] = {     1,
-32768,   139,-32768,   -34,-32768,-32768,-32768,    29,   121,     7,
-32768,   178,   -25,    59,-32768,   -32,    -2,    -1,-32768,-32768,
     5,     6,   177,   168,   186,-32768,-32768,   133,   -50,-32768,
-32768,-32768,    43,    93,-32768,-32768,-32768,    58,-32768,-32768,
-32768,   114,-32768
};


#define	YYLAST		262


static const short yytable[] = {    29,
   179,    64,    83,    86,    40,    14,    15,   100,   -97,   -60,
    58,   139,   136,    54,    43,    89,   -98,    49,   -97,    66,
    59,    61,    62,   -57,    63,    90,   -98,     9,   -60,   -60,
   -60,   116,    29,   -60,    55,   -60,   105,    43,   134,    27,
    51,    52,   -57,   -57,   -57,    53,   170,   -57,   135,   -57,
    98,    10,     9,    40,    78,    79,    40,    48,    80,    94,
    95,    84,   107,    14,   101,    88,   119,   120,   121,   122,
   123,   124,   125,   126,   127,   128,   129,   130,    92,    20,
   -16,    75,    76,    77,    78,    79,    96,    40,    80,    97,
   152,    21,    80,    22,   104,    23,   155,    20,    76,    77,
    78,    79,   144,   103,    80,   131,    24,   165,   166,    21,
    25,    22,     9,    23,   153,   154,    26,   132,    10,   105,
   135,    27,   140,   141,    24,   117,   118,   139,    25,   156,
   171,   157,   143,   172,    26,   -20,    10,   147,   145,    27,
   176,   146,   150,   107,   149,   158,   163,   164,   168,   173,
   177,     2,   174,   175,   180,     3,     4,     5,   169,   159,
     6,     7,     8,    29,   181,     9,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,    87,
   102,    80,    38,   178,    47,    56,    17,    99,   161,    10,
   142,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78,    79,   162,   138,    80,   151,     0,     0,
     0,     0,     0,     0,     0,   160,    67,    68,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,     0,
     0,    80,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
   -32,   -32,   -32,   -32,   -32,     0,     0,   -32,    69,    70,
    71,    72,    73,    74,    75,    76,    77,    78,    79,     0,
     0,    80
};

static const short yycheck[] = {     2,
     0,    27,    35,    38,     6,     1,     1,    58,    38,    20,
    38,    19,    17,    20,    19,    38,    38,    37,    48,     6,
    48,    24,    25,    20,    26,    48,    48,    19,    39,    40,
    41,    66,    35,    44,    41,    46,    44,    19,    89,    46,
    39,    40,    39,    40,    41,    44,    17,    44,    19,    46,
    53,    43,    19,    55,    32,    33,    58,    19,    36,    39,
    40,    13,    65,    59,    59,    48,    69,    70,    71,    72,
    73,    74,    75,    76,    77,    78,    79,    80,    41,     4,
    41,    29,    30,    31,    32,    33,    19,    89,    36,    19,
   116,    16,    36,    18,    19,    20,   131,     4,    30,    31,
    32,    33,   105,    42,    36,    10,    31,     7,     8,    16,
    35,    18,    19,    20,   117,   118,    41,    12,    43,    44,
    19,    46,    94,    95,    31,    67,    68,    19,    35,   132,
   165,   133,    42,   166,    41,    38,    43,    48,    47,    46,
   175,    37,    48,   146,    38,    42,   149,   150,    13,    13,
   176,     5,    13,     6,     0,     9,    10,    11,   158,    48,
    14,    15,    16,   166,     0,    19,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    41,
    60,    36,     5,   177,     8,    18,     1,    55,   146,    43,
    45,    21,    22,    23,    24,    25,    26,    27,    28,    29,
    30,    31,    32,    33,   147,    92,    36,   115,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    45,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
    -1,    36,    21,    22,    23,    24,    25,    26,    27,    28,
    29,    30,    31,    32,    33,    -1,    -1,    36,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
    -1,    36
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

/* 
** This skeleton file has been modified by Roberto Ierusalimschy to
** avoid the use of non-ANSI features (e.g. alloc, malloc.h)
*/


#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
int	yylex	(void);
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
int	yylex	(YYSTYPE *s, YYLTYPE *l, YYLEX_PARAM_TYPE *p);
#else
#define YYLEX		yylex(&yylval, &yylloc)
int	yylex	(YYSTYPE *s, YYLTYPE *l);
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
int	yylex	(YYSTYPE *s, YYLEX_PARAM_TYPE *p);
#else
#define YYLEX		yylex(&yylval)
int	yylex	(YYSTYPE *s);
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 500
#endif


#line 192 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the parameter list to be passed
   to "yyparse" (changed by RY) */

#ifndef YYPARSE_PARAM
#define YYPARSE_PARAM	void
#endif

int
yyparse(YYPARSE_PARAM)
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  YYSTYPE yylval;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  int yynerrs;			/*  number of parse errors so far       */
  int yychar;			/*  the lookahead symbol		*/
  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow

      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
	  yyerror("parser stack overflow");
	  return 2;
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 6:
#line 687 "lua.stx"
{ codeIf(yyvsp[-5].vInt, yyvsp[-2].vInt); ;
    break;}
case 8:
#line 692 "lua.stx"
{{
	 FuncState *fs = L->currState;
         int expsize = yyvsp[-3].vInt-yyvsp[-4].vInt;
	 int newpos = yyvsp[-4].vInt+JMPSIZE;
	 check_pc(expsize);
	 memcpy(fs->f->code+fs->pc, fs->f->code+yyvsp[-4].vInt, expsize);
	 luaO_memdown(fs->f->code+yyvsp[-4].vInt, fs->f->code+yyvsp[-3].vInt, fs->pc-yyvsp[-4].vInt);
	 newpos += fix_jump(yyvsp[-4].vInt, JMP, fs->pc-expsize);
	 fix_upjmp(IFTUPJMP, newpos);
       };
    break;}
case 9:
#line 704 "lua.stx"
{
	 fix_upjmp(IFFUPJMP, yyvsp[-3].vInt);
	 deltastack(-1);  /* pops condition */
       ;
    break;}
case 10:
#line 710 "lua.stx"
{{
	  int i;
          int left = 0;
	  adjust_mult_assign(yyvsp[-2].vInt, yyvsp[0].vLong);
	  for (i=yyvsp[-2].vInt-1; i>=0; i--)
	    left = lua_codestore(i, left);
          adjuststack(left);  /* remove eventual 'garbage' left on stack */
       };
    break;}
case 11:
#line 719 "lua.stx"
{ adjust_functioncall(yyvsp[0].vLong, 0); ;
    break;}
case 12:
#line 722 "lua.stx"
{
         L->currState->nlocalvar += yyvsp[-1].vInt;
         adjust_mult_assign(yyvsp[-1].vInt, yyvsp[0].vInt);
       ;
    break;}
case 13:
#line 727 "lua.stx"
{ storevar(yyvsp[-1].vLong); ;
    break;}
case 14:
#line 730 "lua.stx"
{yyval.vInt = L->currState->nlocalvar;;
    break;}
case 15:
#line 731 "lua.stx"
{
           adjuststack(L->currState->nlocalvar - yyvsp[-1].vInt);
	   for (; L->currState->nlocalvar > yyvsp[-1].vInt; L->currState->nlocalvar--)
	     luaI_unregisterlocalvar(L->lexstate->linenumber);
         ;
    break;}
case 16:
#line 738 "lua.stx"
{ yyval.vLong = yyvsp[0].vLong; init_func(); ;
    break;}
case 17:
#line 740 "lua.stx"
{
          yyval.vLong = 0;  /* flag indexed variable */
	  init_func();
	;
    break;}
case 18:
#line 745 "lua.stx"
{
          yyval.vLong = 0;  /* flag indexed variable */
	  init_func();
	  add_localvar(luaS_new("self"));
	;
    break;}
case 19:
#line 752 "lua.stx"
{ lua_pushvar(yyvsp[0].vLong); ;
    break;}
case 20:
#line 754 "lua.stx"
{ code_string(yyvsp[0].pTStr); ;
    break;}
case 21:
#line 756 "lua.stx"
{ func_onstack(close_func()); ;
    break;}
case 24:
#line 760 "lua.stx"
{ codeIf(yyvsp[-4].vInt, yyvsp[-1].vInt); ;
    break;}
case 26:
#line 765 "lua.stx"
{
	   adjust_functioncall(yyvsp[-1].vLong, MULT_RET);
           codereturn();
          ;
    break;}
case 27:
#line 771 "lua.stx"
{ yyval.vInt = L->currState->pc; ;
    break;}
case 28:
#line 774 "lua.stx"
{ yyval.vInt = L->currState->pc;
	  check_pc(JMPSIZE);
	  L->currState->pc += JMPSIZE;  /* open space */
	;
    break;}
case 29:
#line 780 "lua.stx"
{ yyval.vInt = yyvsp[0].vInt; deltastack(-1);  /* pop condition */ ;
    break;}
case 30:
#line 782 "lua.stx"
{ yyval.vInt = yyvsp[0].vInt; deltastack(1);  /* push a value */ ;
    break;}
case 31:
#line 784 "lua.stx"
{ yyval.vInt = yyvsp[0].vInt; ;
    break;}
case 32:
#line 786 "lua.stx"
{ adjust_functioncall(yyvsp[0].vLong, 1); ;
    break;}
case 33:
#line 788 "lua.stx"
{ yyval.vLong = yyvsp[-1].vLong; ;
    break;}
case 34:
#line 789 "lua.stx"
{ code_binop(EQOP);   yyval.vLong = 0; ;
    break;}
case 35:
#line 790 "lua.stx"
{ code_binop(LTOP);   yyval.vLong = 0; ;
    break;}
case 36:
#line 791 "lua.stx"
{ code_binop(GTOP);   yyval.vLong = 0; ;
    break;}
case 37:
#line 792 "lua.stx"
{ code_binop(NEQOP);  yyval.vLong = 0; ;
    break;}
case 38:
#line 793 "lua.stx"
{ code_binop(LEOP);   yyval.vLong = 0; ;
    break;}
case 39:
#line 794 "lua.stx"
{ code_binop(GEOP);   yyval.vLong = 0; ;
    break;}
case 40:
#line 795 "lua.stx"
{ code_binop(ADDOP);  yyval.vLong = 0; ;
    break;}
case 41:
#line 796 "lua.stx"
{ code_binop(SUBOP);  yyval.vLong = 0; ;
    break;}
case 42:
#line 797 "lua.stx"
{ code_binop(MULTOP); yyval.vLong = 0; ;
    break;}
case 43:
#line 798 "lua.stx"
{ code_binop(DIVOP);  yyval.vLong = 0; ;
    break;}
case 44:
#line 799 "lua.stx"
{ code_binop(POWOP);  yyval.vLong = 0; ;
    break;}
case 45:
#line 800 "lua.stx"
{ code_binop(CONCOP);  yyval.vLong = 0; ;
    break;}
case 46:
#line 801 "lua.stx"
{ code_unop(MINUSOP); yyval.vLong = 0;;
    break;}
case 47:
#line 802 "lua.stx"
{ code_unop(NOTOP);  yyval.vLong = 0;;
    break;}
case 48:
#line 803 "lua.stx"
{ yyval.vLong = yyvsp[0].vLong;  /* simple expressions */ ;
    break;}
case 49:
#line 804 "lua.stx"
{ yyval.vLong = 0; ;
    break;}
case 50:
#line 805 "lua.stx"
{ code_number(yyvsp[0].vReal); yyval.vLong = 0; ;
    break;}
case 51:
#line 806 "lua.stx"
{ code_string(yyvsp[0].pTStr); yyval.vLong = 0; ;
    break;}
case 52:
#line 807 "lua.stx"
{ adjuststack(-1); yyval.vLong = 0; ;
    break;}
case 53:
#line 808 "lua.stx"
{ init_func(); ;
    break;}
case 54:
#line 808 "lua.stx"
{ yyval.vLong = 0; ;
    break;}
case 55:
#line 809 "lua.stx"
{ code_shortcircuit(yyvsp[-1].vInt, ONFJMP); yyval.vLong = 0; ;
    break;}
case 56:
#line 810 "lua.stx"
{ code_shortcircuit(yyvsp[-1].vInt, ONTJMP); yyval.vLong = 0; ;
    break;}
case 57:
#line 813 "lua.stx"
{ adjust_functioncall(yyvsp[0].vLong, 1); ;
    break;}
case 58:
#line 815 "lua.stx"
{ lua_pushvar(yyvsp[0].vLong); yyval.vLong = 0; ;
    break;}
case 59:
#line 816 "lua.stx"
{ pushupvalue(yyvsp[0].pTStr); yyval.vLong = 0; ;
    break;}
case 60:
#line 817 "lua.stx"
{ yyval.vLong = yyvsp[0].vLong; ;
    break;}
case 61:
#line 820 "lua.stx"
{ yyval.vLong = yyvsp[0].vLong; ;
    break;}
case 62:
#line 821 "lua.stx"
{ yyval.vLong = 0; ;
    break;}
case 63:
#line 822 "lua.stx"
{ yyval.vLong = (-string_constant(yyvsp[0].pTStr, L->currState))-1; ;
    break;}
case 64:
#line 825 "lua.stx"
{ yyval.vLong = singlevar(yyvsp[0].pTStr, L->currState); ;
    break;}
case 65:
#line 827 "lua.stx"
{ fix_opcode(yyvsp[-2].vInt, CREATEARRAY, 2, yyvsp[-1].vInt); ;
    break;}
case 66:
#line 830 "lua.stx"
{
	  code_byte(0);  /* save space for opcode */
	  code_byte(yyvsp[-1].vInt+yyvsp[0].vInt);  /* number of parameters */
	  yyval.vLong = L->currState->pc;
	  code_byte(0);  /* must be adjusted by other rules */
	;
    break;}
case 67:
#line 838 "lua.stx"
{ yyval.vInt = 0; ;
    break;}
case 68:
#line 840 "lua.stx"
{
               code_oparg(PUSHSELF, 8, string_constant(yyvsp[0].pTStr, L->currState), 1);
               yyval.vInt = 1;
	     ;
    break;}
case 69:
#line 846 "lua.stx"
{ yyval.vInt = adjust_functioncall(yyvsp[-1].vLong, 1); ;
    break;}
case 70:
#line 847 "lua.stx"
{ yyval.vInt = 1; ;
    break;}
case 71:
#line 848 "lua.stx"
{ code_string(yyvsp[0].pTStr); yyval.vInt = 1; ;
    break;}
case 72:
#line 851 "lua.stx"
{ yyval.vLong = 0; ;
    break;}
case 73:
#line 852 "lua.stx"
{ yyval.vLong = yyvsp[0].vLong; ;
    break;}
case 74:
#line 855 "lua.stx"
{ if (yyvsp[0].vLong != 0) yyval.vLong = yyvsp[0].vLong; else yyval.vLong = -1; ;
    break;}
case 75:
#line 856 "lua.stx"
{ yyval.vLong = adjust_functioncall(yyvsp[-1].vLong, 1); ;
    break;}
case 76:
#line 857 "lua.stx"
{
	  if (yyvsp[0].vLong == 0) yyval.vLong = -(yyvsp[-1].vLong + 1);  /* -length */
	  else {
            L->currState->f->code[yyvsp[0].vLong] = yyvsp[-1].vLong;  /* store list length */
	    yyval.vLong = yyvsp[0].vLong;
	  }
	;
    break;}
case 77:
#line 866 "lua.stx"
{ code_args(0, 0); ;
    break;}
case 78:
#line 867 "lua.stx"
{ code_args(0, 1); ;
    break;}
case 79:
#line 868 "lua.stx"
{ code_args(yyvsp[0].vInt, 0); ;
    break;}
case 80:
#line 869 "lua.stx"
{ code_args(yyvsp[-2].vInt, 1); ;
    break;}
case 81:
#line 872 "lua.stx"
{ yyval.vInt = abs(yyvsp[0].vInt); ;
    break;}
case 82:
#line 874 "lua.stx"
{
	  if (yyvsp[-2].vInt*yyvsp[0].vInt > 0)  /* repeated parts? */
	    luaY_error("invalid constructor syntax");
	  yyval.vInt = abs(yyvsp[-2].vInt)+abs(yyvsp[0].vInt);
	;
    break;}
case 83:
#line 881 "lua.stx"
{ yyval.vInt = 0; ;
    break;}
case 84:
#line 882 "lua.stx"
{ yyval.vInt = yyvsp[0].vInt; ;
    break;}
case 85:
#line 883 "lua.stx"
{ yyval.vInt = yyvsp[0].vInt; ;
    break;}
case 88:
#line 889 "lua.stx"
{
	  flush_record(yyvsp[-1].vInt%RFIELDS_PER_FLUSH);
	  yyval.vInt = -yyvsp[-1].vInt;  /* negative signals a "record" part */
	;
    break;}
case 89:
#line 896 "lua.stx"
{
	  flush_list(yyvsp[-1].vInt/LFIELDS_PER_FLUSH, yyvsp[-1].vInt%LFIELDS_PER_FLUSH);
	  yyval.vInt = yyvsp[-1].vInt;
	;
    break;}
case 90:
#line 902 "lua.stx"
{yyval.vInt=1;;
    break;}
case 91:
#line 904 "lua.stx"
{
		  yyval.vInt=yyvsp[-2].vInt+1;
		  if (yyval.vInt%RFIELDS_PER_FLUSH == 0)
	            flush_record(RFIELDS_PER_FLUSH);
		;
    break;}
case 95:
#line 917 "lua.stx"
{yyval.vInt=1;;
    break;}
case 96:
#line 919 "lua.stx"
{
		  yyval.vInt=yyvsp[-2].vInt+1;
		  if (yyval.vInt%LFIELDS_PER_FLUSH == 0)
		    flush_list(yyval.vInt/LFIELDS_PER_FLUSH - 1, LFIELDS_PER_FLUSH);
		;
    break;}
case 97:
#line 926 "lua.stx"
{ yyval.vInt = 1; add_varbuffer(yyvsp[0].vLong, 0); ;
    break;}
case 98:
#line 927 "lua.stx"
{ add_varbuffer(yyvsp[0].vLong, yyvsp[-2].vInt); yyval.vInt = yyvsp[-2].vInt+1; ;
    break;}
case 99:
#line 930 "lua.stx"
{store_localvar(yyvsp[0].pTStr, 0); yyval.vInt = 1;;
    break;}
case 100:
#line 931 "lua.stx"
{ store_localvar(yyvsp[0].pTStr, yyvsp[-2].vInt); yyval.vInt = yyvsp[-2].vInt+1; ;
    break;}
case 101:
#line 934 "lua.stx"
{ yyval.vInt = 0; ;
    break;}
case 102:
#line 935 "lua.stx"
{ yyval.vInt = yyvsp[0].vLong; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 938 "lua.stx"


