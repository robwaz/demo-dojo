#include <stdio.h>
#include <unistd.h>
void vuln(void)
{
    char buf[0x100];
    read(0, buf, 0x128);
    puts(buf);
}
 
int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    puts("overflow me");
    vuln();
 
    return 0;
}

