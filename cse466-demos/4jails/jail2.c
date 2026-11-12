#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * ACT 2a: jail1's missing chdir() is fixed, so relative paths no longer escape.
 *
 * This jail is a little file server. Like every real sandboxed worker, it is
 * handed the descriptors it needs BEFORE it is locked down -- here, a data
 * directory. It then answers requests two ways: against its own root, and
 * against that handle.
 *
 * Intentionally vulnerable teaching code, setuid root so that chroot() works.
 * Only makes sense inside a disposable pwn.college container.
 *
 *     jail2 /etc flag      ->  both lookups contained
 *     jail2 /    flag      ->  ?
 *
 * The program does not change between those two runs. Only the breadth of the
 * descriptor it was handed changes.
 */

#define NAME  "jail2"
#define DECOY "FLAG{decoy -- you are still inside the jail}\n"

static void die(const char *what)
{
    fprintf(stderr, "[" NAME "] %s: %s\n", what, strerror(errno));
    exit(1);
}

static void make_jail(void)
{
    char jail_path[] = "/tmp/jail-XXXXXX";
    if (mkdtemp(jail_path) == NULL) die("mkdtemp");

    if (chroot(jail_path) != 0) die("chroot (is this binary setuid root?)");
    fprintf(stderr, "[" NAME "] chroot(\"%s\")\n", jail_path);

    if (chdir("/") != 0) die("chdir");
    fprintf(stderr, "[" NAME "] chdir(\"/\") -- jail1's hole is closed\n");

    int fd = open("/flag", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) die("create decoy");
    if (write(fd, DECOY, strlen(DECOY)) < 0) die("write decoy");
    close(fd);
    fprintf(stderr, "[" NAME "] put a decoy at /flag inside the jail\n");
}

static void dump(int fd)
{
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        if (write(1, buf, n) < 0) die("write");
}

/* Resolve the request two ways and show both answers. */
static void serve(int datafd, const char *name)
{
    fprintf(stderr, "\n[" NAME "] lookup A: open(\"%s\") -- from our own root\n", name);
    int a = open(name, O_RDONLY);
    if (a < 0)
        fprintf(stderr, "[" NAME "]   -> %s\n", strerror(errno));
    else { dump(a); close(a); }

    fprintf(stderr, "\n[" NAME "] lookup B: openat(datafd, \"%s\") -- from the handle we were given\n", name);
    int b = openat(datafd, name, O_RDONLY);
    if (b < 0)
        fprintf(stderr, "[" NAME "]   -> %s\n", strerror(errno));
    else { dump(b); close(b); }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <data-directory> <name>\n", argv[0]);
        return 2;
    }

    // Note WHEN this happens: before the jail exists. chroot() will not take
    // this descriptor away, and openat() will happily resolve against it.
    int datafd = open(argv[1], O_RDONLY | O_DIRECTORY);
    if (datafd < 0) die(argv[1]);
    fprintf(stderr, "[" NAME "] handed a data directory `%s` as FD %d, before jailing\n",
            argv[1], datafd);

    make_jail();
    serve(datafd, argv[2]);
    return 0;
}
