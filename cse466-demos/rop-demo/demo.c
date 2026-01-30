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

// Syscall gadget: syscall; ret
__attribute__((naked, used, section(".text")))
void syscall_gadget(void) {
    __asm__ volatile (
        "syscall\n"
        "ret\n"
    );
}

// pop rax; ret - syscall number
__attribute__((naked, used, section(".text")))
void pop_rax(void) {
    __asm__ volatile (
        "pop %rax\n"
        "ret\n"
    );
}

// pop rdi; ret - 1st argument
__attribute__((naked, used, section(".text")))
void pop_rdi(void) {
    __asm__ volatile (
        "pop %rdi\n"
        "ret\n"
    );
}

// pop rsi; ret - 2nd argument
__attribute__((naked, used, section(".text")))
void pop_rsi(void) {
    __asm__ volatile (
        "pop %rsi\n"
        "ret\n"
    );
}

// pop rdx; ret - 3rd argument
__attribute__((naked, used, section(".text")))
void pop_rdx(void) {
    __asm__ volatile (
        "pop %rdx\n"
        "ret\n"
    );
}

// pop r10; ret - 4th argument (syscall convention)
__attribute__((naked, used, section(".text")))
void pop_r10(void) {
    __asm__ volatile (
        "pop %r10\n"
        "ret\n"
    );
}

// pop r8; ret - 5th argument
__attribute__((naked, used, section(".text")))
void pop_r8(void) {
    __asm__ volatile (
        "pop %r8\n"
        "ret\n"
    );
}

// pop r9; ret - 6th argument
__attribute__((naked, used, section(".text")))
void pop_r9(void) {
    __asm__ volatile (
        "pop %r9\n"
        "ret\n"
    );
}

// Bonus useful gadgets:

// pop rbx; ret
__attribute__((naked, used, section(".text")))
void pop_rbx(void) {
    __asm__ volatile (
        "pop %rbx\n"
        "ret\n"
    );
}

// pop rcx; ret
__attribute__((naked, used, section(".text")))
void pop_rcx(void) {
    __asm__ volatile (
        "pop %rcx\n"
        "ret\n"
    );
}

// pop rbp; ret
__attribute__((naked, used, section(".text")))
void pop_rbp(void) {
    __asm__ volatile (
        "pop %rbp\n"
        "ret\n"
    );
}

// xor rax, rax; ret (zero rax)
__attribute__((naked, used, section(".text")))
void xor_rax(void) {
    __asm__ volatile (
        "xor %rax, %rax\n"
        "ret\n"
    );
}

// ret (for stack alignment / nop slide)
__attribute__((naked, used, section(".text")))
void ret_gadget(void) {
    __asm__ volatile (
        "ret\n"
    );
}
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


int main() {
    char buf[64];
    printf("[leak] main=%p\n", (void *)&main);
    printf("[leak] syscall_gadget=%p\n", (void *)&syscall_gadget);
    puts("overflow a 64 byte buffer NOW");
    ssize_t n = read(0, buf, 0x1000);
    if (n <= 0) return 0;

    print_chain((void *) &buf, n / 8);
}
