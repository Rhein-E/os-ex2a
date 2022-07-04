#define NULL ((void *)0)

volatile void exit(int exit_code) {
    __asm__("int $0x80" ::"a"(1), "b"((long)exit_code));
    while (1)
        ;
}

volatile int fork() {
    int ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(2));
    return ret;
}
volatile int read(int fd, char *buf, int count) {
    int ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(3), "b"((long)fd), "c"((long)buf), "d"((long)count));
    return ret;
}
volatile int write(int fd, char *buf, int count) {
    int ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(4), "b"((long)fd), "c"((long)buf), "d"((long)count));
    return ret;
}

volatile short waitpid(short pid, int *wait_stat, int options) {
    short ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(7), "b"((long)pid), "c"((long)wait_stat), "d"((long)options));
    return ret;
}
volatile int sync() {
    int ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(36));
    return ret;
}
volatile int execve(const char *file, char **argv, char **envp) {
    int ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(11), "b"((long)file), "c"((long)argv), "d"((long)envp));
    return ret;
}
volatile int chdir(const char *filename) {
    int ret;
    __asm__("int $0x80" : "=a"(ret) : "a"(12), "b"((long)filename));
    return ret;
}

char *strtok(char *s, const char *ct);
int strcmp(const char *cs, const char *ct);
char *strcpy(char *dest, const char *src);
void *memset(void *s, char c, int count);
char *strcat(char *dest, const char *src);
int print_string(char *buf);

void main() {
    int pid, status, num;
    char cmdstr[255];
    char *p;
    char cmdpath[80]; /* cmd with full path  */
    int argc = 0;
    char *argv[10];
    char *envp[] = {"HOME=/", NULL, NULL};

    while (1) {
        /* prompt */
        print_string("$ ");

        /* get command */
        memset((void *)cmdstr, (char)0, 255);
        if ((num = read(0, cmdstr, 255)) < 1)
            break;
        cmdstr[num - 1] = '\0';

        /* parse command */
        argc = 0;
        p = strtok(cmdstr, " \n\t");
        while (p && argc < 9) {
            argv[argc] = p;
            p = strtok(NULL, " \n\t");
            argc++;
        }
        argv[argc] = NULL;

        /* interpret internal commands */
        if (argc == 0)
            continue;
        if (strcmp(argv[0], "exit") == 0)
            break;
        if (strcmp(argv[0], "sync") == 0) {
            sync();
            continue;
        }
        if (strcmp(argv[0], "cd") == 0) {
            if ((argc == 1) || (strcmp(argv[1], "~") == 0))
                argv[1] = "/";
            if (chdir(argv[1]) < 0)
                print_string("cd error\n");
            continue;
        }

        /* execute external commands */
        strcpy(cmdpath, "/usr/bin/");
        strcat(cmdpath, argv[0]);
        if (!(pid = fork())) {
            if (execve(cmdpath, argv, envp) == -1)
                print_string("execve error\n");
            exit(0);
        }
        while (1)
            if (pid == waitpid(-1, &status, 0))
                break;
    }

    /* exit */
    exit(0);
}

// 下面是重写部分函数
int print_string(char *buf) {
    char *p = buf;
    int ret;
    while (*(p++))
        ;
    __asm__("movl $1, %%ebx\n"    // fd
            "movl %%ecx, %%ecx\n" // buf
            "movl %%edx, %%edx\n" // count
            "movl $4, %%eax\n"
            "int $0x80\n" // sys_write
            : "=a"(ret)
            : "c"(buf), "d"(p - buf)
            : "ebx");
}

// 下面是copy部分标准库函数
char *strcat(char *dest, const char *src) {
    __asm__("cld\n\t"
            "repne\n\t"
            "scasb\n\t"
            "decl %1\n"
            "1:\tlodsb\n\t"
            "stosb\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b" ::"S"(src),
            "D"(dest), "a"(0), "c"(0xffffffff)
            :);
    return dest;
}

char *strcpy(char *dest, const char *src) {
    __asm__("cld\n"
            "1:\tlodsb\n\t"
            "stosb\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b" ::"S"(src),
            "D"(dest)
            : "ax");
    return dest;
}

int ___strtok;
char *strtok(char *s, const char *ct) {
    register char *__res __asm__("si");
    __asm__("testl %1,%1\n\t"
            "jne 1f\n\t"
            "testl %0,%0\n\t"
            "je 8f\n\t"
            "movl %0,%1\n"
            "1:\txorl %0,%0\n\t"
            "movl $-1,%%ecx\n\t"
            "xorl %%eax,%%eax\n\t"
            "cld\n\t"
            "movl %4,%%edi\n\t"
            "repne\n\t"
            "scasb\n\t"
            "notl %%ecx\n\t"
            "decl %%ecx\n\t"
            "je 7f\n\t" /* empty delimeter-string */
            "movl %%ecx,%%edx\n"
            "2:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 7f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "je 2b\n\t"
            "decl %1\n\t"
            "cmpb $0,(%1)\n\t"
            "je 7f\n\t"
            "movl %1,%0\n"
            "3:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 5f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "jne 3b\n\t"
            "decl %1\n\t"
            "cmpb $0,(%1)\n\t"
            "je 5f\n\t"
            "movb $0,(%1)\n\t"
            "incl %1\n\t"
            "jmp 6f\n"
            "5:\txorl %1,%1\n"
            "6:\tcmpb $0,(%0)\n\t"
            "jne 7f\n\t"
            "xorl %0,%0\n"
            "7:\ttestl %0,%0\n\t"
            "jne 8f\n\t"
            "movl %0,%1\n"
            "8:"
            : "=b"(__res), "=S"(___strtok)
            : "0"(___strtok), "1"(s), "g"(ct));
    return __res;
}

void *memset(void *s, char c, int count) {
    __asm__("cld\n\t"
            "rep\n\t"
            "stosb" ::"a"(c),
            "D"(s), "c"(count));
    return s;
}

int strcmp(const char *cs, const char *ct) {
    register int __res __asm__("ax");
    __asm__("cld\n"
            "1:\tlodsb\n\t"
            "scasb\n\t"
            "jne 2f\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b\n\t"
            "xorl %%eax,%%eax\n\t"
            "jmp 3f\n"
            "2:\tmovl $1,%%eax\n\t"
            "jl 3f\n\t"
            "negl %%eax\n"
            "3:"
            : "=a"(__res)
            : "D"(cs), "S"(ct)
            :);
    return __res;
}