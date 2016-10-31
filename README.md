# TCParty

Mary Connolly mconnol6
John Rocha jrocha2
Paul Dowling pdowling

The included files in this repository are:
  1.) This README
  2.) client directory which includes:
          Makefile
          client.c
  3.) server directory whih includes:
          Makefile
          server.c
In order to compile the programs, just type "make" in the command line, "make clean" will remove the executable

To properly use the client, the server should be running first in order to process the commands used on the client side. 

To execute the server:
./myftpd [PORT NUMBER]

To execute the client:
./myftp Server_Name [PORT NUMBER]

Once both executables are running, inputs are given on the client side. The client will wait for an input that matches one of the following:

REQ  requests a file from the server
UPL  upload a file to the server
DEL  deletes a file from the server
LIS  lists the directory at the server
MKD  make a directory at the server
RMD  remove a directory at the server
CHD  change a directory at the server
XIT  exit the server

depending on the input the user gives the client will wait for the user to input the file or directory that they wish to alter. Non-existant files or directories will result in an error.

Helpful sources:
http://stackoverflow.com/questions/1220046/how-to-get-the-md5-hash-of-a-file-in-c
