#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

char scratch[0x200];

#define GADGET(name, body)                                                   \
    __attribute__((naked, used, section(".text"))) void name(void)           \
    {                                                                        \
        __asm__ volatile(body);                                              \
    }

GADGET(pop_rdi, "pop %rdi\n ret\n")  // 1st argument (and syscall arg 1)
GADGET(pop_rsi, "pop %rsi\n ret\n")  // 2nd argument
GADGET(pop_rdx, "pop %rdx\n ret\n")  // 3rd argument
GADGET(pop_rax, "pop %rax\n ret\n")  // syscall number
GADGET(ret_gadget, "ret\n")          // alignment / nop slide

// The same four functions you can reach through the PLT are also system calls.
// Same arguments, same registers -- rax picks which one, and the kernel is the
// callee. Nothing in a chain cares whether its target is code or the kernel.
GADGET(syscall_gadget, "syscall\n ret\n")

static const char *name_of(unsigned long a, const char **kind)
{
    *kind = "injected gadget";
    if (a == (unsigned long)pop_rdi)        return "pop rdi ; ret";
    if (a == (unsigned long)pop_rsi)        return "pop rsi ; ret";
    if (a == (unsigned long)pop_rdx)        return "pop rdx ; ret";
    if (a == (unsigned long)pop_rax)        return "pop rax ; ret";
    if (a == (unsigned long)ret_gadget)     return "ret";
    if (a == (unsigned long)syscall_gadget) return "syscall ; ret";

    *kind = "library call, via the PLT";
    if (a == (unsigned long)open)       return "open()";
    if (a == (unsigned long)read)       return "read()";
    if (a == (unsigned long)write)      return "write()";
    if (a == (unsigned long)puts)       return "puts()";
    if (a == (unsigned long)printf)     return "printf()";
    return NULL;
}

static void print_gadget(unsigned long addr)
{
    unsigned char vec[64];
    const char *name, *kind;

    printf("| 0x%016lx : ", addr);

    if (addr < 0x1000)
    {
        printf("%-20s <- VALUE (%lu), popped into a register\n", "", addr);
        return;
    }

    if ((name = name_of(addr, &kind)))
    {
        printf("%-20s <- %s\n", name, kind);
        return;
    }

    if (addr >= (unsigned long)scratch &&
        addr < (unsigned long)scratch + sizeof scratch)
    {
        printf("%-20s <- POINTER into .bss (scratch+%#lx)\n", "",
               addr - (unsigned long)scratch);
        return;
    }

    if (mincore((void *)(addr & ~0xfffUL), 64, vec) < 0 && errno == ENOMEM)
    {
        printf("%-20s <- UNMAPPED: this address does not exist\n", "");
        return;
    }

    {
        const char *s = (const char *)addr;
        int printable = (s[0] != 0);

        for (int k = 0; k < 16 && s[k]; k++)
            if (s[k] < 0x20 || s[k] > 0x7e)
            {
                printable = 0;
                break;
            }
        if (printable)
        {
            printf("%-20s <- POINTER to the string \"%.28s\"\n", "", s);
            return;
        }
    }

    printf("%-20s <- mapped; bytes:", "");
    for (int k = 0; k < 8; k++)
        printf(" %02hhx", ((uint8_t *)addr)[k]);
    printf("\n");
}

static void print_chain(unsigned long *chain, int slots)
{
    printf("\n+--- your chain, as the CPU will see it (%d slots from the saved "
           "return address)\n",
           slots);
    for (int i = 0; i < slots; i++)
        print_gadget(chain[i]);
    printf("+--- main is about to return into the first slot.\n\n");
}

// ------------------------------------------------------------------- main

int main(void)
{
    char buf[64];
    static ssize_t n;
    static long ret_off;
    static int devnull;
    static char *bufp;

    setvbuf(stdout, NULL, _IONBF, 0);

    devnull = open("/dev/null", O_RDONLY);
    if (devnull >= 0)
        close(devnull);
    write(1, "", 0);

    bufp = buf;
    ret_off = (char *)__builtin_frame_address(0) + 8 - buf;

    printf("  scratch (.bss)   %p   %#zx bytes, address\n",
           (void *)scratch, sizeof scratch);
    printf("  offset to saved return address   %#lx\n\n", ret_off);
    printf("  pop rdi ; ret    %p\n", (void *)pop_rdi);
    printf("  pop rsi ; ret    %p\n", (void *)pop_rsi);
    printf("  pop rdx ; ret    %p\n", (void *)pop_rdx);
    printf("  pop rax ; ret    %p\n", (void *)pop_rax);
    printf("  ret              %p\n", (void *)ret_gadget);
    printf("  syscall ; ret    %p\n\n", (void *)syscall_gadget);
    printf("  open   %p\n", (void *)open);
    printf("  read   %p\n", (void *)read);
    printf("  write  %p\n\n", (void *)write);
    printf("overflow %#lx bytes of buffer and build a chain:\n> ", sizeof buf);

    n = read(0, buf, 0x400);
    if (n <= 0)
        return 0;

    if (n > ret_off)
        print_chain((unsigned long *)(bufp + ret_off), (n - ret_off) / 8);

    return 0;
}
