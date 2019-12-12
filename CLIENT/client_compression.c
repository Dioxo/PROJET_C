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


static int readData(int fd, char *str)
{
	int sz;
    int size = 1;
    while (sz > 0)
    {
        str = (char *)realloc(str, size* sizeof(char));
        sz = read(fd, &str[size-1], sizeof(char));
        size += sz;
    }
    str[size] = '\0';
    return size-1;
}


static void writeData(int fd, int size, char *str)
{
	write(fd, &str, size * sizeof(char)); 	
}


void client_compression_sendData(Pair *pipes, int argc, char * argv[])
{
    // par exemple argv[2] est le nom du fichier contenant le texte
    // à envoyer pour compression
    myassert(argc == 2,"nombre paramètres incorrect");
    int fd;
    fd = open(argv[0], O_RDONLY | O_CREAT, 0644);
    myassert(fd != -1 ,"ouverture impossible");
    char *str = (char *)malloc(sizeof(char));
    int size = readData(fd,str);
    close(fd);
    printf("chaine : %s, size : %d\n",str, size);
    clientWriteData(pipes, &size, sizeof(int));
    clientWriteData(pipes, str, size * sizeof(char));
}

// fonction de réception des résultats en provenance du service
// Les paramètres sont
// - les tubes de communication
// - argc et argv fournis au main
// Cette fonction analyse argv pour savoir quoi faire des résultats
void client_compression_receiveResult(Pair *pipes, int argc, char * argv[])
{
	myassert(argc == 2,"nombre paramètres incorrect");
    // par exemple on décide de sauvegarder le résultat dans un fichier et
    // argv[3] est le nom du fichier où écrire le texte compressé
    
    int fd = open(argv[1], O_WRONLY | O_CREAT, 0644);
    myassert(fd != -1 ,"ouverture impossible");

    int size;
    clientReadData(pipes, &size, sizeof(int));

    char *str = (char *) malloc(size * sizeof(char));
    clientReadData(pipes, &str, size * sizeof(char));
   
    writeData(fd, size, str);
    write(fd, &str, size * sizeof(char));
    close(fd);

    free(str);
    str = NULL;
}
