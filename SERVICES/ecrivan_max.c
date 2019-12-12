/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "client_service.h"


//FICHIER DE TEST======================================================
int main() {
  Pair client;
  clientOpenPipes("S_C_1", "C_S_1", &client);

  int taille = 10;
  float tab[taille];
  for (int i = 0; i < taille; i++) {
    tab[i] = i;
  }

  int mdp = 0;
  clientWriteData(&client,&mdp, sizeof(int));

  int code;
  printf("reading code service\n");
  clientReadData(&client, &code, sizeof(int));
  printf("code = %d\n", code);

  clientWriteData(&client, &taille, sizeof(int));

  for (int i = 0; i < taille; i++)
    clientWriteData(&client, &tab[i], sizeof(float));

  int nbThreads = 3;
  clientWriteData(&client, &nbThreads, sizeof(int));

  //lire result
  float max;
  clientReadData(&client, &max, sizeof(float));
  printf("%f\n", max);

  int codeError = -1;
  clientWriteData(&client, &codeError, sizeof(int));
  clientClosePipes(&client);

  return 0;
}
