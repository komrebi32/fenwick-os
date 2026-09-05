#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>

#define SYSCALL_WRITE   1
#define SYSCALL_READ    2
#define SYSCALL_EXIT    3
#define SYSCALL_OPEN    4
#define SYSCALL_CLOSE   5
#define SYSCALL_GETCHAR 6
#define SYSCALL_PUTCHAR 7
#define SYSCALL_GETPID  8
#define SYSCALL_TIME    9

#define SYSCALL_MAX 32

typedef long (*syscall_fn_t)(long a1, long a2, long a3, long a4, long a5, long a6);

long sys_write(long fd, long buf, unsigned long len);
long sys_read(long fd, long buf, unsigned long len);
long sys_open(long path, long flags);
long sys_close(long fd);
long sys_getchar(long unused);
long sys_putchar(long c);
long sys_exit(long code);
long sys_getpid(long unused);
long sys_time(long unused);

void syscall_init(void);
void syscall_dispatcher(void);

int  syscall_register(int num, syscall_fn_t fn);
long syscall_handle(long num, long a1, long a2, long a3, long a4, long a5, long a6);

extern void syscall_entry(void);

void tss_init(void);
void tss_set_rsp0(uint64_t rsp0);

void jump_to_usermode(uint64_t entry, uint64_t user_stack);

#endif
