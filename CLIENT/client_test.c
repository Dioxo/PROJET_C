#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "client_orchestre.h"

static void send(Pair *pipes, Response *response)
{
    co_orchestraWriteData(pipes, &(response->password), sizeof(int));
    co_orchestraWriteData(pipes, &(response->lengthCtoO), sizeof(int));
    co_orchestraWriteData(pipes, &(response->lengthOtoC), sizeof(int));
    co_orchestraWriteData(pipes, response->CtoO, (response->lengthOtoC) * sizeof(char));
    co_orchestraWriteData(pipes, response->CtoO, (response->lengthCtoO) * sizeof(char));
}


int main() {
	Pair pipes;
    Connection connection;
    Response data;


    co_orchestraCreatePipes(&pipes);

    co_orchestraOpenPipes("pipeClientToOrchestra","pipeOrchestraToClient", &pipes);
    co_orchestraReadData(&pipes, &(connection.request), sizeof(int));
    printf("%d\n", connection.request);

    data.password  = 123456;
    data.lengthCtoO = 8;
    data.lengthOtoC = 8;
    data.CtoO = "Totototo";
    data.OtoC = "otototoT";

    

    if (connection.request != REQUEST_STOP)
    {

    	Connection response = {REQUEST_ACCEPT};
    	co_orchestraWriteData(&pipes, &response, sizeof(int));
        //
        printf("Orchestre receive : %d\n", connection.request);
        send(&pipes, &data);
        int ack;
        co_orchestraReadData(&pipes, &(ack), sizeof(int)); 
        printf("Accusé de reception : %d\n", ack);
    }
    else
    {
    	printf("Stop the orchestra");
    	

    }
    //co_orchestraDestroyPipes(&pipes);
   

    
  return EXIT_SUCCESS;
}
