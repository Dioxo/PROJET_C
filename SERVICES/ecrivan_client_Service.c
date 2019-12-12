/* NOMS ET PRENOMS : HAYRAULT JULIEN - MOLINARES VALENCIA DIOGENES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "client_service.h"


int main(int argc, char const *argv[]) {
  Pair service;

  service = serviceOpenPipes("S_C_1", "C_S_1");

  char *chaine = "test";
  printf("sending chaine \n");
  serviceWriteData(&service, chaine, 5);
  printf("chaine envoyé\n");
  serviceClosePipes(&service);
  return 0;
}
