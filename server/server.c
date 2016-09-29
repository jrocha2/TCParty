// server.c
// John Rocha, Mary Connolly, Paul Dowling
// jrocha2, mconnol6, pdowling

#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<time.h>
#include<stdint.h>
#include<sys/time.h>

#define MAX_PENDING 5
#define MAX_LINE 4096

int new_s; // global variable for socket

int receive_data(char *);
void send_data(char *);
int get_command(char *, int new_s); 
void run_command(char *command);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: myfptd [port number]\n");
        exit(1);
    }

    int port = atoi(argv[1]);

    struct sockaddr_in sin;
    int s;
    char buf[MAX_LINE]; //buffer for message
    int len;
    int opt = 1;

    //build address data structure
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    //create socket
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket\n");
        exit(1);
    }

    //set socket option
    if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(int))) < 0) {
        perror("simplex-talk: setsockopt\n");
        exit(1);
    }

    //bind socket to address
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind\n");
        exit(1);
    }

    //listen to socket
    if ((listen(s, MAX_PENDING)) < 0) {
        perror("simplex-talk: listen\n");
        exit(1);
    }

    while(1) {
        //accept incoming connection
        if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
            perror("simplex talk: accept\n");
            exit(1);
        }

        while(1) {

            //receive command
            char command[100];	
            int valid_command = get_command(command, new_s);
            if (valid_command == 0) {
                break;
            }
        }

        printf("Client finished, close the connection!\n");
        close(new_s);
    }

    return 0;
}

// Return the length of data received
int receive_data(char* buffer) {
    int len;
    if ((len = recv(new_s, buffer, sizeof(buffer), 0)) == -1) {
        perror("Receive error!\n");
        exit(1);
    }
    return len;
}

void send_data(char* buffer) {
    if (send(new_s, buffer, strlen(buffer)+1, 0) == -1) {
        perror("Client send error!\n");
        exit(1);
    }
}

//returns 0 if quit, 1 otherwise
int get_command(char *command, int new_s) {

    if (receive_data(command) == 0) {
        return 0;
    }

    run_command(command);

    return 1;
}

void run_command(char *command ) {
    if (!strcmp(command, "REQ")) {
    } else if (!strcmp(command, "UPL")) {
    } else if (!strcmp(command, "LIS")) {
    } else if (!strcmp(command, "MKD")) {
    } else if (!strcmp(command, "RMD")) {
    } else if (!strcmp(command, "CHD")) {
    } else if (!strcmp(command, "DEL")) {
    } else if (!strcmp(command, "XIT")) {
    }
}
