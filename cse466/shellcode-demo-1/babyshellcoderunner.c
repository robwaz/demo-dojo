#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
int main(void)
{
    char *page = mmap(0, 4096, PROT_READ|PROT_WRITE|PROT_EXEC,
                      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    puts("Ready to receive shellcode bytes!");
    int recv = read(0, page, 4096);
    printf("You sent %d bytes!\n", recv);
    puts("Executing shellcode!");
    ((void(*)())page)();
    puts("Goodbye!");
}
