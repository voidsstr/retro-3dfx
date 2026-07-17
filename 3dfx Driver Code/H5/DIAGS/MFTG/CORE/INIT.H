/*
 * init.h
 *
 */

#ifndef MDC_INIT_H
#define MDC_INIT_H

extern FILE *stdout_file;
extern FILE *stderr_file;


extern int break_requested;

int mdc_init(void);
void mdc_exit(void);
void mdc_run_luascript(void);
void mdc_args_to_lua(int argc, char **argv);

void mdc_restore_text(void);


#endif
