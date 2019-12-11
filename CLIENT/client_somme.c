#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "client_service.h"
#include "client_somme.h"

#include "myassert.h"

/*----------------------------------------------*
 * fonctions appelables par le client
 *----------------------------------------------*/

// fonction d'envoi des données du client au service
// Les paramètres sont
// - les tubes de communication
// - argc et argv fournis au main
// Cette fonction analyse argv et en déduit les données à envoyer




void client_somme_sendData(Pair *pipes, int argc, char * argv[])
{


	myassert(argc == 3,"nombre paramètres incorrect");
	int a = atoi(argv[0]);
	int b = atoi(argv[1]);
	printf("%d %d\n",a,b);
	clientWriteData(pipes, &a, sizeof(int));
	clientWriteData(pipes, &b, sizeof(int));

    // par exemple argv[2] et argv[3] contiennent les deux nombres
    // à envoyer (pour les sommer)
}

// fonction de réception des résultats en provenance du service
// Les paramètres sont
// - les tubes de communication
// - argc et argv fournis au main
// Cette fonction analyse argv pour savoir quoi faire des résultats
void client_somme_receiveResult(Pair *pipes, int argc, char * argv[])
{
	myassert(argc == 3,"nombre paramètres incorrect");
	int sum;
	clientReadData(pipes, &sum, sizeof(int));
	printf("%s %d\n", argv[3], sum);
}
    // par exemple on décide d'afficher le résultat et argv[4] contient
    // une chaine à afficher avant le résultat

/*
int main() 
{

  return EXIT_SUCCESS;
}
*/