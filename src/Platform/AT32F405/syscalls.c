/* Minimal newlib syscall glue. nosys.specs supplies the rest as stubs. */
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int _write(int fd, const char *buf, int len) { (void)fd; (void)buf; return len; }
int _read(int fd, char *buf, int len)   { (void)fd; (void)buf; (void)len; return 0; }
int _close(int fd)                      { (void)fd; return -1; }
int _fstat(int fd, struct stat *st)     { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)                     { (void)fd; return 1; }
int _lseek(int fd, int off, int dir)    { (void)fd; (void)off; (void)dir; return 0; }
void _exit(int code)                    { (void)code; for (;;) {} }
int _kill(int pid, int sig)             { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void)                       { return 1; }
