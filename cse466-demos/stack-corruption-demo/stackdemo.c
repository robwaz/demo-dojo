#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char **argv) {
    uint64_t secret = 0x1122334455667788ULL;
    char     buf[8];

    strcpy(buf, argv[1]);

    return 0;
}
