#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

void win()
{
    static char flag[256];
    static int flag_fd;
    static int flag_length;

    puts("You win! Here is your flag:");
    flag_fd = open("/flag", 0);
    if (flag_fd < 0)
    {
        printf("\n  ERROR: Failed to open the flag -- %s!\n", strerror(errno));
        if (geteuid() != 0)
        {
            printf("  Your effective user id is not 0!\n");
            printf("  You must directly run the suid binary in order to have the correct permissions!\n");
        }
        exit(-1);
    }
    flag_length = read(flag_fd, flag, sizeof(flag));
    if (flag_length <= 0)
    {
        printf("\n  ERROR: Failed to read the flag -- %s!\n", strerror(errno));
        exit(-1);
    }
    write(1, flag, flag_length);
    printf("\n\n");
}

char *msg = "HELLO HACKERS!";

void use_it(char *slot) {
    printf("using it..\n");
    read(0, slot, 8);
}

void vuln() {
    struct {
        char buf[64];
        int recurse;
        char *local;
    } s;

    s.recurse = 0;
    s.local = msg;

    read(0, s.buf, 0x100);
    printf("%s", s.buf);

    if (s.recurse >= 1) {
        vuln();
    } else {
        use_it(s.local);
        putchar('X');
    }
}


int main() {
    setbuf(stdout, NULL);
    vuln();
    return 0;
}
