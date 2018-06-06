Make chatserve.py by typing in chmod +x chatserve.py

On the server instance, execute the program by typing in python chatserve.py <portnumber>, where portnumber is the port number you want the server to listen on.

Type gcc -o chatclient chatclient.c to compile chatclient.c 

Type chatclient <hostname> <portnumber>, where hostname is the hostname of the server and portnumber matches the portnummber specified in the arguemnts of the server.

Enter a handle. Connection will then be established by the server. Take turns typing. Whenever \quit is entered on either program, the connection between 
the server and the client will end. 