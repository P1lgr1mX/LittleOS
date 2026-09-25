#ifndef _UNISTD_H
#define _UNISTD_H

#define SYS_EXIT  1
#define SYS_FORK  2
#define SYS_READ  3
#define SYS_WRITE 4
#define SYS_CLEAR 5

int sys_write(int fd, const void *buf, unsigned int count);
int sys_read(int fd, void *buf, unsigned int count);
void sys_exit(int code);
int sys_clear(void);

#endif /* _UNISTD_H */
