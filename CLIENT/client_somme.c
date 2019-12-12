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
	float a = atof(argv[0]);
	float b = atof(argv[1]);
	printf("%f %f\n",a,b);
	clientWriteData(pipes, &a, sizeof(float));
	clientWriteData(pipes, &b, sizeof(float));

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
	float sum;
	clientReadData(pipes, &sum, sizeof(float));
	printf("%s %f\n", argv[2], sum);
}
    // par exemple on décide d'afficher le résultat et argv[4] contient
    // une chaine à afficher avant le résultat

/*
int main() 
{

  return EXIT_SUCCESS;
}
*/
