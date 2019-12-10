#include "myassert.h"

#include "service_orchestre.h"

void orchestreWrite(AnonymeTube *anonyme, const void *buf, size_t size){
  write(anonyme->fd[1], buf , size);
}
void serviceRead(AnonymeTube *anonyme, void *buf, size_t size){
  read(anonyme->fd[0], buf , size);
}
