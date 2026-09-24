#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_ITEMS     8
#define START_GOLD    100
#define START_HP      100
#define BOSS_HP       50000
#define BOSS_ATTACK   15
#define POTION_THRESH 40
#define CHEST_KEY_ID  0xCAFE

enum item_type { ITEM_WEAPON = 0, ITEM_POTION = 1, ITEM_KEY = 2 };

struct item {
    char     name[16];
    char     desc[32];
    uint8_t  qty;
    uint32_t price;
    uint8_t  type;
    int32_t  value;
};

struct player {
    char        name[16];
    uint8_t     inv_count;
    int8_t      equipped;
    int32_t     hp;
    int32_t     max_hp;
    uint32_t    gold;
    struct item inventory[MAX_ITEMS];
};

static const struct item catalog[] = {
    { "Rusty Dagger",   "Better than nothing.",           1, 10, ITEM_WEAPON, 4 },
    { "Iron Sword",     "Reliable and sharp.",            1, 35, ITEM_WEAPON, 11 },
    { "War Axe",        "Heavy. Hits hard.",              1, 60, ITEM_WEAPON, 18 },
    { "Minor Potion",   "Restores a little health.",      1,  8, ITEM_POTION, 20 },
    { "Greater Potion", "Restores a lot of health.",      1, 20, ITEM_POTION, 50 },
    { "Chest Key",      "Opens the dragon's loot chest!", 1, 45, ITEM_KEY, CHEST_KEY_ID },
};
#define CATALOG_SIZE ((int)(sizeof catalog / sizeof catalog[0]))

static int read_int(void)
{
    char buf[32];
    if (!fgets(buf, sizeof buf, stdin))
        exit(0);
    return (int)strtol(buf, NULL, 10);
}

static void print_inventory(struct player *p)
{
    if (p->inv_count == 0) {
        puts("  (empty)");
        return;
    }
    for (int i = 0; i < p->inv_count; i++)
        printf("  %d) %-16s x%u\n", i + 1, p->inventory[i].name,
               p->inventory[i].qty);
}

static void remove_item(struct player *p, int idx)
{
    for (int i = idx; i < p->inv_count - 1; i++)
        p->inventory[i] = p->inventory[i + 1];
    p->inv_count--;
    memset(&p->inventory[p->inv_count], 0, sizeof(struct item));

    if (p->equipped >= p->inv_count)
        p->equipped = -1;
}

static void shop_buy(struct player *p)
{
    puts("\n-- Wares --");
    for (int i = 0; i < CATALOG_SIZE; i++) {
        const struct item *it = &catalog[i];
        printf("  %d) %-16s %3u gold  %s", i + 1, it->name, it->price, it->desc);
        if (it->type == ITEM_WEAPON)
            printf(" (dmg %d)", it->value);
        else if (it->type == ITEM_POTION)
            printf(" (heals %d)", it->value);
        putchar('\n');
    }
    printf("Gold: %u. Buy which? (0 to cancel) ", p->gold);

    int c = read_int();
    if (c < 1 || c > CATALOG_SIZE)
        return;
    const struct item *want = &catalog[c - 1];

    if (want->price > p->gold) {
        puts("You can't afford that.");
        return;
    }
    for (int i = 0; i < p->inv_count; i++) {
        if (strcmp(p->inventory[i].name, want->name) == 0) {
            if (p->inventory[i].qty >= 99) {
                puts("You can't carry any more of those.");
                return;
            }
            p->inventory[i].qty++;
            p->gold -= want->price;
            printf("Bought another %s.\n", want->name);
            return;
        }
    }
    if (p->inv_count >= MAX_ITEMS) {
        puts("Your bag is full.");
        return;
    }
    p->inventory[p->inv_count++] = *want;
    p->gold -= want->price;
    printf("Bought %s.\n", want->name);
}

static void shop_sell(struct player *p)
{
    puts("\n-- Your bag --");
    print_inventory(p);
    printf("Sell which? (0 to cancel) ");

    int s = read_int() - 1;
    if (s < 0 || s >= p->inv_count)
        return;

    struct item *it = &p->inventory[s];
    uint32_t refund = it->price / 2;
    p->gold += refund;
    printf("Sold %s for %u gold.\n", it->name, refund);

    if (--it->qty == 0)
        remove_item(p, s);
}

static void shop(struct player *p)
{
    for (;;) {
        printf("\n== Shop ==  Gold: %u\n1) Buy\n2) Sell\n3) Leave\n> ", p->gold);
        switch (read_int()) {
        case 1: shop_buy(p);  break;
        case 2: shop_sell(p); break;
        case 3: return;
        default: puts("The shopkeeper stares blankly.");
        }
    }
}

static void equip(struct player *p)
{
    puts("\n-- Your bag --");
    print_inventory(p);
    printf("Equip which? (0 to cancel) ");

    int e = read_int() - 1;
    if (e < 0 || e >= p->inv_count)
        return;
    if (p->inventory[e].type != ITEM_WEAPON) {
        puts("You can't wield that.");
        return;
    }
    p->equipped = (int8_t)e;
    printf("You equip the %s.\n", p->inventory[e].name);
}

static void drink_potion(struct player *p)
{
    for (int i = 0; i < p->inv_count; i++) {
        struct item *it = &p->inventory[i];
        if (it->type != ITEM_POTION)
            continue;
        p->hp += it->value;
        if (p->hp > p->max_hp)
            p->hp = p->max_hp;
        printf("          You drink a %s. Your HP: %d\n", it->name, p->hp);
        if (--it->qty == 0)
            remove_item(p, i);
        return;
    }
}

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
    printf("Inside the chest you find: %s", buf);
}

static void open_chest(struct player *p)
{
    puts("\nThe dragon collapses. Behind it sits an iron-banded loot chest.");
    for (int i = 0; i < p->inv_count; i++) {
        struct item *it = &p->inventory[i];
        if (it->type == ITEM_KEY && it->value == CHEST_KEY_ID) {
            puts("Your key turns in the lock...");
            print_flag();
            return;
        }
    }
    puts("It's locked. If only you had a key...");
}

static void fight(struct player *p)
{
    int32_t dmg = 1;
    const char *wname = "bare fists";
    if (p->equipped >= 0) {
        dmg = p->inventory[p->equipped].value;
        wname = p->inventory[p->equipped].name;
    }

    int32_t boss_hp = BOSS_HP;
    printf("\nThe dragon awakens! (HP %d)\nYou raise your %s.\n", boss_hp, wname);

    for (int round = 1;; round++) {
        boss_hp -= dmg;
        printf("[round %d] You hit for %d. Dragon HP: %d\n",
               round, dmg, boss_hp > 0 ? boss_hp : 0);
        if (boss_hp <= 0)
            break;

        p->hp -= BOSS_ATTACK;
        printf("          The dragon hits you for %d. Your HP: %d\n",
               BOSS_ATTACK, p->hp > 0 ? p->hp : 0);
        if (p->hp <= 0) {
            puts("You have been slain.");
            exit(0);
        }
        if (p->hp <= POTION_THRESH)
            drink_potion(p);
    }
    open_chest(p);
    exit(0);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    struct player *p = calloc(1, sizeof *p);
    if (!p)
        return 1;

    p->hp = p->max_hp = START_HP;
    p->gold = START_GOLD;
    p->equipped = -1;

    puts("A dragon guards a chest of treasure. You have 100 gold to prepare.");
    printf("What is your name, adventurer? ");
    if (!fgets(p->name, sizeof p->name, stdin))
        return 0;
    p->name[strcspn(p->name, "\n")] = '\0';
    if (p->name[0] == '\0')
        strcpy(p->name, "Hero");

    for (;;) {
        printf("\n%s | HP %d/%d | Gold %u\n1) Shop\n2) Equip\n3) Fight\n> ",
               p->name, p->hp, p->max_hp, p->gold);
        switch (read_int()) {
        case 1: shop(p);  break;
        case 2: equip(p); break;
        case 3: fight(p); break;
        default: puts("You hesitate.");
        }
    }
}