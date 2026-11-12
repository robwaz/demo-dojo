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

    // move the cwd into the new root so relative paths stay inside (jail1's hole)
    if (chdir("/") != 0)
    {
        perror("chdir");
        exit(1);
    }

    // "/flag" now resolves inside the jail
    int fd = open("/flag", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("create decoy");
        exit(1);
    }

    dprintf(fd, "FLAG{decoy -- you are still inside the jail}\n");
    close(fd);
}

int main(int argc, char **argv)
{
    // need exactly one argument: a name to look up
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <name>\n", argv[0]);
        fprintf(stderr, "       (FD 3 is taken to be our data directory, if it is open)\n");
        return 2;
    }

    // open nothing ourselves, just trust whatever FD 3 we inherited
    int datafd = 3;

    // build the jail *and* chdir to its root
    make_jail();

    // buffer for file contents, shared by both lookups
    char buf[4096];

    // open the file with open
    int a = open(argv[1], O_RDONLY);

    if (a < 0)
    {
        perror("open() error");
    }
    else
    {
        ssize_t n = read(a, buf, sizeof(buf));
        if (n > 0)
        {
            write(1, buf, n);
        }
        close(a);
    }

    // use openat on the inherited FD instead
    int b = openat(datafd, argv[1], O_RDONLY);

    if (b < 0)
    {
        perror("openat() error");
    }
    else
    {
        ssize_t n = read(b, buf, sizeof(buf));
        if (n > 0)
        {
            write(1, buf, n);
        }
        close(b);
    }
    return 0;
}