#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Data {
    char vulnerable_buffer[64];
    char shared_netflix_password[16];
    char yans_borscht_recipe[64];
    char enemy_player_coordinates[32];
    char robs_lost_manpage[64];
    int  local_variable;
};

int main(void) {
    // These local variables are allocated on the stack frame for main().
    struct Data sf;

    static int user_guess;

    printf("--- Stack Variable Addresses ---\n\n");
    printf("vulnerable_buffer[64]         @ %p\n", (void *)&sf.vulnerable_buffer);
    printf("shared_netflix_password[16]   @ %p\n", (void *)&sf.shared_netflix_password);
    printf("yans_borscht_recipe[64]       @ %p\n", (void *)&sf.yans_borscht_recipe);
    printf("enemy_player_coordinates[32]  @ %p\n", (void *)&sf.enemy_player_coordinates);
    printf("robs_lost_manpage[64]         @ %p\n", (void *)&sf.robs_lost_manpage);
    printf("local_variable (int)          @ %p\n", (void *)&sf.local_variable);
    printf("\n------------------------------------\n");


    long correct_offset =  (long)&sf.local_variable - (long)&sf.vulnerable_buffer;

    printf("[?] Challenge: What is the offset (in bytes) from the start of\n");
    printf("               'vulnerable_buffer' to 'local_variable'?\n");
    printf("Enter your guess: ");

    scanf("%d", &user_guess);

    if (user_guess == correct_offset) {
        FILE *f = fopen("/flag", "r");
        char* tempbuf = (char*)malloc(64);
        size_t bytes_read = fread(tempbuf, 1, 64, f);
        fwrite(tempbuf, 1, bytes_read, stdout);
        fclose(f);
        free(tempbuf);
    } else {
        printf("\nIncorrect.\n");
    }

    return 0;
}