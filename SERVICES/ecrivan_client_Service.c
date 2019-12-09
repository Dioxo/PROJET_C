#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "client_service.h"


int main(int argc, char const *argv[]) {
  Pair service;
  createPipes(1 , &service);
  createPipes(1 , &service);

  serviceOpenPipes(&service);

  char *chaine = "test";
  printf("sending chaine \n");
  serviceWriteData(&service, chaine, 5);
  printf("chaine envoyé\n");

  sleep(5);
  destroyPipes(&service);
  return 0;
}
