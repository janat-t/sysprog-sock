#include "main.h"
#include <errno.h>  /* errno */
#include <stdarg.h> /* va_list, ... */
#include <stdio.h>  /* perror, fprintf, sprintf */
#include <stdlib.h> /* exit */
#include <string.h> /* strerror */
#include <unistd.h> /* getpid */

#define MSG_SIZE 8192

/* quiet mode (-q option) */
int is_quiet = 0;

/** Ge/set quiet mode. */
int l_get_quiet(void) { return is_quiet; }
void l_set_quiet(int quiet) { is_quiet = quiet; }

/** Prints log message to stderr. */
int l_log(const char *file, int line, const char *func, const char *msg) {
    int pid = getpid();
    return fprintf(stderr, "%s:%d(%s) [%d]: %s\n", file, line, func, pid, msg);
}

/** Prints log message with parameters to stderr. */
int l_logf(const char *file, int line, const char *func, const char *fmt, ...) {
    if (is_quiet) {
        return 0;
    }
    char msg[MSG_SIZE];
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, sizeof(msg), fmt, va);
    va_end(va);
    return l_log(file, line, func, msg);
}

/** Prints error message to stderr and exit. */
int l_die(const char *file, int line, const char *func, const char *fmt, ...) {
    char msg[MSG_SIZE];
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, sizeof(msg), fmt, va);
    va_end(va);
    l_log(file, line, func, msg);
    exit(EXIT_FAILURE);
    return 1;
}

/** Prints errno-based error message to stderr and exit. */
int l_pdie(const char *file, int line, const char *func, const char *cxt) {
    if (cxt == NULL) {
        return l_die(file, line, func, strerror(errno));
    }
    char msg[MSG_SIZE];
    snprintf(msg, sizeof(msg), "%s: %s", cxt, strerror(errno));
    return l_die(file, line, func, msg);
}
