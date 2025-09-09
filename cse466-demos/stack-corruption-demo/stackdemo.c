#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct frame {
    char buf[8];
    char robs_lost_manpages[64];
    uint64_t secret3;
    uint64_t secret2;
    uint64_t secret;
};

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_string>\n", argv[0]);
        return 1;
    }

    struct frame f = {
        .secret  = 0x1122334455667788ULL,
        .secret2 = 0x123123123123123ULL,
        .secret3 = 0x567567567567568ULL,
    };

    strcpy(f.buf, argv[1]);

    printf("The value of secret is: 0x%016lx\n", f.secret);

    return 0;
}
