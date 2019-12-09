#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include "client_service.h"


static bool created = false;
static bool deleted = false;

//---------------------------------------------------------------------------------
static void createPipe(const char *basename, int numService, NamedPipe *pipe)
{

		int nameLength;
		nameLength = snprintf(NULL, 0, "%s_%d", basename, numService);
		pipe->name = malloc((nameLength + 1) * sizeof(char));
	  sprintf(pipe->name, "%s_%d", basename, numService);

		int ret = mkfifo(pipe->name, 0644);
	  assert(ret == 0);
}
//---------------------------------------------------------------------------------
void createPipes(int numService, Pair *pipes)
{
	if(!created){
		createPipe("C_S", numService, &(pipes->C_S));
		createPipe("S_C", numService, &(pipes->S_C));
		created = true;
	}
}


//---------------------------------------------------------------------------------
static void destroyPipe(NamedPipe *pipe)
{
		int ret;
		ret = unlink(pipe->name);
		assert(ret == 0);
		free(pipe->name);
		pipe->name = NULL;

}
//---------------------------------------------------------------------------------
void destroyPipes(Pair *pipes)
{
	if (!deleted) {
    destroyPipe(&(pipes->S_C));
    destroyPipe(&(pipes->C_S));
		deleted = true;
	}
}


//---------------------------------------------------------------------------------
static void openPipe(int flag, NamedPipe *pipe)
{
    pipe->fd = open(pipe->name, flag);
    assert(pipe->fd != -1);
}
//---------------------------------------------------------------------------------
void serviceOpenPipes(const char *s_c,const char *c_s, Pair *pipes)
{
	pipes->C_S.name = malloc(sizeof(char) * (strlen(c_s) + 1));
	strcpy(pipes->C_S.name, c_s);

	pipes->S_C.name = malloc(sizeof(char) * (strlen(s_c) + 1 ));
	strcpy(pipes->S_C.name, s_c);

	openPipe( O_RDONLY , &(pipes->C_S));
	openPipe( O_WRONLY , &(pipes->S_C));
}
//---------------------------------------------------------------------------------
void clientOpenPipes(const char *s_c,const char *c_s, Pair *pipes)
{
	pipes->C_S.name = malloc(sizeof(char) * (strlen(c_s) + 1));
	strcpy(pipes->C_S.name, c_s);

	pipes->S_C.name = malloc(sizeof(char) * (strlen(s_c) + 1 ));
	strcpy(pipes->S_C.name, s_c);

	openPipe( O_WRONLY, &(pipes->C_S));
	openPipe( O_RDONLY,&(pipes->S_C));

}

//---------------------------------------------------------------------------------
static void closePipe(NamedPipe *pipe)
{
	int ret;
  free(pipe->name);
  pipe->name = NULL;
  ret = close(pipe->fd);
	if (ret == -1) {
		perror("ERROR : ");
	}
  assert(ret == 0);
  pipe->fd = -1;
}

//---------------------------------------------------------------------------------
void clientClosePipes(Pair *pipes)
{
	closePipe(&(pipes->C_S));
	closePipe(&(pipes->S_C));
}

//---------------------------------------------------------------------------------
void serviceClosePipes(Pair *pipes)
{
	closePipe(&(pipes->S_C));
	closePipe(&(pipes->C_S));
}

//---------------------------------------------------------------------------------
static void writeData(NamedPipe *pipe, const void *buf, size_t size)
{
    ssize_t ret = write(pipe->fd, buf, size);
    assert(ret != -1);
    assert((size_t)ret == size);
}
//---------------------------------------------------------------------------------
void serviceWriteData(Pair *pipes, const void *buf, size_t size)
{
	writeData(&(pipes->S_C), buf, size);
}
//---------------------------------------------------------------------------------
void clientWriteData(Pair *pipes, const void *buf, size_t size)
{
	writeData(&(pipes->C_S), buf, size);
}

//--------------------------------------------------------------------------------
static void readData(NamedPipe *pipe, void *buf, size_t size)
{
	ssize_t ret = read(pipe->fd, buf, size);
    assert(ret != -1);
    assert((size_t)ret == size);
}
//---------------------------------------------------------------------------------
void serviceReadData(Pair *pipes, void *buf, size_t size)
{
    readData(&(pipes->C_S), buf, size);
}
//---------------------------------------------------------------------------------
void clientReadData(Pair *pipes, void *buf, size_t size)
{
    readData(&(pipes->S_C), buf, size);
}
