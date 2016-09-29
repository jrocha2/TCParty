// client.c
// John Rocha, Mary Connolly, Paul Dowling
// jrocha2, mconnol6, pdowling


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>

int s;  // Global variable for socket fd

int receive_string(char *);
void send_string(char *);
void request_file();
void upload_file();
void list_dir();
void make_dir();
void remove_dir();
void change_dir();
void delete_file();

int main(int argc, char** argv) {

    // Check for valid input arguments
    if (argc != 3) {
        fprintf(stderr, "usage: ./myftp serverName portNumber\n");
        exit(1);
    }

    struct hostent *hp;
    struct sockaddr_in sin;
    char buf[100];

    // Translate host name into IP address
    hp = gethostbyname(argv[1]);
    if (!hp) {
        fprintf(stderr, "myftp: unknown host: %s\n", argv[1]);
        exit(1);
    }

    // Build socket-in address
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(atoi(argv[2]));

    // Open socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("myftp: socket\n");
        exit(1);
    }

    // Connect to server
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("myftp: connect\n");
        close(s);
        exit(1);
    }

    printf("\nEnter an operation: ");

    while(fgets(buf, sizeof(buf), stdin)) {
        if (!strncmp(buf, "REQ", 3)) {
            request_file();       
        } else if (!strncmp(buf, "UPL", 3)) {
            upload_file();
        } else if (!strncmp(buf, "LIS", 3)) {
            list_dir();
        } else if (!strncmp(buf, "MKD", 3)) {
            make_dir();
        } else if (!strncmp(buf, "RMD", 3)) {
            remove_dir();
        } else if (!strncmp(buf, "CHD", 3)) {
            change_dir();
        } else if (!strncmp(buf, "DEL", 3)) {
            delete_file();
        } else if (!strncmp(buf, "XIT", 3)) {
            close(s);
            printf("\nThe session has now been closed.\n");
            return 0;
        } else {
            printf("\nInvalid operation\n");
        }

        printf("\nEnter an operation: ");
    }

    close(s);
    return 0;
}

int receive_string(char* buffer) {
    int len;
    if ((len = recv(s, buffer, sizeof(buffer), 0)) == -1) {
        perror("Receive error!\n");
        close(s);
        exit(1);
    }
    return len;
}

void send_string(char* buffer) {
    if (send(s, buffer, strlen(buffer)+1, 0) == -1) {
        perror("Client send error!\n");
        close(s);
        exit(1);
    }
}

void request_file() {

}

void upload_file() {

}

void list_dir() {
    int bytes_read = 0, size = 0;
    char buf[256];
    strcpy(buf, "LIS");
    send_string(buf);

    if (recv(s, &size, sizeof(int32_t), 0) == -1) {
        perror("Receive error!\n");
        close(s);
        exit(1);
    }
    size = ntohl(size);
    
    bzero(buf, sizeof(buf));
    while (bytes_read < size) {
        bytes_read += receive_string(buf);
        printf("\n%s", buf);
        bzero(buf, sizeof(buf));
    }  
}

void make_dir() {

}

void remove_dir() {

}

void change_dir() {

}

void delete_file() {

}
