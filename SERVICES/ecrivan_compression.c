#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "client_service.h"


//FICHIER DE TEST======================================================
int main(int argc, char *argv[]) {
  if (argc != 4)
    return -1;

  Pair client;
  clientOpenPipes("S_C_1","C_S_1", &client);

  int taille, mdp;
  mdp = atoi(argv[1]);
  taille = atoi(argv[2]) + 1;
  clientWriteData(&client, &mdp, sizeof(int));

  int code;
  printf("reading code service\n");
  clientReadData(&client, &code, sizeof(int));
  printf("code = %d\n", code);


  clientWriteData(&client, &taille, sizeof(int));
  clientWriteData(&client, argv[3], taille);

  //lire result
  clientReadData(&client, &taille, sizeof(int));
  char *res = malloc(taille);
  clientReadData(&client, res, taille);
  printf("%s\n", res);

  code = -1;
  clientWriteData(&client, &code, sizeof(int));
  clientClosePipes(&client);
  return 0;
}
