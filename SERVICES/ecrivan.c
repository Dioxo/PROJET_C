#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  if (argc != 4)
    return -1;

  int fd = open("c_s", O_WRONLY);
  float a, b;
  a = atof(argv[1]);
  b = atof(argv[2]);
  int mdp = atoi(argv[3]);
  write(fd, &mdp, sizeof(int));
  write(fd, &a, sizeof(int));
  write(fd, &b, sizeof(int));
  return 0;
}
