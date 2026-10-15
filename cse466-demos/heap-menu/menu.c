#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define NSLOT 16

static char *ptr[NSLOT];

oid (*hook)(void)  __attribute__((aligned(0x40))) = NULL;
long  guard         __attribute__((aligned(0x40))) = 0x1337;
char  secret[0x20]  __attribute__((aligned(0x40))) =
      "hack_the_planet";
static char heap_spacer[0x1000];

void win(void) { puts("\n[*] hook() hijacked -> win()"); _exit(0); }


static long num(const char *p) {
    char b[0x20];
    fputs(p, stdout);
    if (!fgets(b, sizeof b, stdin)) _exit(0);
    return strtol(b, 0, 0);
}
static void menu(void) {
    puts("\n--- heap manager ---------------------------------");
    puts(" 1) alloc    2) free     3) write    4) show");
    puts(" 5) note     6) status   7) quit");
}

int main(void) {
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stdin,  0, _IONBF, 0);
    (void) heap_spacer;

    puts("welcome to da heap)");

    for (;;) {
        menu();
        switch (num("cmd: ")) {

        case 1: {
            long i = num("idx: "), n = num("size: ");
            if (i < 0 || i >= NSLOT) break;
            ptr[i] = malloc(n);
            if ((char *)ptr[i] >= secret - 0x20 &&
                (char *)ptr[i] <  secret + sizeof secret) {
                puts("[!] allocation too close to the secret - discarded");
                ptr[i] = NULL;
                break;
            }
            printf("[+] slot %ld @ %s\n", i, ptr[i] ? "ok" : "null");
            break;
        }
        case 2: {
            long i = num("idx: ");
            if (i < 0 || i >= NSLOT) break;
            free(ptr[i]);
            printf("[+] slot %ld freed\n", i);
            break;
        }
        case 3: {
            long i = num("idx: "), n = num("len: ");
            if (i < 0 || i >= NSLOT || !ptr[i]) break;
            fputs("[+] data: ", stdout);
            read(0, ptr[i], n);
            break;
        }
        case 4: {
            long i = num("idx: "), n = num("len: ");
            if (i < 0 || i >= NSLOT || !ptr[i]) break;
            printf("[+] slot %ld: ", i);
            write(1, ptr[i], n);
            putchar('\n');
            break;
        }
        case 5: {
            char note[0x80];
            long off = num("offset: ");
            printf("[dbg] note buffer @ %p\n", (void *)note);
            fputs("[+] note: ", stdout);
            read(0, note, sizeof note);
            free(note + off);
            puts("[+] note discarded");
            break;
        }
        case 6:
            if (guard == 0) puts("[+] SYSTEM UNLOCKED");
            else            printf("[-] locked (guard = %#lx)\n", guard);
            break;

        case 7:
            if (hook) hook();
            puts("bye");
            _exit(0);
        }
    }
}