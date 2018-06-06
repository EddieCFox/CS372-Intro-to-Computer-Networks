#!/bin/python

#Author: Eddie C. Fox
#Date: March 14, 2017

#CS 372 Project 2

#Description: This is a file transfer client that is designed to interact with a file transfer
#server that can be found as file ftserver.c. Please see the README for additional details and 
#for full citation of resources and references.

#Proper syntax:
#python ftclient.py [HOSTNAME] [SERVER PORT] [COMMAND] [DATA PORT] [FILENAME](if -g is chosen)

from socket import *
import sys
from struct import *
import os.path

# Port validation function. Used to validate server and data port numbers as between 1024 and 65535.
# Will return the port number if successful, and -1 if unsuccessful.

def portValidation(port):
    if int(port) < 1024 or int(port) > 65535:
        return -1
    
    else:
        return int(port)

# Port error message printing

def portError():
    print("Invalid port number. Port must be between 1024 and 65535.")

# Function to create socket. Will take a hostname and a portnumber and attempt
# to open a socket to the host server on the port number. Will return the socket file
# descriptor if successful, and return -1 if unsuccessful. 

# Syntax taken from "Creating a Socket" header in the Socket Programming HOWTO documentation.

def connectSocket(hostname, port):
    fileDescriptor = socket(AF_INET, SOCK_STREAM)
    
    try:
        fileDescriptor.connect((hostname, port))
    except:
        return -1
    
    return fileDescriptor

def connectToServer(hostname, port):
    if portValidation(port) < 1: #Here we call the port validation function on the port, and call the error if invalid.
        portError()
        sys.exit(0)
        
    fileDescriptor = connectSocket(hostname, port)
    
    if fileDescriptor < 0:
        print("Error connecting to host: " + hostname + " on port " + port)
        sys.exit(0)
    
    return fileDescriptor

# Now that we have handled connecting to the server, we need to create functions in order to send and receive messages. 
# Packing can be found in the struct python documentation.

# Takes socket file descriptor and message to be sent. Will send the size of the message
# and then the message. All of the following functions use the same general exception 
# format. That is, printing that there is an error, closing the file descriptor,
# and then exiting.

def sendMessage(sockDescriptor, message):
    
    size = len(message) # Size of the message to be sent
    
    try:
        # Formatting information can be find in struct documentation 7.3.2.2 - Format Characters
        
        packedData = pack('I', size) # Pack the size of the message in the format of unsigned integer. 
        sockDescriptor.send(packedData) # Send the size of the message.
        sockDescriptor.send(message) # Send the message
    
    # If there is an error, print there is an error, close the socket, and exit. 
    except: 
        print("Error with sending message.")
        sockDescriptor.close()
        sys.exit(1)

# Takes socket file descriptor and receives the message. Message is returned if successful.
# Otherwise, the program exits.

def receiveMessage(sockDescriptor):
    try:
        sizeData = sockDescriptor.recv(4) # Get size of message to be receive
        size = unpack('I', sizeData) # Unpack size data to get size in unsigned integer format.
        message = sockDescriptor.recv(size[0]) # Receive the message itself
        return message # Return the message
    
    except:
        print("Error with receiving message.")
        sockDescriptor.close()
        sys.exit(1)

# Besides functions to send and receive messages, we also need a function to send the port number over the connection.

def sendPortNumber(sockDescriptor, port):
    try:
        portData = pack('I', port) # Pack to prepare the sending of the port number in unsigned integer format
        sockDescriptor.send(portData) # Send the data which contains the port number.
    
    except:
        print("Error with sending port number.")
        sockDescriptor.close()
        sys.exit(1)

# We now create a function to send requests to the server from the client. A request contains both a port number
# to establish a data connection on and either a command or a filename. The later can be considered a message, 
# and explains why we created functions to send / receive messages and port numbers before we created the function
# to send requests.

def sendRequest(sockDescriptor, port, message):
    try:
        sendMessage(sockDescriptor, message) # Send the message
        sendPortNumber(sockDescriptor, port) # Send the port number
    
    except:
        print("Error sending request to the server.")
        sockDescriptor.close()
        sys.exit(1)

# The most complicated of the helper functions, the receiveFile function. 
# This function uses a socket file descriptor and a filename. It receives 
# the requested data from the server and writes it to a file if it doesn't already exist.

def receiveFile(sockDescriptor, fullFilename):
    
    # Here, we use the splitext function is os.path to split the full file name into the base filename 
    # and the extension of the file, and put them into two variables, filename and extension. 
    
    filename, extension = os.path.splitext(fullFilename)
    
    print("Attempting to receive file.")
    
    file = receiveMessage(sockDescriptor)
    
    # At this point, we need to create a way to handle if the file already exists, or a file with the given file name already exists. 
    # We use isfile to determine if a file with that name already exists already. If so, we will give the user the choice to overwrite
    # the file. If they decide to overwrite the file, then we don't need to do anything. But if they decide to to not overwrite it, then
    # we need to append something to the file, so we know it is a duplicate of the original file name. 
    
    # This is the reason why we split up the filename and the extension, so that we could append a duplicate note to the filename if necessary
    # before reattaching the modified filename with the extension. 
    
    if os.path.isfile(fullFilename): # If there is already a file with that name
        
        # Ask user if they would like to overwrite the file. Assign overwriteFile true or false depending on response
        # and use that value to decide if action needs to be taken or the file name can stay as is.
        
        userInput = raw_input("File with that name already exists. Overwrite? (Y/N)") 
        if userInput == 'Y':
            overwriteFile = 'True'
        else:
            overwriteFile = 'False'
        
        # If we aren't overwriting, we need to append "copy" to the filename so we know it is a copy. 
        
        if overwriteFile == False:
            filename = filename + "_copy"
            fullFilename = filename + extension
    
    # Now we take the file from the received message and write the data to the file indicated by the proper filename.
    # Information for writing files taken from PythonForBeginners
    
    target = open(fullFilename, 'w')
    target.write(file)
    
    print("File successfully written.")
    
# -- MAIN FUNCTION -- 

def main():
    
    # First, we must check the number of arguments. We know there must be at least 5 arguments, and no more than
    # 6 arguments (if they decide to request a file). 
    
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print("Improper number of arguments. Please consult README file for proper syntax.")
        sys.exit(0)
    
    else:
        
        # Create a socket for the TCP control connection between the client and server.
        
        controlSocket = connectToServer(sys.argv[1], int(sys.argv[2])) 
        
        # Create a variable to hold the command that will be sent to the server.
        
        command = sys.argv[3]
        
        # Send command to server via the request formatting we created earlier.
        
        sendRequest(controlSocket, int(sys.argv[4]), command)
        
        # Create a socket for the TCP data connection between the client and server.
        
        dataSocket = connectToServer(sys.argv[1], int(sys.argv[4]))
        
        # Create handler for if the command was -g
        
        if command == "-g": 
            try:
                filename = sys.argv[5] # Set filename variable according to argument for later placing into receiveFile.
            except:
                print("Error: Filename parameter not provided after -g command.")
                controlSocket.close()
                dataSocket.close()
                sys.exit(0)
            
            # Send filename to server via request interface.
            
            sendRequest(controlSocket, int(sys.argv[4]), filename)
            
            messageFromServer = receiveMessage(controlSocket) # Receive message using control socket and put it into messageFromServer variable.
            
            if messageFromServer == "DATA":
                
                # If there is data incoming from the server (as opposed to an error message), receive the file.
                
                receiveFile(dataSocket, filename)
            
            # Otherwise, we provide an error message.
            
            else:
                print("Error: File requested does not exist.")
        
        # If the command is not -g, then they either entered -l for list or improperly entered a command. We needed to 
        # handle file get commands first because it references sys.argv[5], which would only exist if they actually 
        # wanted to receive a file. Thus, we put that section first to avoid an error referencing an argument
        # that doesn't exist.
        
        else:
            
            messageFromServer = receiveMessage(controlSocket)
            
            # If the server has the listing, receive the directory listing through the data socket. 
            
            if messageFromServer == "DATA":
                messageFromServer = receiveMessage(dataSocket) # Receive the message through the data socket.
            
            print(messageFromServer) # Print message from server, regardless if it is data or not. 
        
        # Close data and control sockets before exiting program.
        
        print("Closing data socket connection to server.")
        
        dataSocket.close()
        
        print("Closing control socket connection to server.")
        
        controlSocket.close()
        
        print("Terminating connection between server and client.")
        
        sys.exit(0)
        
if __name__ == "__main__":
    main()