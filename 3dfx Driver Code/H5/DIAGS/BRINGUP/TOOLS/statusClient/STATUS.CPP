
#include <iostream.h>
#include <stdio.h>
#include <time.h>
#include <winsock2.h>

#define SERVER "babyjesus"
#define PORT 10666

#define MESSAGE_SIZE 256*1024
char message[MESSAGE_SIZE];

void buildMessage(int argc, char **argv, char *message, int messageLength);
void errorMessageAndExit(const char *, ...);
void usage(char *applicationName);

int main(int argc, char *argv[])
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int returnCode;

  time_t currentTime;

  SOCKET socketDescriptor;

  HOSTENT *pHostEnt;
  struct sockaddr_in sin;

  if(argc <= 1)
    usage(argv[0]);

  //Build the message
  buildMessage(argc, argv, message, MESSAGE_SIZE-1);
  time(&currentTime);
  strcat(message, "\n<br>");
  strcat(message, ctime(&currentTime));

  //Send the message to the status server
  wVersionRequested = MAKEWORD(2,2);	// Request winsock 2.2
  returnCode = WSAStartup(wVersionRequested, &wsaData);
  if(returnCode != 0)
    errorMessageAndExit("Startup failed: %d\n", returnCode);

  if(LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) ||
     HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))
    {
      printf("Supported version is too low\n");
      WSACleanup( );
      return 0; 
    }
  
  socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);
  if(socketDescriptor == INVALID_SOCKET)
    errorMessageAndExit("Socket creation failed: %d\n", WSAGetLastError());

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(PORT);

  if(pHostEnt = gethostbyname(SERVER))
    memcpy(&sin.sin_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);      
  else 
    errorMessageAndExit("Can't get %s\" host entry: %d\n", SERVER, WSAGetLastError());
  
  returnCode = connect(socketDescriptor, (struct sockaddr *) &sin, sizeof(sin));
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Connect failed: %d\n", WSAGetLastError());

  returnCode = send(socketDescriptor, message, strlen(message), 0);
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Send failed: %d\n", WSAGetLastError());
  
  returnCode = closesocket(socketDescriptor);
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Close socket failed: %d\n", WSAGetLastError());

  returnCode = WSACleanup();
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Cleanup failed: %d\n", WSAGetLastError());
  
  return(0);
}


void buildMessage(int argc, char **argv, char *message, int messageLength)
{
  if(!strcmp(argv[1], "-f"))
    {
      //Case where file is used for status
      if(argc == 2)
	usage(argv[0]);

      FILE *fin;
      int length;
      if((fin = fopen(argv[2], "rt")) == NULL)
	{
	  cerr<<"Couldn't open \""<<argv[2]<<"\""<<endl;
	  exit(-1);
	}

      length = fread(message, 1, messageLength-2, fin);
      message[length] = 0;  //Make sure the message is null terminated
    }
  else
    {
      //Case where status is typed on command line
      int i;

      for(i=1; i<argc; i++)
	{
	  strcpy(message, argv[i]);
	  message += strlen(argv[i]);
	  *message = ' ';
	  message++;
	  *message = 0;	  
	}
    }
}

void errorMessageAndExit(const char *format, ...)
{
  va_list args;
  
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  WSACleanup();
  exit(1);
}

void usage(char *applicationName)
{
  cerr<<"Usage: "<<applicationName<< 
    "  -f <filename> | <status message>"<<endl;
  
  exit(-1);
}
