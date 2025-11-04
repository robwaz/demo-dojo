#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>


volatile int tickets = 100000;

void *handle_client(void *arg) {
    int fd = *(int*)arg;
    free(arg);
    char buf[64];

    while (1) {
        ssize_t r = recv(fd, buf, sizeof(buf)-1, 0);
        if (r <= 0) break;
        buf[r] = '\0';
        
        int seen = tickets;

        int newval = seen - 1;

        tickets = newval;

        int n = snprintf(buf, sizeof(buf), "OK %d\n", seen);
        send(fd, buf, n, 0);

        if (seen <= 0) {
            printf("[*] sold ticket id 0 — exiting now\n");
            fflush(stdout);
            exit(0);
        }
    }

    close(fd);
    return NULL;
}

int main(int argc, char **argv) {
    int port = 31337;

    //networking junk
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = INADDR_ANY;
    srv.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&srv, sizeof(srv)) < 0) { 
        return 1; 
    }
    if (listen(listen_fd, 128) < 0) { 
        return 1; 
    }

    printf("listening on port %d with %d tickets\n", port, tickets);
    fflush(stdout);

    // accept connections in a loop
    while (1) {
        struct sockaddr_in client;
        socklen_t clisize = sizeof(client);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(listen_fd, (struct sockaddr*)&client, &clisize);

        if (*client_fd < 0) { 
            free(client_fd); 
            continue; 
        }

        pthread_t t;
        pthread_create(&t, NULL, handle_client, client_fd);
        pthread_detach(t);
    }

    close(listen_fd);
    return 0;
}