#define _GNU_SOURCE
#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef CAPSTONE_ARCH
#define CAPSTONE_ARCH CS_ARCH_X86
#endif
#ifndef CAPSTONE_MODE
#define CAPSTONE_MODE CS_MODE_64
#endif

static void print_disassembly(void *shellcode_addr, size_t shellcode_size)
{
  csh handle;
  cs_insn *insn = NULL;

  if (cs_open(CAPSTONE_ARCH, CAPSTONE_MODE, &handle) != CS_ERR_OK) {
    fprintf(stderr, "ERROR: disassembler failed to initialize.\n");
    return;
  }

  size_t count = cs_disasm(handle,
                           (const uint8_t *)shellcode_addr,
                           shellcode_size,
                           (uint64_t)(uintptr_t)shellcode_addr,
                           0,
                           &insn);

  if (count > 0) {
    printf("      Address      |                      Bytes                    |          Instructions\n");
    printf("------------------------------------------------------------------------------------------\n");
    for (size_t j = 0; j < count; j++) {
      printf("0x%016" PRIx64 " | ", insn[j].address);
      for (int k = 0; k < insn[j].size; k++) printf("%02hhx ", insn[j].bytes[k]);
      for (int k = insn[j].size; k < 15; k++) printf("   ");
      printf(" | %s %s\n", insn[j].mnemonic, insn[j].op_str);
    }
    cs_free(insn, count);
  } else {
    fprintf(stderr, "ERROR: Failed to disassemble shellcode! Bytes are:\n\n");
    fprintf(stderr, "      Address      |                      Bytes\n");
    fprintf(stderr, "--------------------------------------------------------------------\n");
    const uint8_t *p = (const uint8_t *)shellcode_addr;
    for (size_t i = 0; i < shellcode_size; i += 16) {
      fprintf(stderr, "0x%016" PRIx64 " | ", (uint64_t)(uintptr_t)((const uint8_t*)shellcode_addr + i));
      size_t line = (shellcode_size - i < 16) ? (shellcode_size - i) : 16;
      for (size_t k = 0; k < line; k++) fprintf(stderr, "%02hhx ", p[i+k]);
      for (size_t k = line; k < 16; k++) fprintf(stderr, "   ");
      fprintf(stderr, "\n");
    }
  }

  cs_close(&handle);
}

static int verify_unique_opcodes(const uint8_t *buf, size_t size, bool verbose)
{
  if (size == 0) {
    if (verbose) fprintf(stderr, "ERROR: empty input.\n");
    return 2;
  }

  csh handle;
  if (cs_open(CAPSTONE_ARCH, CAPSTONE_MODE, &handle) != CS_ERR_OK) {
    if (verbose) fprintf(stderr, "ERROR: Capstone init failed.\n");
    return 2;
  }

  cs_option(handle, CS_OPT_DETAIL, CS_OPT_OFF);

  cs_insn *insn = NULL;
  size_t count = cs_disasm(handle, buf, size, (uint64_t)(uintptr_t)buf, 0, &insn);
  if (count == 0 || !insn) {
    if (verbose) fprintf(stderr, "ERROR: Disassembly failed (0 instructions).\n");
    cs_close(&handle);
    return 2;
  }

  typedef struct {
    bool seen;
    uint64_t first_addr;
    const char *first_mnemonic;
  } slot_t;

  slot_t seen[X86_INS_ENDING];
  memset(seen, 0, sizeof(seen));

  bool dup_found = false;

  for (size_t i = 0; i < count; i++) {
    unsigned id = insn[i].id;
    const char *mn = insn[i].mnemonic ? insn[i].mnemonic : "(?)";
    uint64_t addr = insn[i].address;

    if (id == 0 || id >= X86_INS_ENDING) continue;
    if (id == X86_INS_SYSCALL) continue;

    if (!seen[id].seen) {
      seen[id].seen = true;
      seen[id].first_addr = addr;
      seen[id].first_mnemonic = mn;
    } else {
      if (!dup_found) {
        fprintf(stderr, "\nDuplicate opcode(s) detected (besides allowed 'syscall'):\n");
      }
      fprintf(stderr, "  %-12s first at 0x%016" PRIx64 ", again at 0x%016" PRIx64 "\n",
              seen[id].first_mnemonic, seen[id].first_addr, addr);
      dup_found = true;
    }
  }

  if (!dup_found && verbose) {
    fprintf(stderr, "OK: All opcodes are unique (except 'syscall' which may repeat).\n");
  }

  cs_free(insn, count);
  cs_close(&handle);
  return dup_found ? 1 : 0;
}

int main(void) {
    const size_t pagesz = 4096;

    void *page = mmap(NULL, pagesz, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (page == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    puts("Ready to receive shellcode bytes!");
    size_t have = 0;
    while (have < pagesz) {
        ssize_t n = read(STDIN_FILENO, (uint8_t *)page + have, pagesz - have);
        if (n < 0) { 
          perror("read"); 
          exit(1);
        }
        if (n == 0) break;
        have += (size_t)n;
    }
    printf("You sent %zu bytes!\n", have);

    print_disassembly(page, have);

    if (verify_unique_opcodes(page, have, true) != 0) {
        perror("duplicate opcodes found!");
        exit(1);
    }

    puts("Executing shellcode!");
    fflush(stdout);

    if (mprotect(page, pagesz, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        exit(1);
    }

    ((void (*)(void))page)();

    puts("Goodbye!");
    munmap(page, pagesz);
    return 0;
}

