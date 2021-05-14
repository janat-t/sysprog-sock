#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>

/* logger.c */
#define LOG(...) l_logf(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define DIE(...) l_die(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define PERROR_DIE(cxt) l_pdie(__FILE__, __LINE__, __func__, (cxt))

int l_get_quiet(void);
void l_set_quiet(int quiet);
int l_logf(const char *file, int line, const char *func, const char *fmt, ...);
int l_die(const char *file, int line, const char *func, const char *fmt, ...);
int l_pdie(const char *file, int line, const char *func, const char *cxt);

#endif
