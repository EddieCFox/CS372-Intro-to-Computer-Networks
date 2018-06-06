USAGE INSTRUCTIONS: 

First, compile the ftserver with the makefile by typing "make ftserver" and hitting enter.

Then type in "ftserver" and a port number to make the server start listening on a port.  

Run the client by using: python ftclient.py [HOSTNAME] [SERVER PORT] [COMMAND] [DATA PORT] [FILENAME](if -g is chosen)

Sources used: 

-- PYTHON RESOURCES --

http://stackoverflow.com/questions/419163/what-does-if-name-main-do
Did not cite this properly for project 1. I had seen this several times and decided to implement it into my own projects. 

http://sebastianraschka.com/Articles/2014_python_2_3_key_diff.html 
Used to remind me of the big differences between 2.7 and 3.6. I learned on Project 1 that the server used 2.7, while I was used to programming in 3.6.

https://docs.python.org/3/tutorial/errors.html
Used to teach me how to handle exceptions in Python. Using this, I learned about the Try and Except commands. 

http://stackoverflow.com/questions/6501121/difference-between-exit-and-sys-exit-in-python
Although I used exit in Project 1, I learned since then that sys.exit is intended for use in programs.

https://docs.python.org/2/howto/sockets.html 
for additional information on python socket programming than the socket information found in the book. 

https://docs.python.org/2/library/struct.html 
for information on structs in python

http://stackoverflow.com/questions/82831/how-do-i-check-whether-a-file-exists-using-python
Told me that I should use os.path to check if a path is a file using the isfile function. 

https://docs.python.org/2/library/os.path.html 
To confirm the stack overflow information on the os.path module, which we use the isfile function to determine if the given path is a file or not. Also gave me information on other
useful os.path functions such as splitext, which allowed me to modify the filename in case of duplicates.
Note: I used import os.path instead of import os because the documentation mentioned that you should import os.path directly.

http://www.pythonforbeginners.com/files/reading-and-writing-files-in-python
Provided me information for proper syntax for reading and writing files in python. 

-- C RESOURCES --

C didn't require as many resources as I am fairly familiar with C, and also practiced many similar operations in my Operating Systems class last quarter. 

http://stackoverflow.com/questions/2811006/what-is-a-good-buffer-size-for-socket-programming 
For determining how large to make the buffers. 

http://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
For how to read files into buffers. 

http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
Consulted a lot during this project. Lots of code from project 1 was re-used in some fashion, and the code from project 1 largely came from this guide. 
Some new stuff I did with it was using setsockopt(), detailed in Section 9.20 

http://www.thegeekstuff.com/2012/06/c-directory/
Used to help me understand how to program Linux directories in C. 


