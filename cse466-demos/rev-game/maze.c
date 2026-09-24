/*
 * level_viewer.c - toy level loader for the assembler/disassembler demo.
 *
 * Format:
 *   header:  "LVL" | count (u8)
 *   records: tag (u8) | x (u8) | y (u8)      -- count records, 3 bytes each
 *   tags:    1 = wall, 2 = spawn, 3 = exit, 7 = treasure (no built-in level uses it)
 *
 * Usage: ./level_viewer             play the built-in levels
 *        ./level_viewer my.lvl      play a custom level
 *
 * Move with w/a/s/d. Walking onto a treasure prints the flag.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define W 16
#define H 8
#define MAX_FILE (4 + 255 * 3)

enum { TILE_WALL = 1, TILE_SPAWN = 2, TILE_EXIT = 3, TILE_TREASURE = 7 };

/* Built-in levels. sizeof - 1 drops the string's trailing NUL. */
static const char level1[] =
    "\x4c\x56\x4c\x09\x02\x01\x03\x03\x0e\x03\x01\x07\x00\x01\x07\x01"
    "\x01\x07\x02\x01\x07\x03\x01\x07\x04\x01\x07\x06\x01\x07\x07";
static const char level2[] =
    "\x4c\x56\x4c\x43\x01\x00\x00\x01\x01\x00\x01\x02\x00\x01\x03\x00"
    "\x01\x04\x00\x01\x05\x00\x01\x06\x00\x01\x07\x00\x01\x08\x00\x01"
    "\x09\x00\x01\x0a\x00\x01\x0b\x00\x01\x0c\x00\x01\x0d\x00\x01\x0e"
    "\x00\x01\x0f\x00\x01\x00\x01\x02\x01\x01\x01\x06\x01\x01\x0f\x01"
    "\x01\x00\x02\x01\x02\x02\x01\x03\x02\x01\x04\x02\x01\x06\x02\x01"
    "\x08\x02\x01\x09\x02\x01\x0a\x02\x01\x0b\x02\x01\x0c\x02\x01\x0f"
    "\x02\x01\x00\x03\x01\x04\x03\x01\x08\x03\x01\x0c\x03\x01\x0f\x03"
    "\x01\x00\x04\x01\x01\x04\x01\x02\x04\x01\x04\x04\x01\x05\x04\x01"
    "\x06\x04\x01\x07\x04\x01\x08\x04\x01\x0a\x04\x01\x0c\x04\x01\x0f"
    "\x04\x01\x00\x05\x01\x0a\x05\x03\x0e\x05\x01\x0f\x05\x01\x00\x06"
    "\x01\x02\x06\x01\x03\x06\x01\x04\x06\x01\x05\x06\x01\x06\x06\x01"
    "\x07\x06\x01\x08\x06\x01\x09\x06\x01\x0a\x06\x01\x0c\x06\x01\x0d"
    "\x06\x01\x0e\x06\x01\x0f\x06\x01\x00\x07\x01\x0f\x07";

static void print_flag(void)
{
    char buf[256];
    FILE *f = fopen("/flag", "r");
    if (!f || !fgets(buf, sizeof buf, f)) {
        puts("(couldn't read /flag)");
        if (f)
            fclose(f);
        return;
    }
    fclose(f);
    printf("You found the treasure: %s", buf);
}

struct level {
    char grid[H][W];
    int px, py;              /* player position */
};

static void draw_border(void)
{
    putchar('+');
    for (int x = 0; x < W; x++)
        putchar('-');
    puts("+");
}

static void draw(const struct level *lv)
{
    draw_border();
    for (int y = 0; y < H; y++) {
        putchar('|');
        for (int x = 0; x < W; x++)
            putchar(x == lv->px && y == lv->py ? '@' : lv->grid[y][x]);
        puts("|");
    }
    draw_border();
}

static int load_level(struct level *lv, const uint8_t *data, size_t len)
{
    if (len < 4 || memcmp(data, "LVL", 3) != 0) {
        puts("Not a level file.");
        return 1;
    }
    uint8_t count = data[3];
    if (len < 4 + (size_t)count * 3) {
        puts("Level is truncated.");
        return 1;
    }

    memset(lv->grid, '.', sizeof lv->grid);
    int spawns = 0;

    const uint8_t *rec = data + 4;
    for (int i = 0; i < count; i++) {
        uint8_t tag = rec[0];
        uint8_t x   = rec[1];
        uint8_t y   = rec[2];
        rec += 3;

        if (x >= W || y >= H) {
            printf("Record %d is off the map.\n", i);
            return 1;
        }

        if (tag == TILE_WALL) {
            lv->grid[y][x] = '#';
        } else if (tag == TILE_SPAWN) {
            lv->px = x;
            lv->py = y;
            spawns++;
        } else if (tag == TILE_EXIT) {
            lv->grid[y][x] = 'E';
        } else if (tag == TILE_TREASURE) {
            lv->grid[y][x] = '$';
        } else {
            printf("Record %d has unknown type %u.\n", i, tag);
            return 1;
        }
    }
    if (spawns != 1) {
        puts("A level needs exactly one spawn point.");
        return 1;
    }
    return 0;
}

/* Returns 0 if the player reached the exit, 1 if they quit. */
static int play(struct level *lv)
{
    char line[256];
    for (;;) {
        draw(lv);
        printf("Move (w/a/s/d, several per line ok, q quits): ");
        if (!fgets(line, sizeof line, stdin))
            return 1;

        for (char *c = line; *c; c++) {
            int nx = lv->px, ny = lv->py;
            switch (*c) {
            case 'w': ny--; break;
            case 's': ny++; break;
            case 'a': nx--; break;
            case 'd': nx++; break;
            case 'q': return 1;
            default:  continue;
            }
            if (nx < 0 || nx >= W || ny < 0 || ny >= H || lv->grid[ny][nx] == '#')
                continue;
            lv->px = nx;
            lv->py = ny;

            if (lv->grid[ny][nx] == '$') {
                draw(lv);
                print_flag();
                exit(0);
            }
            if (lv->grid[ny][nx] == 'E') {
                draw(lv);
                puts("You reached the exit!");
                return 0;
            }
        }
    }
}

int main(int argc, char **argv)
{
    struct level lv;

    if (argc < 2) {
        char choice[16];
        printf("Pick a level (1 or 2): ");
        if (!fgets(choice, sizeof choice, stdin))
            return 1;

        const uint8_t *data;
        size_t len;
        if (choice[0] == '1') {
            data = (const uint8_t *)level1;
            len = sizeof level1 - 1;
        } else if (choice[0] == '2') {
            data = (const uint8_t *)level2;
            len = sizeof level2 - 1;
        } else {
            puts("No such level.");
            return 1;
        }
        if (load_level(&lv, data, len))
            return 1;
        if (play(&lv) == 0)
            puts("...but the legends speak of treasure.");
        return 0;
    }

    uint8_t buf[MAX_FILE];
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("open");
        return 1;
    }
    size_t len = fread(buf, 1, sizeof buf, f);
    fclose(f);
    if (load_level(&lv, buf, len))
        return 1;
    play(&lv);
    return 0;
}