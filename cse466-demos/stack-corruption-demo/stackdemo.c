#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_string>\n", argv[0]);
        return 1;
    }

    uint64_t secret = 0x1122334455667788ULL;
    char     buf[8];

    strcpy(buf, argv[1]);

    printf("The value of secret is: 0x%016lx\n", secret);

    return 0;
}