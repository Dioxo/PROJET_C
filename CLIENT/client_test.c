#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#include "client_orchestre.h"

static void send(co_Pair *pipes, co_Response *response)
{
    co_orchestraWriteData(pipes, &(response->password), sizeof(int));
    co_orchestraWriteData(pipes, &(response->lengthCtoS), sizeof(int));
    co_orchestraWriteData(pipes, &(response->lengthStoC), sizeof(int));
    co_orchestraWriteData(pipes, response->CtoS, (response->lengthStoC) * sizeof(char));
    co_orchestraWriteData(pipes, response->CtoS, (response->lengthCtoS) * sizeof(char));
}


int main() {
	co_Pair pipes;
    co_Connection connection;
    co_Response data;


    Semaphore mutex;
    mutex = createSema(1);



    co_orchestraCreatePipes(&pipes);

    co_orchestraOpenPipes("pipeClientToOrchestra","pipeOrchestraToClient", &pipes);
    co_orchestraReadData(&pipes, &(connection.request), sizeof(int));
    printf("%d\n", connection.request);

    data.password  = 123456;
    data.lengthCtoS = 9;
    data.lengthStoC = 9;
    data.CtoS = "Totototo";
    data.StoC = "otototoT";



    

    if (connection.request != REQUEST_STOP)
    {

    	co_Connection response = {REQUEST_ACCEPT};
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
    pSema(mutex);
    vSema(mutex);
    co_orchestraDestroyPipes(&pipes);
   

    
  return EXIT_SUCCESS;
}
