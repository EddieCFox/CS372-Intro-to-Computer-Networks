/***********************
** Author: Eddie C. Fox
** Date: February 18, 2017
** Description: This is the chatclient.c program to be run on host B with the following parameters:
** <program name> <hostname> <portnumber>. It will connect to host A, running the chatserve.py 
** program and attempt to chat with it. 
***********************/

/* Some standard includes from the beej networking guide. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/**********************
**                       void getClientName(char *parameter)
** Description: This function briefly queries the user to enter a 10 character 
** or less handle that will be used in all chat messages. 
**********************/

void getClientName(char *parameter) 
{
	printf("Please enter a 10 charachter handle to chat with.\n");
	scanf("%s", parameter);
}


/**********************
**      struct addrinfo* getAddressInfo(char *hostName, char *portNumber)
** Description: This function returns a pointer to an addrinfo structure, the details of which
** we create in this function. It takes the two arguments from the command line of starting the program
** (after the executable name), hostName and prtNumber. Beej guide used the terms hint and res for two 
** addrinfo structures, so I am just going with that. 
**********************/

struct addrinfo* getAddressInfo(char *hostName, char *portNumber)
{
	struct addrinfo hints, *res; /* Hints structure and res is a pointer to an addrinfo linked list structure that we will end up returning.*/
	int status; /* Variable to hold status indicator. */

	memset(&hints, 0, sizeof hints); /* Clear out all the fields of hints. Beej's guide says to make sure the structure is empty. */
	hints.ai_family = AF_INET; /* Specifies IPV4*/
	hints.ai_socktype = SOCK_STREAM; /* Specifies TCP protocol. */

	/* Calls getaddrinfo on the parameters that we passed in to the program. 
	** According to Beej's guide, the status should return 0, or there is an error, so we must check for this. 
	** If there is an error, we print it to stderr, using gai_sterror(status) to tell us what exactly the error is, then exit. */

	if ((status = getaddrinfo(hostName, portNumber, &hints, &res)) != 0)
	{
		fprintf(stderr, "Error with getaddrinfo: %s\n Try enter hostname and port again.\n", gai_strerror(status));
		exit(1);
	}

	return res;
}


/**********************
**                          int createSocket(struct addrinfo * res)
** Description: This function creates a socket using the addrinfo pointer, and returns 
** a socket descriptor integer. 

**********************/

int createSocket(struct addrinfo * res)
{
	int socketDescriptor;

	/* We call the socket function using the domain, type, and protocol as parameters. */

	socketDescriptor = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	/* We return the sock descriptor unless it is -1. In that case, we print to stderr and exit. */

	if (socketDescriptor == -1)
	{
		fprintf(stderr, "Some error occured while creating the socket.\n");
		exit(1);
	}

	else
	{
		return socketDescriptor;
	}
}
	
/**********************
**                          void connectSocket(int sockDescriptor, struct addrinfo * res)
** Description: This function connects a socket to the server using the sockDescriptor from createSocket() and 
** the address information from getAddressInfo().
**********************/

void connectSocket(int socketDescriptor, struct addrinfo * res)
{
	int status; /* integer variable to hold the status header.*/

	status = connect(socketDescriptor, res->ai_addr, res->ai_addrlen);

	/* We return the sock descriptor unless it is -1. In that case, we print to stderr and exit. */

	if (status == -1)
	{
		fprintf(stderr, "Some error occured while connecting socket to server.\n");
		exit(1);
	}
}

/**********************
**                  void exchangeHandles(int socketDescriptor, char * clientname, char *servername)
** Description: This function sends our clientname to the server, and recieves the servername from the server.
**********************/

void exchangeHandles(int socketDescriptor, char * clientname, char * servername)
{
	/* Send clientname to server.*/

	int sendMessage = send(socketDescriptor, clientname, strlen(clientname), 0); /* send(socket file descriptor, buffer of what to send, length, flags) */

	/* Recieve servername from server. */

	int recieveMessage = recv(socketDescriptor, servername, 10, 0); /* recv (socket file descriptor, buffer to store recieved message, length, flags) */
}

/**********************
**                      void chat(int socketDescriptor, char * clientname, char *servername)
** Description: This function facillitates a conversation between the client and the server in a while loop. 
**********************/

void chat(int socketDescriptor, char * clientname, char *servername)
{
	/* Create buffers to store the sent and recieve messages. Their values will be reset at the end of each while loop.*/
	char sendMessage[503];
	char recieveMessage[501];

	/* Clear out hte buffers and initialize the data in each element of the array to 0.*/

	memset(sendMessage, 0, sizeof(sendMessage));
	memset(recieveMessage, 0, sizeof(recieveMessage));

	/* We can create two error handlers here. One for send and for for recieve.*/
	/* send() returns the number of bytes that are actually sent, so we can compare that to -1. */
	/* recv() also returns the number of bytes returned to the buffer, but it also can say 0 to indicate the server is closed.*/
	/* We need to create a separate error handler for this. */

	int bytesSent;
	int bytesRecieved;

	fgets(sendMessage, 500, stdin); // Use fgets to clear newlines from standard input. 

	/* Begin while loop. */

	while (1)
	{
		printf(" %s> ", clientname);
		fgets(sendMessage, 502, stdin); /* Get input from client to send to server.*/

		/* If the string the user sent was \quit followed by an enter sign, we close the connection and leave the while loop. */
		/* We need to do \\ to escape character the back slashes.*/

		if (strcmp(sendMessage, "\\quit\n") == 0)
		{
			break;
		}

		else
		{
			bytesSent = send(socketDescriptor, sendMessage, strlen(sendMessage), 0); // If \quit was not entered, send the message we stored earlier.
		}

		if (bytesSent == -1) /* If there was an error sending the message, indicate as such. */
		{
			fprintf(stderr, "Error occured while sending message to server.\n");
			exit(1);
		}

		bytesRecieved = recv(socketDescriptor, recieveMessage, 500, 0);

		if (bytesRecieved == -1) /* If there was an error recieving the message, indicate as such.*/
		{
			fprintf(stderr, "Error occured while recieving message froms erver.\n");
			exit(1);
		}

		else if (bytesRecieved == 0) /*I If the server closed the connection, indicate as such.*/

		{
			printf("Server closed connection.\n");
			break;
		}
		
		else /* Otherwise, print the recieved message.*/

		{
			printf("%s> %s\n", servername, recieveMessage);
		}

		/* Clear out the buffers to prepare for the next send and recieve messages. */

		memset(sendMessage, 0, sizeof(sendMessage));
		memset(recieveMessage, 0, sizeof(recieveMessage));
	}

	close(socketDescriptor); /* Close socket connection between client and server. */
	printf("Closed connection between client and server.");
}


int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Invald syntax.\nUsage:<program_name> <host_name> <port_number>\n\n");
		exit(1);
	}

	char clientname[10];
	char servername[10];
	int socketDescriptor;
	getClientName(clientname);

	struct addrinfo * res = getAddressInfo(argv[1], argv[2]); /* Call the address info function using the hostname and the port number.*/

	socketDescriptor = createSocket(res); // Create socket using the information from the address info function. 

	/* Connects the socket to the server. */
	connectSocket(socketDescriptor, res);

	/* Exchange information so both the client and server know each others names. */

	exchangeHandles(socketDescriptor, clientname, servername);

	/* Begin the while loop between the serve and the client that will continue until one of them closes the connection. */

	chat(socketDescriptor, clientname, servername);

	freeaddrinfo(res); /* Free up the memory of the linked list to prevent memory links.*/
}