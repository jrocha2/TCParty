all: myftpd

myftpd: server.c
	gcc server.c -o myftpd -lssl -lrt

clean:
	rm myftpd
