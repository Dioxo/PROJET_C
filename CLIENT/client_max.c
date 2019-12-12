#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "client_service.h"
#include "client_compression.h"

#include "myassert.h"

/*----------------------------------------------*
 * fonctions appelables par le client
 *----------------------------------------------*/

// fonction d'envoi des données du client au service
// Les paramètres sont
// - les tubes de communication
// - argc et argv fournis au main
// Cette fonction analyse argv et en déduit les données à envoyer


static int readData(int fd, float *data)
{
	int sz;
    int size = 1;
    while (sz > 0)
    {
        data = (float *)realloc(data, size * sizeof(float));
        sz = read(fd, &data[size-1], sizeof(float));
        size += sz;
    }
    return size-1;
}


void client_max_sendData(Pair *pipes, int argc, char * argv[])
{
	myassert(argc < 3,"nombre paramètres incorrect");
	int fd;

	int nbthread = atoi(argv[1]);
	

	fd = open(argv[2], O_RDONLY | O_CREAT, 0644);
	myassert(fd == -1 ,"ouverture impossible");
	float *data = (float *)malloc(sizeof(float));
	int size = readData(fd, data);
	close(fd);


	

	clientWriteData(pipes, &nbthread, sizeof(int));
	clientWriteData(pipes, &size, sizeof(int));
	for (int i = 0; i<size; i++)
	{
		clientWriteData(pipes, &data[i], sizeof(float));
	}


    // par exemple argv[2] est le nom du fichier contenant la suite
    // de nombres
}

// fonction de réception des résultats en provenance du service
// Les paramètres sont
// - les tubes de communication
// - argc et argv fournis au main
// Cette fonction analyse argv pour savoir quoi faire des résultats
void client_max_receiveResult(Pair *pipes,  int argc, char * argv[])
{
    // par exemple on décide d'afficher le résultat et argv[3] contient
    // une chaine à afficher avant le résultat
    myassert(argc < 3,"nombre paramètres incorrect");
    int max;
    clientReadData(pipes, &max, sizeof(float));
    printf("%s %d\n", argv[1], max);
}
