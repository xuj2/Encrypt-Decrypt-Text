#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#define MAX_CHAR 999999 //max size for data to send

char *server_hostname = "localhost";

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsRead;
  struct sockaddr_in serverAddress;

  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"CLIENT ERROR: text, key and port number required!\n"); 
    exit(0); 
  } 

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), server_hostname);

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }
  
  //opening text and key files
  FILE *text_file, *key_file;
  text_file = fopen(argv[1], "r");
  key_file = fopen(argv[2], "r");
  if (text_file == NULL || key_file == NULL)
  {
    fprintf(stderr, "CLIENT ERROR: can not open input files!\n");
    close(socketFD);
    exit(1);
  }

  //reading text and key files by character and storing them into strings
  char text[MAX_CHAR];
  char key[MAX_CHAR];
  int text_len = 0;
  int key_len = 0;
  char text_ch, key_ch;
  while ((text_ch = fgetc(text_file)) != EOF)
  {
    if (text_ch < 65 || text_ch > 90)
    {
      if (text_ch != 32 && text_ch != 10) //bad character detected in text
      {
        fprintf(stderr, "CLIENT ERROR: text contains bad character!\n");
        close(socketFD);
        exit(1);
      }
    }
    text[text_len] = text_ch;
    text_len++;
  }

  while ((key_ch = fgetc(key_file)) != EOF)
  {
    if (key_ch < 65 || key_ch > 90)
    {
      if (key_ch != 32 && key_ch != 10) // bad character detected in key
      {
        fprintf(stderr, "CLIENT ERROR: key contains bad character!\n");
      }
    }
    if (key_len < text_len)
    {
      key[key_len] = key_ch;
      key_len++;
    }
  }
  key[key_len] = 10;

  if (key_len < text_len)
  {
    fprintf(stderr, "CLIENT ERROR: key is shorter than text!\n");
    close(socketFD);
    exit(1);
  }

  fclose(text_file);
  fclose(key_file);

  //send ciphertext and key to server as one packet
  char data[MAX_CHAR];
  data[0] = 'd'; //indicates it's dec_client and not enc_client
  strcat(data, text);
  strcat(data, key);
  int data_sent;
  while (1) //repeat until all data sent
  {
    data_sent = send(socketFD, data, strlen(data), 0);
    if (data_sent < 0)
    {
      fprintf(stderr, "CLIENT ERROR: failed to send data\n");
    }
    else if (data_sent < strlen(data))
    {
      fprintf(stderr, "CLIENT ERROR: not all data sent\n");
    }
    else
    {
      break;
    }
  }
  
  //receive text from server
  char enc_data[MAX_CHAR];
  int data_recv = recv(socketFD, enc_data, MAX_CHAR, 0);;
  if (data_recv < 0)
  {
    fprintf(stderr, "CLIENT ERROR: failed to receive data.\n");
    close(socketFD);
    exit(2);
  }
  printf("%s", enc_data);
  // Close the socket
  close(socketFD);
  return 0;
}
