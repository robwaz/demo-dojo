#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define CHAIN_ADDR  0x13370000UL
#define CHAIN_BYTES 0x100


#define MAP_ADDR  (CHAIN_ADDR - 0x10000UL)
#define MAP_SIZE  0x11000UL


static void handle(void)
{
    char buf[0x40];

    size_t off = (size_t)((char *)__builtin_frame_address(0) - buf);

    printf("overflow now:");
    read(0, buf, off + 10);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    if (mmap((void *)MAP_ADDR, MAP_SIZE, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) != (void *)MAP_ADDR)
    {
        perror("mmap");
        return 1;
    }

    printf("exit: %p\n", dlsym(RTLD_NEXT, "exit"));

    printf("enter your ropchain (located at 0x%lx, send 0x%x bytes)> ", CHAIN_ADDR, CHAIN_BYTES);
    read(0, (void *)CHAIN_ADDR, CHAIN_BYTES);

    handle();
    return 0;
}