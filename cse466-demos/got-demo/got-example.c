#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <link.h>
#include <dlfcn.h>
#include <elf.h>


extern void *_GLOBAL_OFFSET_TABLE_;

uint64_t find_pltgot() {
    struct link_map *lm = NULL;
    void *self = dlopen(NULL, RTLD_NOW);
    dlinfo(self, RTLD_DI_LINKMAP, &lm);

    ElfW(Addr) gotplt = 0;
    for (ElfW(Dyn)* d = lm->l_ld; d->d_tag != DT_NULL; ++d)
        if (d->d_tag == DT_PLTGOT) { gotplt = d->d_un.d_ptr; break; }
    return gotplt;
}

void main() {
    uint64_t gotplt = find_pltgot();
    puts("hello world!\n");
    printf("The GOT.PLT is located at: 0x%llx\n", (unsigned long long) gotplt);   
}
