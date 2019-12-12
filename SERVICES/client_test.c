/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "client_service.h"


int main(int argc, char const *argv[]) {
  Pair client;
  client = clientOpenPipes("S_C_1", "C_S_1");

  char *chaine = malloc(20);
  clientReadData(&client, chaine, 5);

  printf("%s\n",chaine);
  clientClosePipes(&client);
  return 0;
}
