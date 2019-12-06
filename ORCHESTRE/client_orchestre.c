#define _POSIX_C_SOURCE 200809L    // pour strdup

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "myassert.h"

#include "client_orchestre.h"

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void createPipe(const char *basename, const char *msg, int numPipe, int numService, NamedPipe *pipe)
{
	int nameLength;
	nameLength = snprintf(NULL, 0, "%s_%d_%d", basename, numPipe, numService);
	pipe->name = malloc((nameLength + 1) * sizeof(char));
	sprintf(pipe->name, "%s_%d_%d", basename, numPipe, numService);

	int ret = mkfifo(pipe->name, 0600);
    myassert(ret == 0, msg);
}
//---------------------------------------------------------------------------------
void orchestraCreatePipes(int numPipe, int numService, Pair *pipes)
{
	createPipe("pipeClientToOrchestra", "Création tube CtoO", 
		numPipe, numService, &(pipes->CtoO));

	createPipe("pipeOrchestraToClient", "Création tube OtoC",
		numPipe, numService, &(pipes->OtoC));

}


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void destroyPipe(const char *msg, NamedPipe *pipe)
{
    int ret;

    ret = unlink(pipe->name);
    myassert(ret == 0, msg);
    free(pipe->name);
    pipe->name = NULL;
}
//---------------------------------------------------------------------------------
void orchestraDestroyPipes(Pair *pipes)
{
    destroyPipe("destruction tube CtoO", &(pipes->CtoO));
    destroyPipe("destruction tube OtoC", &(pipes->OtoC));
}


//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void openPipe(const char *name, int flag, const char *msg, NamedPipe *pipe)
{
    pipe->name = strdup(name);
    pipe->fd = open(pipe->name, flag);
    myassert(pipe->fd != -1, msg);
}
//---------------------------------------------------------------------------------
void orchestraOpenPipes(const char *nameCtoO, const char *nameOtoC, Pair *pipes)
{
	openPipe(nameCtoO, O_WRONLY, "orchestra ouverture tube CtoO", &(pipes->CtoO));
	openPipe(nameOtoC, O_RDONLY, "orchestra ouverture tube OtoC", &(pipes->OtoC));
}
//---------------------------------------------------------------------------------
void clientOpenPipes(const char *nameCtoO, const char *nameOtoC, Pair *pipes)
{
	openPipe(nameCtoO, O_RDONLY, "client ouverture tube CtoO", &(pipes->CtoO));
	openPipe(nameOtoC, O_WRONLY, "client ouverture tube OtoC", &(pipes->OtoC));

}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void closePipe(const char *msg, NamedPipe *pipe)
{
	int ret;
    
    free(pipe->name);
    pipe->name = NULL;
    ret = close(pipe->fd);
    myassert(ret == 0, msg);
    pipe->fd = -1;
}
//---------------------------------------------------------------------------------
void orchestraClosePipes(Pair *pipes)
{
	closePipe("orchestra fermeture tube CtoO",&(pipes->CtoO));
	closePipe("orchestra fermeture tube OtoC", &(pipes->OtoC));
}
//---------------------------------------------------------------------------------
void clientClosePipes(Pair *pipes)
{
	closePipe("client fermeture tube CtoO",&(pipes->CtoO));
	closePipe("client fermeture tube OtoC", &(pipes->OtoC));
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
static void writeData(NamedPipe *pipe, const void *buf, size_t size, const char *msg)
{
    ssize_t ret = write(pipe->fd, buf, size);
    myassert(ret != -1, msg);
    myassert((size_t)ret == size, msg);
}
//---------------------------------------------------------------------------------
void orchestraWriteData(Pair *pipes, const void *buf, size_t size)
{
	writeData(&(pipes->OtoC), buf, size, "orchestra écriture vers client");
}
//---------------------------------------------------------------------------------
void clientWriteData(Pair *pipes, const void *buf, size_t size)
{
	writeData(&(pipes->CtoO), buf, size, "client écriture vers orchestra");
}

//---------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
static void readData(NamedPipe *pipe, void *buf, size_t size, const char *msg)
{
	ssize_t ret = read(pipe->fd, buf, size);
    myassert(ret != -1, msg);
    myassert((size_t)ret == size, msg);
}
//---------------------------------------------------------------------------------
void orchestraReadData(Pair *pipes, void *buf, size_t size)
{
    readData(&(pipes->OtoC), buf, size, "orchestra lecture de client");
}
//---------------------------------------------------------------------------------
void clientReadData(Pair *pipes, void *buf, size_t size)
{
    readData(&(pipes->CtoO), buf, size, "client lecture de orchestra");
}


/*
NE SERT A RIEN
int main()
{
	    return EXIT_SUCCESS;
}

*/
