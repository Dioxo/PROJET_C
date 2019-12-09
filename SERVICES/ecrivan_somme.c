#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>


#include "client_service.h"

//FICHIER DE TEST======================================================
int main(int argc, char *argv[]) {
  if (argc != 4)
    return -1;

  Pair client;
  clientOpenPipes("S_C_1","C_S_1", &client);



  float a, b;
  a = atof(argv[1]);
  b = atof(argv[2]);
  int mdp = atoi(argv[3]);




  clientWriteData(&client, &mdp, sizeof(int));

  int code;
  printf("reading code service\n");
  clientReadData(&client, &code, sizeof(int));
  printf("code = %d\n", code);


  clientWriteData(&client, &a, sizeof(float));
  clientWriteData(&client, &b, sizeof(float));

  float res;
  printf("reading result\n");
  clientReadData(&client, &res, sizeof(float));
  printf("result = %f\n", res);

  code = -1;
  clientWriteData(&client, &code, sizeof(int));

  clientClosePipes(&client);
  return 0;
}
