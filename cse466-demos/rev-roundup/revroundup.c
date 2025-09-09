#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

uint8_t SECRET[256] = {
    0x3a, 0x7f, 0x1c, 0x88, 0x5b, 0xd9, 0x02, 0x46, 0x9e, 0x14, 0xaf, 0x63,
    0xc2, 0x55, 0x7a, 0xe1, 0x90, 0x0d, 0x77, 0x22, 0xbc, 0x6e, 0x49, 0xfd,
    0x31, 0x5f, 0x0a, 0xcb, 0x84, 0x19, 0x6a, 0x52, 0x27, 0x8c, 0xf4, 0x38,
    0xa2, 0xde, 0x11, 0x70, 0x4f, 0x9a, 0x06, 0xbd, 0x23, 0x81, 0xe7, 0x5c,
    0x13, 0xca, 0x72, 0x40, 0xfa, 0x8d, 0x29, 0x64, 0xbe, 0x35, 0x01, 0xac,
    0x59, 0x7d, 0x92, 0xf8, 0x6b, 0x0e, 0xd3, 0x41, 0x9c, 0x25, 0x87, 0xfe,
    0x4a, 0xb1, 0x5e, 0x03, 0x6f, 0xc4, 0x18, 0xa9, 0x73, 0xdd, 0x2b, 0x08,
    0xf1, 0x94, 0x60, 0x37, 0x21, 0xe2, 0x4d, 0x7b, 0x15, 0x98, 0xcf, 0x56,
    0x83, 0xba, 0x09, 0x3f, 0xd7, 0x65, 0x12, 0xa4, 0x5a, 0xc8, 0x36, 0xeb,
    0x04, 0x7c, 0x9f, 0x20, 0x6d, 0xf5, 0x0b, 0xae, 0x48, 0x93, 0xd0, 0x17,
    0x7e, 0x26, 0xb7, 0x54, 0x01, 0xca, 0x69, 0xf2, 0x3c, 0x85, 0xdb, 0x42,
    0xa6, 0x0f, 0x71, 0xec, 0x57, 0x99, 0x34, 0xd8, 0x2c, 0x63, 0x07, 0xbe,
    0xf0, 0x45, 0x8a, 0x12, 0x6c, 0xad, 0x79, 0x25, 0xcb, 0x58, 0x9d, 0x04,
    0xe9, 0x1f, 0x80, 0x36, 0xaf, 0x62, 0xdc, 0x28, 0x5b, 0x94, 0x0c, 0x77,
    0x1a, 0xe5, 0x50, 0xbd, 0x3e, 0x08, 0xf6, 0x61, 0x93, 0x4c, 0xa0, 0x2d,
    0x7f, 0x15, 0xca, 0x39, 0xd2, 0x86, 0x1e, 0xb9, 0x43, 0x64, 0x0a, 0xfd,
    0x57, 0x20, 0xce, 0x88, 0x34, 0xab, 0x1b, 0x96, 0x72, 0x0f, 0xe8, 0x59,
    0xbc, 0x27, 0x04, 0xd5, 0x6a, 0x91, 0xf3, 0x3d, 0x12, 0xc1, 0x4e, 0x80,
    0x25, 0x7a, 0xdf, 0x19, 0xb0, 0x46, 0x6f, 0x03, 0xad, 0x5c, 0xe4, 0x10,
    0x97, 0x2f, 0x68, 0xcb, 0x3b, 0xf9, 0x84, 0x1d, 0x55, 0xa8, 0x02, 0x76,
    0xdd, 0x44, 0x9b, 0x0e, 0xc7, 0x21, 0xfa, 0x60,
};

struct Node {
  int id;
  char data[32];
  struct Node *left;
  struct Node *right;
};

struct Node *create_node(int id, char *data) {
  struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
  newNode->id = id;
  strcpy(newNode->data, data);
  newNode->left = newNode->right = NULL;
  return newNode;
}

struct Node *insert_node(struct Node *root, struct Node *newNode) {
  if (root == NULL) {
    return newNode;
  }
  if (newNode->id < root->id) {
    root->left = insert_node(root->left, newNode);
  } else {
    root->right = insert_node(root->right, newNode);
  }
  return root;
}

void print_in_order(struct Node *root) {
  if (root != NULL) {
    print_in_order(root->left);
    printf("(%d, %s) ", root->id, root->data);
    print_in_order(root->right);
  }
}

// lol
int check_perfect(struct Node *root) {
  if (root && root->left && root->right && root->left->left &&
      root->left->right && root->right->left && root->right->right &&
      !root->left->left->left && !root->left->left->right &&
      !root->left->right->left && !root->left->right->right &&
      !root->right->left->left && !root->right->left->right &&
      !root->right->right->left && !root->right->right->right) {
    return 1;
  }
  return 0;
}

int rev_datastructure() {
  struct Node *root = NULL;
  int choice;

  do {
    puts("\n===== MENU =====\n1. Add\n2. View\n3. Check\n0. Exit");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
      int id;
      char data[32];
      printf("Enter ID: ");
      scanf("%d", &id);
      printf("Enter data: ");
      scanf(" %31s", data);
      struct Node *newNode = create_node(id, data);
      root = insert_node(root, newNode);
      puts("Node added.");
    } else if (choice == 2) {
      print_in_order(root);
      puts("");
    } else if (choice == 3) {
      if (check_perfect(root)) {
        puts("NICE!");
        return 1;
      }
    }
  } while (choice != 0);
  return 0;
}

int rev_transform() {
  uint8_t target[8] = {'i', 'd', 'a', '4', 'e', 'v', 'e', 'r'};
  puts("input data: ");
  uint8_t buf[8];
  ssize_t n = read(0, buf, 8);

  for (ssize_t i = 0; i < n; i++) {
    uint8_t t = (uint8_t)(buf[i] * 197);
    t = (t + 113) ^ ((t & 0xF) << 2);
    t = (t * 29) % 256;
    buf[i] = t ^ 0xA7;
  }
  puts("response: ");
  write(1, buf, n);
  puts("\n");

  if (memcmp(buf, target, 8) == 0) {
    puts("NICE!\n");
    return 1;
  } else {
    return 0;
  }
}

int rev_dumpdata() {
  unsigned char input[256];
  read(STDIN_FILENO, input, 256);

  if (memcmp(input, SECRET, 256) == 0) {
    puts("NICE!\n");
    return 1;
  } else {
    return 0;
  }
}

int make_listener(int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  listen(sock, 1);
  return sock;
}

int rev_connectionjuggle() {
  int sock1 = make_listener(1337);
  int sock2 = make_listener(13372);

  puts("Waiting for connections..");
  int con1 = accept(sock1, NULL, NULL);
  puts("Connection 1 recieved!");
  int con2 = accept(sock2, NULL, NULL);
  puts("Connection 2 recieved!");

  char req[64];
  int i = 0, n1, n2;

  while (1) {
    if (read(con1, &req[i++], 1) <= 0)
      break;
    if (read(con2, &req[i++], 1) <= 0)
      break;
    if (i >= sizeof(req) - 1)
      break;
    if (req[i - 2] == '\n' || req[i - 1] == '\n')
      break;
  }
  req[i] = '\0';

  printf("Assembled request: %s\n", req);

  close(con1);
  close(con2);
  close(sock1);
  close(sock2);
  if (strncmp(req, "GET / HTTP/1.1", 14) == 0) {
    puts("NICE!\n");
    return 1;
  }
  return 0;
}

__attribute__((noinline))
uint16_t mysterious_operation(const uint8_t *p) {
    uint16_t val;
    memcpy(&val, p, sizeof(val));
    return val;
}

int rev_decompilation() {
  uint8_t d[2] = {0x24,0x68};
  uint16_t res = mysterious_operation(d);

  unsigned int guess;
  puts("predict the mystery... ");
  if (scanf("%x", &guess) != 1) {
        printf("invalid input\n");
        return 0;
    }
    if (guess == res) {
      return 1;
    } else {
      return 0;
    }
}

void win() {
  char flag[128];
  int flag_fd = open("/flag", 0);
  if (flag_fd < 0) {
    printf("\n  ERROR: Failed to open the flag -- %s!\n", strerror(errno));
    if (geteuid() != 0) {
      printf("  Your effective user id is not 0!\n");
      printf("  You must directly run the suid binary in order to have the "
             "correct permissions!\n");
    }
    exit(-1);
  }
  int flag_length = read(flag_fd, flag, sizeof(flag));
  if (flag_length <= 0) {
    printf("\n  ERROR: Failed to read the flag -- %s!\n", strerror(errno));
    exit(-1);
  }
  write(1, flag, flag_length);
  printf("\n\n");
}

int main() {
  setbuf(stdout, NULL);
  setbuf(stdin, NULL);
  int passed[5] = {0, 0, 0, 0, 0};
  int choice;

  while (!(passed[0] && passed[1] && passed[2] && passed[3] & passed[4])) {
    printf("\n--- Menu ---\n");
    printf("1. Data Dumping [%s]\n", passed[0] ? "PASSED" : "NOT PASSED");
    printf("2. Transformation Terror [%s]\n",
           passed[1] ? "PASSED" : "NOT PASSED");
    printf("3. Structure Shenanigans [%s]\n",
           passed[2] ? "PASSED" : "NOT PASSED");
    printf("4. Communication Conundrum [%s]\n",
           passed[3] ? "PASSED" : "NOT PASSED");
    printf("5. Misleading Manifestation [%s]\n",
           passed[4] ? "PASSED" : "NOT PASSED");
    printf("Choose: ");
    scanf("%d", &choice);

    if (choice == 1)
      passed[0] = rev_dumpdata();
    if (choice == 2)
      passed[1] = rev_transform();
    if (choice == 3)
      passed[2] = rev_datastructure();
    if (choice == 4)
      passed[3] = rev_connectionjuggle();
    if (choice == 5)
      passed[4] = rev_decompilation();
  }

  printf("NICE!\n");
  win();
  return 0;
}
