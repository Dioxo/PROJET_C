#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "client_orchestre.h"

int main() {
	Pair pipes;
    Connection connection;


    orchestraCreatePipes(&pipes);
    orchestraOpenPipes("pipeClientToOrchestra","pipeOrchestraToClient", &pipes);
    
    orchestraReadData(&pipes, &(connection.request), sizeof(int));
    printf("%d\n", connection.request);

/*
    if (connection.request == REQUEST_ASK)
    {
    	Connection reponse = {REQUEST_ACCEPT};
    	orchestraWriteData(&pipes, &reponse, sizeof(int));
    }
    else
    {
    	printf("Error");

    }
    orchestraClosePipes(&pipes);
*/

  return EXIT_SUCCESS;
}
