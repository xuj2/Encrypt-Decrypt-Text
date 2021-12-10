#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define MAX_CHAR 999999 //max size for data to receive

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

void *encrypt(void *args)
{
  char text[MAX_CHAR];
  char key[MAX_CHAR];
  char enc_text[MAX_CHAR];
  char *saveptr;
  char *token = strtok_r((char *)args, "\n", &saveptr);
  strcpy(text, token);
  token = strtok_r(NULL, "\n", &saveptr);
  strcpy(key, token);

  int m, n;
  int enc_char;
  for (int i = 0; i < (strlen(text) - 1); i++)
  {
    m = text[i + 1];
    n = key[i];
    if (text[i + 1] == 32)
    {
      m = 91;
    }
    if (key[i] == 32)
    {
      n = 91;
    }
    enc_char = (((m - 65) + (n - 65)) % 27);
    if (enc_char == 26)
    {
      enc_char = -33;
    }
    enc_text[i] = (enc_char + 65);
  }
  memset((char *)args, '\0', MAX_CHAR);
  strcpy((char *)args, enc_text);
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 

  //allows up to 10 threads to be created
  pthread_t tid[10];
  int t = 0;
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    int data_recv;
    char data[MAX_CHAR];
    memset(data, '\0', MAX_CHAR);
    data_recv = recv(connectionSocket, data, MAX_CHAR, 0);
    if (data_recv <= 0) //client closes its connection or failed to send data
    {
      fprintf(stderr, "SERVER ERROR: no data received from client.\n");
      close(connectionSocket);
    }
    else
    {
      if (data[0] == 100) //reject dec_client
      {
        fprintf(stderr, "SERVER ERROR: connection rejected.\n");
        close(connectionSocket);
      }
      else
      {
        if (data[strlen(data) - 1] == 10) //all data received
        {
          while (1) //loop until there's opening for new thread
          {
            if (t >= 10)
            {
              t = 0;
            }
            if (pthread_create(&tid[t], NULL, encrypt, (void *)data) != 0)
            {
              t++;
              fprintf(stderr, "SERVER ERROR: failed to create thread, attemping to create a new one...\n");
            }
            else
            {
              break; //end loop after thread successfully created
            }
          }
          //runs thread and store its returned data
          pthread_join(tid[t], NULL);
          data[strlen(data)] = 10;
          //send the data back to client
          int data_sent;
          while (1) //repeat until all data sent
          {
            data_sent = send(connectionSocket, data, strlen(data), 0);
            if (data_sent < strlen(data))
            {
              fprintf(stderr, "SERVER ERROR: failed to send all data.\n");
            }
            else
            {
              break;
            }
          }
          close(connectionSocket);
        }
        else
        {
          fprintf(stderr, "SERVER ERROR: only partial data received!\n");
          close(connectionSocket);
        }
      }
    }
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
