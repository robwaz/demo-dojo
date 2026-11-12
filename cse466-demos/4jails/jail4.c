#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>

#define NAME "jail4c"
#define PAGE 0x1000

static void only_read(void)
{
    struct sock_filter filter[] = {
        // kill anything not arriving on the x86-64 ABI

        // A = seccomp_data.arch
        //   BPF_LD = load into A, BPF_W = a 32-bit word, BPF_ABS = from the fixed offset k
        BPF_STMT(BPF_LD  | BPF_W   | BPF_ABS, offsetof(struct seccomp_data, arch)),
        // if (A == AUDIT_ARCH_X86_64) skip 1, hopping over the kill; else skip 0, landing on it
        //   BPF_JMP = branch, BPF_JEQ = test for equality, BPF_K = compare against the constant k
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        // return k as the verdict: kill the whole process
        //   BPF_RET = return from the filter, BPF_K = the returned value is the constant k
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),

        // allow read(), kill everything else

        // A = seccomp_data.nr, the syscall number
        BPF_STMT(BPF_LD  | BPF_W   | BPF_ABS, offsetof(struct seccomp_data, nr)),
        // if (A == __NR_read) skip 0, landing on the allow; else skip 1, hopping over it to the kill
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_read, 0, 1),
        // return k as the verdict: let the syscall run
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        // return k as the verdict: kill the whole process
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL_PROCESS),
    };
    struct sock_fprog prog = { .len = sizeof(filter) / sizeof(filter[0]), .filter = filter };

    // required to install a filter without CAP_SYS_ADMIN
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0)
    {
        perror("no_new_privs");
        exit(1);
    }

    // install the filter, every syscall from here on goes through it
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog, 0, 0) != 0)
    {
        perror("set_seccomp");
        exit(1);
    }
}

// we hand this address to the shellcode in rdi, so it never has to go find it
static char secret[256];

int main(int argc, char **argv)
{
    // need exactly one argument: a file holding the code to run inside the jail
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <shellcode-file>\n", argv[0]);
        return 2;
    }

    // load your code while open() and mmap() still work
    int sc = open(argv[1], O_RDONLY);
    if (sc < 0)
    {
        perror(argv[1]);
        return 1;
    }

    // a writable page to land it in, not executable yet
    void *page = mmap(NULL, PAGE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (page == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    ssize_t len = read(sc, page, PAGE);
    if (len <= 0)
    {
        perror("read shellcode");
        return 1;
    }
    close(sc);

    // flip the page to executable: never writable and executable at the same time
    if (mprotect(page, PAGE, PROT_READ | PROT_EXEC) != 0)
    {
        perror("mprotect");
        return 1;
    }
    fprintf(stderr, "[" NAME "] loaded %zd bytes of shellcode\n", len);

    // open the secret BEFORE the filter goes on
    int secret_fd = open("/flag", O_RDONLY);
    if (secret_fd < 0)
    {
        perror("open /flag (is this binary running as root?)");
        return 1;
    }

    // read it in while we still have a syscall that can put it somewhere
    if (read(secret_fd, secret, sizeof(secret)) <= 0)
    {
        perror("read /flag");
        return 1;
    }
    close(secret_fd);

    // lock down to read() only, no open, write or exit after this
    only_read();

    // hand over control: rdi = secret, rsi = its length, and no way to print it
    ((void (*)(char *, size_t))page)(secret, sizeof(secret));

    // exit_group() isn't allowed either, so returning dies with SIGSYS
    return 0;
}