/***********************
** Author: Eddie C. Fox
** Date: March 14, 2017
** Description: This is ftserver.c program, a file transfer server designed to work
** with ftclient.py. For additional details and citations of resources used, see the
** README.txt file accompanying this. 

** Proper syntax: ftserver [PORT NUMBER]
***********************/

/* Some standard includes from the beej networking guide. I mostly used the includes I used
for project 1, but added <dirent.h>, <stdint.h>, and <signal.h> because I needed them. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <signal.h>
#include <dirent.h>

#define HANDLE "ftserver" /* Define handle for being used by functions below. */

/* Because we frequently create buffers, I have defined BUFFER for the size of messages, 
and MAXBUFFER for the maximum size for reading from files. Most protocols use 8192 max. */

/* Message buffer size.*/
#define BUFFER 1024

/* File read buffer size. */
#define MAXBUFFER 8192

/* Some functions are used by others, so first I list prototypes of all functions used by main. Then I define these functions
and end with the main() function, which will utilize these functions. */

/* -- Function Prototypes -- */

int max(int a, int b);

void sigintHandle(int sigNum);
void removeNewline(char *string);
int getLine(char *buffer, char *handle);

int portValidation(char *input);
int startServer(int port);

int sendMessage(int sockDescriptor, char *message);
int receiveMessage(int sockDescriptor, char *message, unsigned size);

unsigned sendNumber(int sockDescriptor, unsigned number);
unsigned receiveNumber(int sockDescriptor);

int handleRequest(int clientSocket, char *buffer);

int readFile(char *filename, char *string);
int sendFile(char* file, int clientDataSocket, int clientSocket);

int sendResponse(char *sentMessage, char *message, int clientSocket);

/* -- Function definitions --*/

/* Return the largest of two integers. Fairly simple. */

int max(int a, int b)
{
	if (a >= b)
	{
		return a;
	}
	else
	{
		return b;
	}
}


/* Handler to catch exiting children and prevent zombie processes. Learned this stuff in operating systems class.*/

void sigintHandle(int sigNum)
{
	pid_t pid; /* Find process id. */
	int status;
	pid = waitpid(-1, &status, WNOHANG); /* Wait for child to exit. */
}


/* Take string and remove trailing newline if it exists, replacing it with a null terminator.*/

void removeNewline(char *string)
{
	int last = strlen(string) - 1; /* Variable to hold last character/*/
	if (string[last] == '\n')  /* If last character is a newline, replace it with null terminator.*/
	{
		string[last] = '\0';
	}
}
/* Get line of user input, remove the trailing newline, and put it into buffer.*/

int getLine(char *buffer, char *handle)
{
	printf("%s$ ", handle);
	fgets(buffer, (BUFFER - 1), stdin);
	removeNewline(buffer);
	return 0;
}


/* Take input string, convert to integer, and validate that it is between 1024 and 65535. 
Return -1 if invalid, otherwise return port number. */

int portValidation(char *input) 
{
	int port = atoi(input);
	if (port < 1024 || port > 65535) 
	{
		return -1;
	}

	else
	{
		return port;
	}
}

/* Takes a port number and uses it as a parameter to open and bind a socket for the
server to listen on. Returns -1 if it fails, and the file descriptor of the socket otherwise. */
int startServer(int port)
{
	int sockDescriptor;

	/* If socket fails to be created. */

	if ((sockDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		return -1;
	}

	/* Important stuff needed to get addressing information. From Beej's guide. */

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	int optval = 1;
	setsockopt(sockDescriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	

	// If binding socket fails, return -1. 

	if (bind(sockDescriptor, (struct sockaddr *) &server, sizeof(server)) < 0) 
	{
		return -1;
	}

	/* If listening on the port fails, return -1. */

	if (listen(sockDescriptor, 10) < 0)
	{
		return -1;
	}

	/* If the program reaches this point, it has been successful, and we can return the socket file descriptor.*/

	return sockDescriptor;
}

/* Takes a socket file descriptor and message that will be sent. 
Sends until full message is delivered. Will return 0 on success, or -1 if unsuccessful. */

int sendMessage(int sockDescriptor, char *message) 
{
	/* Status variable.*/
	unsigned status;

	/* Variable for size of message. */
	unsigned size = strlen(message) + 1;

	/* Variable to hold the total number of bytes sent. */
	unsigned bytesSent = 0;

	/* While sending is not complete, continue the loop. */

	while (bytesSent < size)
	{
		/* Continue to send message.*/
		status = write(sockDescriptor, message + bytesSent, size - bytesSent);

		/* Add bytes sent to bytes sent. */
		bytesSent += status;

		if (status < 0) 
		{
			return -1;
		}

		/* If complete. */

		else if (status == 0) 
		{
			bytesSent = size - bytesSent;
		}
	}
	return 0;
}

/* Takes socket file descriptor, an output string to receive a messaage in, and the size of the message.
Will read until the entire message has been received, then copy it to the string. Will return 0 if successful 
and -1 if not. */

int receiveMessage(int sockDescriptor, char *message, unsigned size) 
{
	char buffer[BUFFER];
	unsigned status;
	unsigned bytesReceived = 0;

	while (bytesReceived < size) 
	{
		status = read(sockDescriptor, buffer + bytesReceived, size - bytesReceived);
		bytesReceived += status;

		if (bytesReceived < 0) 
		{
			return -1;
		}

		else if (status == 0) 
		{
			bytesReceived = size - bytesReceived;
		}
	}

	strncpy(message, buffer, size); /* Copy buffer into message variable. */

	return 0;
}

/* Takes socket descriptor and sends integer parameter over the socket. 
Returns -1 if unsuccessful and 0 if successful. */

unsigned sendNumber(int sockDescriptor, unsigned number) 
{
	unsigned realNumber = number;
	int status = write(sockDescriptor, &realNumber, sizeof(unsigned));

	if (status < 0) 
	{
		return -1;
	}

	else
	{
		return 0;
	}
}

/* Takes socket descriptor and receives integer over the socket. Returns -1 if unsuccessful and 
0 if successful. */

unsigned receiveNumber(int sockDescriptor)
{
	unsigned number;
	int status = read(sockDescriptor, &number, sizeof(unsigned));

	if (status < 0) 
	{
		return -1;
	}

	else
	{
		return number;
	}
		
}

/* Takes socket file descriptor and buffer address. Will receive client request,
and after processing it, return integer code for the request type. */

int handleRequest(int clientSocket, char *buffer) 
{
	int size = receiveNumber(clientSocket); /* Get size of incoming command message from client. */
	int receivedMessage = receiveMessage(clientSocket, buffer, size); /* Get command.*/
	buffer[size] = '\0'; /* Add null terminator.*/

	/* return appropriate code depending on what the command was. -g get command
	has a return type of 2. -l list command has a return type of 1. Otherwise, return -1. */


	if (strncmp("-g", buffer, strlen(buffer)) == 0) 
	{
		return 2;
	}

	else if (strncmp("-l", buffer, strlen(buffer)) == 0) 
	{
		return 1;
	}

	else 
	{
		return -1;
	}
}

/* Takes filename and string pointer. Opens the file, reads the contents, and writes
the contents to the string. Will return 0 if successful. Used stack overflow to figure out how. */

int readFile(char *filename, char *string) 
{
	
	FILE *filePointer; /* Create file pointer. */
	filePointer = fopen(filename, "rb"); /* Open file. */
	fseek(filePointer, 0, SEEK_END); /* Find end of file. */
	long fsize = ftell(filePointer); /* Determine size of file. */
	fseek(filePointer, 0, SEEK_SET); /* Position at beginning of file. */

	char *buffer = malloc(fsize + 1); /* Create buffer large enough to read entire file.*/
	fread(buffer, fsize, 1, filePointer); /* Read file into buffer. */
	buffer[fsize] = '\0'; /* Add null terminator. */

	fclose(filePointer); /* Close file. */
	strncpy(string, buffer, MAXBUFFER); /* Copy buffer into the string. */
	return 0;
}

/* Takes file buffer, client data socket file descriptor, and client 
control socket descriptor. Sends file to client using data socket, then close connection. */


int sendFile(char* file, int clientDataSocket, int clientSocket) 
{
	int r = sendNumber(clientDataSocket, (strlen(file))); /* Send size. */
	r = sendMessage(clientDataSocket, file);            // send the reponse
	printf("File sent successfully. Closing client data socket.\n");
	close(clientDataSocket);            // close the data line
}

/* Takes message buffer to be sent to client, a message, and the socket file descriptor. Copies
message into buffer and send to client. */


int sendResponse(char *sentMessage, char* message, int clientSocket) 
{
	strncpy(sentMessage, message, BUFFER);			// copy message into sentMessage buffer. 
	int r = sendNumber(clientSocket, (strlen(sentMessage)));	// Send size.
	r = sendMessage(clientSocket, sentMessage);		// send the reponse
}

/* -- BEGINNING MAIN PROGRAM -- */

int main(int argc, char *argv[])
{
	signal(SIGCHLD, sigintHandle); // start the signal handler

	char buffer[BUFFER];        /* buffer for messages in general. */
	char receivedMessage[BUFFER];  /* string to hold received Messages. */
	char sentMessage[BUFFER];	/* string to hold messages that will be sent. */
	int size;                   /* Variable to hold size of messages that are sent or received. */
	int status;                 /* Variable to hold status of messages that are sent or received. */
	int dataPortNumber; /* Variable to hold number of data port. */

	/* Verify number of arguments. Should be 2. First argument is the program name,
	and the second is the port number that the server should listen on. */

	if (argc != 2)
	{
		fprintf(stderr, "Invalid number of arguments. Usage: [PROGRAM NAME] [PORT NUMBER]\n\n");
		exit(0);
	}

	/* Validate port number. */

	int serverPort = portValidation(argv[1]);

	/* If port isn't valid, print appropriate error message. */

	if (serverPort < 0)
	{
		fprintf(stderr, "Invalid port number. Must be between 1024 and 65535.\n");
		exit(1);
	}

	/* Start up server, open socket, bind / listen on socket / port etc. */

	int sockDescriptor = startServer(serverPort);

	/* If starting the failure failed, print error message and exit. */

	if (sockDescriptor < 0)
	{
		fprintf(stderr, "Error. Failed to start server on Port %d\n", serverPort);
		exit(1);
	}

	printf("Starting server on Port %d\n", serverPort); /* Print message indicating the server is starting up on the particular port. */

	int pid; /* Used for child process. */

	/* Now we begin the while loop. We set the value to 1 because we should always listen for connections until it is terminated by an INT signal. */

	while (1)
	{
		int clientSocket = accept(sockDescriptor, NULL, NULL);

		/* If failed to accept client socket connection, we close it. */

		if (clientSocket < 0)
		{
			fprintf(stderr, "Error, failed to accept connection from client. Closing.\n");
			close(clientSocket);
		}

		/* If we reach this point, the client socket and connection is good, so we can use it. */

		else
		{
			/* fork process */
			pid = fork();

			/* If fork failed, exit and print error message. */

			if (pid < 0)
			{
				printf("Error encountered while trying to fork process. Terminating.\n");
				exit(1);
			}

			/* If not terminated, we are working with the child process now. */

			else if (pid == 0)
			{
				printf("Client connected. Beginning session.\n");

				int requestNumber = handleRequest(clientSocket, receivedMessage);
				printf("Request # is: %d\n", requestNumber);

				dataPortNumber = receiveNumber(clientSocket); /* Get port number that will be used in client data socket connection. */

				if (status == 0)  /* If message received. */
				{

					/* Open up new port for data. */
					int clientDataSocketFD = startServer(dataPortNumber);
					printf("Opening Data Connection on Port %d\n", dataPortNumber);

					// listen on it for the connection
					int clientDataSocket = accept(clientDataSocketFD, NULL, NULL);

					/* Handling -l list command here. */

					if (requestNumber == 1) 
					{
						char directory[BUFFER];

						/* Get full path for current working directory and put it in the buffer variable. */
						getcwd(buffer, BUFFER);


						/* get the contents of the directory. */ 

						DIR *dirpointer = NULL; /* Create directory pointer. */
						dirpointer = opendir(buffer); /* Open directory at buffer path. */
						struct dirent *dirstruct = NULL; /* Create directory structure. */

						while ((dirstruct = readdir(dirpointer)) != NULL) 
						{
							/* Filter out . and .. listings and build the directory list appropriately. */

							if ((strncmp(dirstruct->d_name, ".", strlen(dirstruct->d_name)) != 0) && (strncmp(dirstruct->d_name, "..", strlen(dirstruct->d_name)) != 0)) 
							{
								strncat(directory, "[", 1); /* left bracket.*/
								strncat(directory, dirstruct->d_name, strlen(dirstruct->d_name));  /* name. */
								strncat(directory, "] ", 2); /* right bracket. */
							}
						}
						
						/* Send response on control connection to set up sending the list via the data connection. */

						sendResponse(sentMessage, "DATA", clientSocket);

						/* Send directory list. */

						sendResponse(sentMessage, directory, clientDataSocket);
						close(clientSocket); /* Close client socket. */
						_exit(0); /* _exit(0) used because exiting child process. */
					}

					/* Next we handle the -g get command. In these cases, the requestNumber will equal 2 instead of 1. */

					else if (requestNumber == 2) 
					{
						/* Get filename into receivedMessage string. */

						size = receiveNumber(clientSocket);
						status = receiveMessage(clientSocket, receivedMessage, size);

						receivedMessage[size] = '\0'; /* Add null terminator.*/

						printf("Client has requested the following file: [%s]\n", receivedMessage);

						/* Validate file exists and get full path for current working directory. */
						getcwd(buffer, BUFFER);

						/* Get directory contents like we did with the listing command. */

						DIR *dirpointer = NULL;
						dirpointer = opendir(buffer);
						struct dirent *dirstruct = NULL;

						int validFile = 0; /* Initially set validFile to false. */

						while ((dirstruct = readdir(dirpointer)) != NULL) 
						{
							/* Compare the file name the client gave us to all file names. If it is found, set validFile to true. */

							if (strncmp(receivedMessage, dirstruct->d_name, max(strlen(dirstruct->d_name), strlen(receivedMessage))) == 0) 
							{
								validFile = 1;	/* Set valid file to true if the file is found. */
							}
						}

						// file exists
						if (validFile == 1) 
						{
							printf("[%s] is a valid filename\n", receivedMessage);
							
							/* Read the file into file buffer. */

							char filebuffer[MAXBUFFER];
							readFile(receivedMessage, filebuffer);

							/* Send response on control connection to set up sending the file via the data connection. */

							sendResponse(sentMessage, "DATA", clientSocket);

							/* Send file over data port.*/

							sendFile(filebuffer, clientDataSocket, clientSocket);

							printf("Transfer Complete\n");
							close(clientSocket); // just to be sure
							_exit(0); /* _exit(0) used because exiting child process. */

						}

						else 
						{
							printf("[%s] is an invalid filename\n", receivedMessage);

							/* Send error message through control connection. */

							sendResponse(sentMessage, "Requested file does not exist.", clientSocket);
							close(clientSocket);
							_exit(0); /* _exit(0) used because exiting child process. */
						}

					}

					/* If we've reached this section of code, the status of the message was 0, but the requestNumber wasn't 1 or 2.
					This means that an invalid command was entered, leading to a request number of -1. */

					else 
					{
						sendResponse(sentMessage, "Invalid command. Only -g and -l are valid commands.\n", clientSocket);
						close(clientSocket);
						_exit(0); /* _exit(0) used because exiting child process. */
					}
				}

				/* If we've reached this section of the code, the status of the message wasn't 0. Indicating there was some
				error in receiving the error, , so we sent an error message, then close the client socket and exit the child process.*/

				else
				{
					fprintf(stderr, "Error in receiving a message from the client.\n");
					close(clientSocket);
					_exit(0); /* _exit(0) used because exiting child process. */
				}
			}

			/* Back in parent process.*/

			else
			{
				/* Nothing here. We just need to finish the if - else if - else conditional chain. */
			}
		} /* End of the else statement at the beginning of the while loop. */

		
	} /* End of while loop. */
	
	return 0;
} /* End of main*/