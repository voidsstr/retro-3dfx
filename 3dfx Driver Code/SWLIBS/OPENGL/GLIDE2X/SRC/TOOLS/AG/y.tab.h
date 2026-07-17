
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
  char *str;
  enum eval val;
  int i;
} YYSTYPE;
extern YYSTYPE yylval;
# define VANILLA 257
# define LEADER 258
# define IDENT 259
# define DOT 260
