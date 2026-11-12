#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>

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

int main(int argc, char **argv)
{
    // need exactly one argument: a byte index, or --try-write
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <byte-index>\n", argv[0]);
        fprintf(stderr, "       %s --try-write\n", argv[0]);
        return 2;
    }

    // open the secret BEFORE the filter goes on
    int secret_fd = open("/flag", O_RDONLY);
    if (secret_fd < 0)
    {
        perror("open /flag (is this binary running as root?)");
        return 1;
    }

    // lock down to read() only, no open, write or exit after this
    only_read();

    // read the secret through the fd we already have
    char secret[256] = { 0 };
    ssize_t n = read(secret_fd, secret, sizeof(secret));

    // the real leak: consume secret[index] bytes of stdin, moving its shared offset by that much
    int index = atoi(argv[1]);
    char scratch[256];
    read(0, scratch, (unsigned char)secret[index]);

    // exit_group() isn't allowed either, so returning dies with SIGSYS
    return 0;
}