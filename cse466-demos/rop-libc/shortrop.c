#include <stdio.h>
#include <unistd.h>

int main(void)
{
    char buf[0x100];

    setvbuf(stdout, NULL, _IONBF, 0);
    puts("overflow me");
    read(0, buf, 0x128);

    return 0;
}
