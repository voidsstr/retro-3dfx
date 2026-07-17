#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int genheader = 1;

main(int argc, char **argv)
{
  char line[200], name[100], str[100];
  char *p, *e, *strStart, *strEnd;
  int i, n;
  FILE *fp;

  for (n=1; n < argc; n++) {
      if (!strcmp(argv[n], "-t")) {
	genheader = 0;
      } else {
	fp = fopen(argv[n], "r");
	if (!fp) {
	  printf("cannot open file %s\n", argv[n]);
	  exit(1);
	}
      }
  }

  while (fgets(line, sizeof(line), fp)) {
    n = strlen(line);

    /* look for first paren */
    for (i=0,p=line; i < n; i++,p++) {
      if (*p == '(') break;
    }
    if (i == n) {
      continue;
    }
    /* look for end of variable name */
    for (i=0,e=p+1; *e; i++,e++) {
      if (*e == ',' || *e == ' ') break;
    }
    if (!*e) {
      continue;
    }
    strncpy(name, p+1, i);
    name[i] = 0;

    /* look for first quote */
    for (; *e; e++) {
      if (*e == '"') break;
    }
    if (!*e) continue;
    strStart = e+1;

    /* look for second quote */
    for (++e; *e; e++) {
      if (*e == '"') break;
    }
    if (!*e) continue;
    strEnd = e-1;
    strncpy(str, strStart, strEnd-strStart+1);
    str[strEnd-strStart+1] = 0;

    if (genheader) {
      /* make header entries */
      printf("\tint %s; char *str%s;\n", name, name);
    } else {
      /* make table entries */
      printf("\t0, \"%s\",\n", str);
    }
  }
return 0;
}
