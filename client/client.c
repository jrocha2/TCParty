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

    int s, len;
    struct hostent *hp;
    struct sockaddr_in sin;
    char buf[4];

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
        buf[3] = '\0';

        if (!strcmp(buf, "REQ")) {
            request_file();       
        } else if (!strcmp(buf, "UPL")) {
            upload_file();
        } else if (!strcmp(buf, "LIS")) {
            list_dir();
        } else if (!strcmp(buf, "MKD")) {
            make_dir();
        } else if (!strcmp(buf, "RMD")) {
            remove_dir();
        } else if (!strcmp(buf, "CHD")) {
            change_dir();
        } else if (!strcmp(buf, "DEL")) {
            delete_file();
        } else if (!strcmp(buf, "XIT")) {
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

void request_file() {

}

void upload_file() {

}

void list_dir() {

}

void make_dir() {

}

void remove_dir() {

}

void change_dir() {

}

void delete_file() {

}
