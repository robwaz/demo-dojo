#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    char vulnerable_buffer[64];
    char shared_netflix_password[16];
    char yans_borscht_recipe[64];
    char enemy_player_coordinates[32];
    char robs_lost_manpage[64];
    int  local_variable;
} FrameDemo;

static void print_row(const char *label, void *p, void *base, size_t off) {
    long long delta = (long long)((intptr_t)p - (intptr_t)base);
    printf("%-26s %p  %+6lld   (offsetof=%zu)\n", label, p, delta, off);
}

static void leaf(int verbose, unsigned long long guess) {
    volatile FrameDemo f;
    memset((void*)&f, 0, sizeof(f));
    void *base = (void*)&f.vulnerable_buffer[0];
    if (verbose) {
        puts("label                      address                bytes_from_base   offsetof");
        print_row("vulnerable buffer",
                  (void*)&f.vulnerable_buffer[0], base,
                  offsetof(FrameDemo, vulnerable_buffer));
        print_row("shared netflix password",
                  (void*)&f.shared_netflix_password[0], base,
                  offsetof(FrameDemo, shared_netflix_password));
        print_row("yan's borscht recipe",
                  (void*)&f.yans_borscht_recipe[0], base,
                  offsetof(FrameDemo, yans_borscht_recipe));
        print_row("enemy player coordinates",
                  (void*)&f.enemy_player_coordinates[0], base,
                  offsetof(FrameDemo, enemy_player_coordinates));
        print_row("rob's lost manpage",
                  (void*)&f.robs_lost_manpage[0], base,
                  offsetof(FrameDemo, robs_lost_manpage));
        print_row("local variable",
                  (void*)&f.local_variable, base,
                  offsetof(FrameDemo, local_variable));
    }
    if (guess == (unsigned long long)((intptr_t)&f.local_variable - (intptr_t)base)) {
        int fd = open("/flag", O_RDONLY);
        char buf[4096];
        write(1, buf, read(fd, buf, sizeof(buf)));
    }
}

int main(int argc, char **argv) {
    int verbose = 0;
    for (int i = 1; i < argc; i++) if (strcmp(argv[i], "-v") == 0) verbose = 1;
    unsigned long long guess;
    printf("how far is the start of your vulnerable buffer to local variable?\n");
    scanf("%lld", &guess);
    leaf(verbose, guess);
    return 0;
}
