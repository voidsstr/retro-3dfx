#include <assert.h>
#include <iostream.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define DIAG_PATH "m:\\3dfx\\devel\\h5\\bin\\"
#define SERVER "babyjesus"
#define PORT 10187

class Diags
{
private:
  SOCKET socketDescriptor;

protected:
  char clientName[2048];
  char boardID[2048];
  char chipID[2048];
  char diagRelease[2048];
  char frequency[2048];
  char voltage[2048];
  
  bool debug;
  
  int diagCount;  //Number of diags in selected list

  void connectToServer(char *serverName, int serverPort);
  void disconnectFromServer(void);
  void errorMessageAndExit(const char *, ...);
  void getVariable(char *variableName, char *value, bool substituteDummyVariables);
  int mySystem(char *commandLine);

public:
  Diags(char *serverName, int serverPort, char *diagList,
	bool _debug=false, bool substituteDummyVariables=false);

  int askServer(char *question, char *answer, int length);
  void getDiagsLists(void);
  void getListDiags(void);
  void runDiags(void);
};

void usage(char *applicationName);

int main(int argc, char **argv)
{
  int i;
  bool debug=false;
  bool printDiagsList=false;
  bool printDiagList=false;
  char diagListToPrint[2048];
  char *diagList=NULL;
  
  //Scan command line
  if(argc <= 1)
    usage(argv[0]);
  
  for(i=1; i<argc; i++)
    {
      if(!strcmp(argv[i], "-d"))
	debug=true;

      if(!strcmp(argv[i], "-h"))
	usage(argv[0]);
      
      if(!strcmp(argv[i], "-l"))
	printDiagsList=true;

      if(!strcmp(argv[i], "-t"))
	{
	  printDiagList=true;

	  if(!(i+1 < argc))  //Make sure the diagList name is included
	    usage(argv[0]);
	  
	  strcpy(diagListToPrint, argv[i+1]);
	}
    }

  //Create the diags client
  if(printDiagsList)
    {
      diagList=NULL;
    }
  else
    diagList=argv[argc-1];

  Diags diags(SERVER, PORT, diagList, debug, 
	      printDiagList || printDiagsList);

  if(printDiagsList)
    {
      diags.getDiagsLists();
      return(0);
    }

  if(printDiagList)
    {
      diags.getListDiags();
      return(0);
    }

  diags.runDiags();

  return(0);
}

Diags::Diags(char *serverName, int serverPort, char *diagList,
	     bool _debug, bool substituteDummyVariables)
{
  char question[8192];
  char response[8192];
  int length;

  debug=_debug;

  connectToServer(serverName, serverPort);

  //Need to get

  getVariable("BOARD_ID", boardID, substituteDummyVariables);
  getVariable("CHIP_ID", chipID, substituteDummyVariables);
  getVariable("DIAG_RELEASE", diagRelease, substituteDummyVariables);
  getVariable("SSTH3_GRXCLOCK", frequency, substituteDummyVariables);
  getVariable("VOLTAGE", voltage, substituteDummyVariables);  

  if(diagList != NULL)
    {
      sprintf(question, "diagList(%s)", diagList);
      length = askServer(question, response, 8192);
      diagCount = atoi(response);

      if(debug)
	cout<<"diagList "<<diagList<<" has "<<diagCount<<" diags"<<endl;
    }

  sprintf(question, "clientName(%s)", clientName);
  length = askServer(question, response, 8192);

  sprintf(question, "boardID(%s)", boardID);
  length = askServer(question, response, 8192);

  sprintf(question, "chipID(%s)", chipID);
  length = askServer(question, response, 8192);

  sprintf(question, "diagRelease(%s)", diagRelease);
  length = askServer(question, response, 8192);

  sprintf(question, "frequency(%s)", frequency);
  length = askServer(question, response, 8192);

  sprintf(question, "voltage(%s)", voltage);
  length = askServer(question, response, 8192);  
}

int Diags::askServer(char *question, char *answer, int length)
{
  int returnCode;
  char junkBuffer[128];

  if(debug)
    cerr<<"CLIENT: "<<question<<endl;

  returnCode = send(socketDescriptor, question, strlen(question), 0);
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Send failed: %d\n", WSAGetLastError());

  returnCode = recv(socketDescriptor, answer, length, 0);
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Receive failed: %d\n", WSAGetLastError());
  //returnCode is the number of bytes read
  answer[returnCode] = 0;  //Null terminate the string

  //Pick up the return transmitted from the server
  junkBuffer[0]=0;
  returnCode = recv(socketDescriptor, junkBuffer, 128, 0);
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Receive failed: %d\n", WSAGetLastError());
  assert(junkBuffer[0] = '\n');

  if(debug)
    cerr<<"SERVER: "<<answer<<endl;

  //Check for error
  if(strstr(answer, "ERROR") || strstr(answer, "error") || strstr(answer, "Error"))
    errorMessageAndExit("Server got pissed off!\nCLIENT asked: %s\nSERVER said: %s\n", 
			question, answer);

  return(returnCode);
}

void Diags::connectToServer(char *serverName, int serverPort)
{
  WORD wVersionRequested;
  WSADATA wsaData;
  int returnCode;

  HOSTENT *pHostEnt;
  HOSTENT *pSelfEnt;
  struct sockaddr_in sin;
  struct sockaddr_in self;
  char selfAddress[4];
  int sizeSelf;
  
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
      exit(-1);
    }
  
  socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);
  if(socketDescriptor == INVALID_SOCKET)
    errorMessageAndExit("Socket creation failed: %d\n", WSAGetLastError());

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(serverPort);

  if(pHostEnt = gethostbyname(serverName))
    memcpy(&sin.sin_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);      
  else 
    errorMessageAndExit("Can't get %s\" host entry: %d\n", SERVER, WSAGetLastError());

  returnCode = connect(socketDescriptor, (struct sockaddr *) &sin, sizeof(sin));
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Connect failed: %d\n", WSAGetLastError());

  //Get the name of the client
  if(!getsockname(socketDescriptor, (struct sockaddr *)&self, &sizeSelf))
    errorMessageAndExit("getsockname failed: %d\n", WSAGetLastError());
  memcpy(selfAddress, &self.sin_addr, sizeof(selfAddress));
  pSelfEnt = gethostbyaddr(selfAddress, 4, PF_INET);
  if(pSelfEnt == NULL)
    errorMessageAndExit("gethostbyaddr of self failed\n");
  strcpy(clientName, pSelfEnt->h_name);

  if(debug)
    cout<<"clientName = \""<<clientName<<"\""<<endl;

  if(debug)
    cout<<"Successfully connected to server with socketDescriptor = "<<socketDescriptor<<endl;
}

void Diags::disconnectFromServer(void)
{
  int returnCode;

  returnCode = closesocket(socketDescriptor);
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Close socket failed: %d\n", WSAGetLastError());
  
  returnCode = WSACleanup();
  if(returnCode == SOCKET_ERROR)
    errorMessageAndExit("Cleanup failed: %d\n", WSAGetLastError()); 
}

void Diags::errorMessageAndExit(const char *format, ...)
{
  va_list args;
  
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  WSACleanup();
  exit(1);
}

void Diags::getDiagsLists(void)
{
  char response[8192];
  int length;

  cout<<"Existing Diag Lists on server:"<<endl;
  cout<<response<<endl;

  length = askServer("diagLists()", response, 8192);
  cout<<response<<endl;
}

void Diags::getListDiags(void)
{
  char question[8192];
  char response[32768];
  int length;
  int i;

  for(i=0; i<diagCount; i++)
    {
      sprintf(question, "diag(%d)", i);
      length = askServer(question, response, 32768);
      cout<<i<<": "<<response<<endl;
    }
}

void Diags::getVariable(char *variableName, char *value, bool substituteDummyVariables)
{
  char *result;

  if(substituteDummyVariables)
    {
      strcpy(value, "DUMMY");
    }
  else
    {
      if((result = getenv(variableName)))
	strcpy(value, result);
      else
	{
	  cout<<"Environment variable \""<<variableName<<"\" not set."<<endl;
	  cout<<"Enter value: ";
	  cin>>value;
	}
    }      

  if(debug)
    cout<<"\""<<variableName<<"\" set to \""<<value<<"\""<<endl;
}

//This is necessary because win98 sucks poo. system() doesn't
//properly return the exit code of the process.
int Diags::mySystem(char *commandLine)
{
  char *copy;
  char *args[1000];
  char commandWithPath[1000];
  int i;
  int exitCode;

  copy = strdup(commandLine);
  assert(copy);

  //Split the f'er up with spaces
  i=0;
  args[i++]=strtok(copy, " ");
  while(args[i++] = strtok(NULL, " "))
    ;
  
  args[i++] = NULL;

  sprintf(commandWithPath, "%s%s", DIAG_PATH, args[0]);
  args[0] = commandWithPath;

  //See if the file exists
  FILE *fin;
  if((fin = fopen(args[0], "rb")) == NULL)
    cerr<<"Couldn't locate "<<args[0]<<endl;
  else
    fclose(fin);

  exitCode = _spawnv(_P_WAIT, args[0], args);
  
  free(copy);

  return(exitCode);
}

void Diags::runDiags(void)
{
  char question[8192];
  char response[32768];
  int length;
  int i;
  int exitCode;

  for(i=0; i<diagCount; i++)
    {
      sprintf(question, "diag(%d)", i);
      length = askServer(question, response, 32768);

      exitCode = mySystem(response);

      //Windows 98 is such a rat piece of shit that
      //it doesn't return the exit code from system()
      //like it's supposed to.

      cout<<"exitCode = "<<exitCode<<endl;      
      
      if(exitCode == 0)
	{
	  sprintf(question, "pass(%d)", i);
	  length = askServer(question, response, 32768);
	}
      else
	{
	  sprintf(question, "fail(%d)", i);
	  length = askServer(question, response, 32768);

	  //Quit on error
	  exit(0);
	}
      
    }
}

void usage(char *applicationName)
{  
  cerr<<"Usage: "<<applicationName<<" (-d | -h | -l)* (-t)? <diagList>"<<endl;
  cerr<<"      diagList: diag list to run"<<endl;
  cerr<<"      d=> Debugging mode"<<endl;
  cerr<<"      h=> Prints this message"<<endl;
  cerr<<"      l=> Lists all diag lists"<<endl;
  cerr<<"      t=> Types out diags in <diagList>"<<endl;
    
  exit(-1);
}

