#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

#define TARGET 0x1337000

int main(void) {
    long *target = mmap((void *)TARGET, 0x1000, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);

    *target = 0x13371337

    printf("setting target=%#lx\n", TARGET);

    // allocate two chunks
    void *a = malloc(0x20);
    void *b = malloc(0x20);

    // free them in a known order
    free(b); 
    free(a);

    // use the second free()'d chunk to corrupt the tcache list
    *(uintptr_t *)a = (uintptr_t)target;  

    // now allocate two chunks the first will take chunk a
    // the second will use the address that we inserted above!
    void *c = malloc(0x20);
    long *d = malloc(0x20);

    //now corrupt some data at 0x1337000
    *d = 0x4141414141414141;
    printf("a=%p c=%p\n", a, c);
    printf("target=%p d=%p\n", (void *)target, (void *)d);
    printf("*(long*)0x%x = %#lx\n", TARGET, *(long *)TARGET);
    return 0;
}