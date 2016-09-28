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

int main(int argc, char** argv) {

    // Check for valid input arguments
    if (argc != 3) {
        fprintf(stderr, "usage: ./myftp serverName portNumber\n");
        exit(1);
    }

    int s;
    struct hostent *hp;
    struct sockaddr_in sin;

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

    char buf[4096];
    int len;
    while(fgets(buf, sizeof(buf), stdin)) {
        buf[4095] = '\0';
        len = strlen(buf) + 1;
        if (send(s, buf, len, 0) == -1) {
            perror("client send error!");
            exit(1);
        }
    }

    close(s);

    return 0;
}
