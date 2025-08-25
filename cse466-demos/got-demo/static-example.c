#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <link.h>
#include <dlfcn.h>
#include <elf.h>


void main() {
    puts("Hello!\n");
    printf("Here is another %s\n", "function");   
    puts("Goodbye!");
}
