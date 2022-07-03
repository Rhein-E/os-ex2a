int str2int(int *dest, char *buf) {
    char *p = buf;
    *dest = 0;
    while (*p) {
        if (*p >= '0' && *p <= '9')
            *dest = *dest * 10 + (*p - '0');
        else if (*p != '-')
            return -1;
        ++p;
    }
    if (*buf == '-')
        *dest = -*dest;
    return 0;
}

int int2str(char *dest, int num) {
    char *p = dest;
    int tmp = num;
    if (tmp == 0)
        *(p++) = '0';
    else {
        if (tmp < 0) {
            tmp = -tmp;
            *(dest++) = '-';
            p = dest;
        }
        while (tmp) {
            *(p++) = '0' + tmp % 10;
            tmp /= 10;
        }
    }
    *p = '\0';

    while (dest < (--p)) {
        *dest ^= *p;
        *p ^= *dest;
        *dest ^= *p;
        ++dest;
    }

    return 0;
}

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

void exit(int exit_code) {
    __asm__("movl $1, %%eax\n"
            "movl %%ebx, %%ebx\n"
            "int $0x80\n" // sys_exit
            ::"b"(exit_code));
    while (1)
        ; // 防止编译警告
}

int a, b;
int main(int argc, char **argv, char **envp) {
    char buf[32];

    if (argc != 3) {
        print_string("need 2 numbers\n");
        exit(1);
    }
    str2int(&a, argv[1]);
    str2int(&b, argv[2]);
    int2str(buf, a + b);
    print_string(buf);
    print_string("\n");

    exit(0);
}
