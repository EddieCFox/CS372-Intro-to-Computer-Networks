#!/bin/python

'''
Author: Eddie C. Fox
Date: February 15, 2017
'''

from socket import *
import sys

def exchangeHandles(connection, serverName):
    
    # Exchange handles with the incoming client. 
    
    clientName = connection.recv(1000) # Get clientname. 
    
    connection.send(serverName) # Send our server handle to 
    
    return clientName # Return the clientName to main.
    
def chatWithClient(connection, serverName, clientName):
    
    # Exchange messages with client, giving them the first message. 
        
    sendMessage = ""
    
    while True:
        recieveMessage = connection.recv(500)[0:-1] # Recieve message from client. 
        
        if (recieveMessage == ""): # If nothing is recieved, close the connection.
            print("Nothing recieved. Closing connection.")
            break
        
        # Print a properly formatted combination of client name and address
        
        print ("{}> {}".format(clientName, recieveMessage))
        
        
    # Read message from server user to respond to client. 
        
        sendMessage = raw_input("{}> ".format(serverName))
        
        if sendMessage == "\quit":
            print("\quit entered. Closing connection.")
            break
        
        connection.send(sendMessage)
        
# Defines the main function. 
    
def main():
    
    if (len(sys.argv) < 2):
        print("Please specify a port number for the server to run on.")
        exit(1)
    
    elif (len(sys.argv) > 2):
        print("Too many arguments. Format: chatserve portnumber")
        exit(1)
        
    else:
        serverPort = sys.argv[1] # Get port number from command line.
        
        serverSocket = socket(AF_INET, SOCK_STREAM) #create TCP socket
        
        serverSocket.bind(('', int(serverPort))) # Bind socket to port from args
        
        serverSocket.listen(1) # Will accept up to 1 connection at a time before refusing others. No queue.
        
        print("Server now listening on Port {}".format(sys.argv[1]))
        
        serverName = "Chatot" # Chatot is a bird that repeats things it hears.
        
        while True:
            connection, address = serverSocket.accept() # Accept the incoming client
            print("Connection successfully established on address {}".format(address)) # Print the address the connection is established on.
            
            clientName = exchangeHandles(connection, serverName)
            chatWithClient(connection, serverName, clientName)
            connection.close()
            exit(0)
            
# Executes main if ran as a standalone script. 

if  (__name__ == '__main__'):
    main()