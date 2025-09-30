#define _GNU_SOURCE
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <capstone/capstone.h>

#define CAPSTONE_ARCH CS_ARCH_X86
#define CAPSTONE_MODE CS_MODE_64

void print_gadget(unsigned long *gadget_addr)
{
  csh handle;
  cs_insn *insn;
  size_t count;
  unsigned char vec[64];

  if (cs_open(CAPSTONE_ARCH, CAPSTONE_MODE, &handle) != CS_ERR_OK) {
    printf("ERROR: disassembler failed to initialize.\n");
    return;
  }

  printf("| 0x%016lx: ", (unsigned long)gadget_addr);

  int r = mincore((void *) ((uintptr_t)gadget_addr & ~0xfff), 64, vec);
  if (r < 0 && errno == ENOMEM) {
    printf("(UNMAPPED MEMORY)");
  }
  else {
    count = cs_disasm(handle, (void *)gadget_addr, 64, (uint64_t)gadget_addr, 0, &insn);
    if (count > 0) {
      for (size_t j = 0; j < count; j++) {
        printf("%s %s ; ", insn[j].mnemonic, insn[j].op_str);
        if (strcmp(insn[j].mnemonic, "ret") == 0 || strcmp(insn[j].mnemonic, "call") == 0) break;
      }

      cs_free(insn, count);
    }
    else {
      printf("(DISASSEMBLY ERROR) ");
      for (int k = 0; k < 16; k++) printf("%02hhx ", ((uint8_t*)gadget_addr)[k]);
    }
  }
  printf("\n");

  cs_close(&handle);
}

void print_chain(unsigned long **chain_addr, int chain_length)
{
  printf("\n+--- Printing %d gadgets of ROP chain at %p.\n", chain_length, chain_addr);
  for (int i = 0; i < chain_length; i++) {
    print_gadget(*(chain_addr + i));
  }
  printf("\n");
}

void free_gadgets()
{
    long long variable_0 = 0xC358;
    long long variable_1 = 0xC35F;
    long long variable_2 = 0xC35E;
    long long variable_3 = 0xC35A;
    long long variable_4 = 0xC35A41;
    long long variable_5 = 0xC35841;
    long long variable_6 = 0xC35941;
    long long variable_7 = 0xC3050F;
}


int main() {
    char buf[64];
    printf("[leak] main=%p\n", (void *)&main);
    puts("overflow a 64 byte buffer NOW");
    ssize_t n = read(0, buf, 0x1000);
    if (n <= 0) exit(1);

    print_chain((void *) &buf, n / 8);
    
}
