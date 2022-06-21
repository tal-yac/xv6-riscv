#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

#define BUFSIZE 1024

void part2(void) {
    char buf[BUFSIZE];
    memset(buf, 'x', BUFSIZE);
    int fd = open("testtest", O_CREATE | O_RDWR);
    if (fd < 0) {
        printf("open failed\n");
        exit(1);
    }

    for (int i = 0; i < 10 * 1024 * 1024;) {
        if (write(fd, buf, BUFSIZE) != BUFSIZE) {
            printf("failed writing\n");
            exit(1);
        }
        i += BUFSIZE;
        if (i == 12 * 1024)
            printf("Finished writing 12KB (direct)\n");
        else if (i == 268 * 1024)
            printf("Finished writing 268KB (single indirect)\n");
    }

    printf("Finished writing 10MB\n");
}

void part3(char **argv) {
    symlink("/ls", "/new_ls");
    symlink("/new_ls", "/super_new_ls");
    char buf[30];
    readlink("/super_new_ls", buf, 30);
    exec("/super_new_ls", argv);
    // unlink("/new_ls");
    printf("%s\n", buf);
}

int main(int argc, char *argv[]) {
    // part2();
    part3(argv);
    exit(0);
}