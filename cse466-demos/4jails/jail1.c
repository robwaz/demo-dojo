#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static void make_jail(void)
{
    char jail[] = "/tmp/jail-XXXXXX";

    // create the jail directory
    if (mkdtemp(jail) == NULL)
    {
        perror("mkdtemp");
        exit(1);
    }

    // make the jail directory this process's "/"
    if (chroot(jail) != 0)
    {
        perror("chroot (is this binary running as root?)");
        exit(1);
    }

    // "/flag" now resolves inside the jail
    int fd = open("/flag", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("creation of decoy flag failed");
        exit(1);
    }

    // plant the decoy flag
    dprintf(fd, "FLAG{decoy -- you are still inside the jail}\n");
    close(fd);
}

int main(int argc, char **argv)
{
    // need exactly one argument: the path to read
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <path>\n", argv[0]);
        return 2;
    }

    // build the jail and chroot into it
    make_jail();

    // now we are jailed!

    // open the requested path
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror(argv[1]);
        return 1;
    }

    char buf[4096];
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n < 0)
    {
        perror("read");
        return 1;
    }

    if (write(1, buf, n) < 0)
    {
        perror("write");
        return 1;
    }

    close(fd);
    return 0;
}