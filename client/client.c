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
void get_and_send_info();
void request_file();
void upload_file();
void list_dir();
void make_dir();
void remove_dir();
void change_dir();
void delete_file();
void delete_file_helper();
void print_md5sum(unsigned char *);
void create_file_in_chunks(char *, int);
void send_file_with_name(char *);

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
    printf("Received string: %s with len: %i\n", buffer, len);
    return len;
}

int receive_string_with_size(char *str, int size) {
    char buf[size];
    int bytes_read = 0;

    bzero(str, sizeof(str));
    while(bytes_read < size) {
        bzero(buf, sizeof(buf));
        bytes_read += receive_string(buf);
        strcat(str, buf);
    }

    printf("string: %s, bytes_read: %i\n", str, bytes_read);

    return bytes_read;
}

int receive_string_unknown_size(char *str) {
    char buf[4096];
    int bytes_read = 0;

    bzero(str, sizeof(str));
}

void create_file_in_chunks(char *filename, int file_size) {
    char buf[4096];
    int total_bytes_read = 0, len;

    //create new file
    FILE *f = fopen(filename, "w+"); 

    printf("here\n");

    if (f) {
        while(total_bytes_read < file_size) {
            bzero(buf, sizeof(buf));
            if ((len = recv(s, buf, sizeof(buf), 0)) == -1) {
                perror("\nReceive error");
                close(s);
                exit(1);
            }

            total_bytes_read += len;
            



            //printf("strlen: %i total read: %i placed in file: %s", strlen(buf), total_bytes_read, buf);
            //add to file
            fwrite(buf, sizeof(char), len, f);
        }

        fclose(f);
    }
}

void send_string(char* buffer) {
    if (send(s, buffer, strlen(buffer)+1, 0) == -1) {
        perror("Client send error!\n");
        close(s);
        exit(1);
    }
}

// Get user string input and sends length along with string
void get_and_send_info() {
    int16_t len;
    char buf[256];
    bzero(buf, sizeof(buf));   

    scanf("%s", buf);
    getchar();

    len = strlen(buf);
    len = htons(len);

    // Send directory/file name length
    if (send(s, &len, sizeof(int16_t), 0) == -1) {
        perror("\nSend error!");
        close(s);
        exit(1);
    }

    // Send name of directory/file
    send_string(buf);
}

void send_file_with_name(char *filename) {

    int16_t len;
    len = strlen(filename);
    len = htons(len);

    // Send directory/file name length
    if (send(s, &len, sizeof(int16_t), 0) == -1) {
        perror("\nSend error!");
        close(s);
        exit(1);
    }

    // Send name of directory/file
    send_string(filename);
}

void request_file() {

    char buf[256] = "REQ";
    int16_t len;
    int32_t file_size;
    unsigned char md5sum[100];

    //send REQ request to server
    send_string(buf);
    bzero(buf, sizeof(buf));

    printf("\nEnter the name of the file you want to download: ");
    char filename[256];
    bzero(buf, sizeof(filename));   

    scanf("%s", filename);
    getchar();

    send_file_with_name(filename);

    //receive result from server: 1 if file exists and -1 if not
    if (recv(s, &file_size, sizeof(int), 0) == -1) {
        perror("\nReceive error!");
        close(s);
        exit(1);
    }
    file_size = ntohl(file_size);

    if (file_size == -1) {
        printf("\nThe file does not exist on the server.\n");
        return;
    }

    //receive_string_with_size(md5sum, 16);

    //print_md5sum(md5sum);

    create_file_in_chunks(filename, file_size);
}

void upload_file() {

}

void list_dir() {
    int bytes_read = 0, size = 0, len = 0, i = 0, j = 0;
    char dir[256], buf[256] = "LIS";

    // Send LIS request to server
    send_string(buf);

    if (recv(s, &size, sizeof(int32_t), 0) == -1) {
        perror("Receive error!\n");
        close(s);
        exit(1);
    }
    size = ntohl(size);

    while (bytes_read < size) {
        bzero(buf, sizeof(buf));
        len = receive_string(buf);
        bytes_read += len;

        // Parse stream for end of strings
        for (i = 0; i < len; i++) {
            if (buf[i] == '\0') {
                printf("\n%s", dir);
                bzero(dir, sizeof(dir));
                j = 0;
            } else {
                dir[j++] = buf[i];
            }       
        }
    } 
    printf("\n"); 
}

void make_dir() {
    char buf[256] = "MKD";
    int16_t len;
    int server_result;

    // Send MKD request to server
    send_string(buf);
    bzero(buf, sizeof(buf));

    printf("\nEnter directory to be created: ");
    get_and_send_info();

    // Receive result of action from server
    if (recv(s, &server_result, sizeof(int), 0) == -1) {
        perror("\nReceive error!");
        close(s);
        exit(1);
    }
    server_result = ntohl(server_result);

    switch (server_result) {
        case -2:
            printf("\nThe directory already exists on the server.\n");
            break;
        case -1:
            printf("\nError in making directory.\n");
            break;
        default:
            printf("\nThe directory was successfully made.\n");
            break;
    }
}

void remove_dir() {
    char buf[256] = "RMD";
    int result;
    int16_t len;

    // Send RMD request to server
    send_string(buf);
    bzero(buf, sizeof(buf));

    printf("\nEnter directory to be deleted: ");
    get_and_send_info();

    // Receive initial result from server
    if (recv(s, &result, sizeof(int), 0) == -1) {
        perror("\nReceive error!");
        close(s);
        exit(1);
    }
    result = ntohl(result);

    if (result == -1) {
        printf("\nThe directory does not exist on the server\n");
        return;
    }

    printf("\nAre you sure you want to delete this directory (Yes/No): ");
    bzero(buf, sizeof(buf));
    scanf("%s", buf);
    getchar();

    // Send response to server
    send_string(buf);

    if (!strcmp(buf, "No")) {
        printf("\nDelete abandoned by the user!\n");
    } else {
        // Receive rmdir result from server
        if (recv(s, &result, sizeof(int), 0) == -1) {
            perror("\nReceive error!");
            close(s);
            exit(1);
        }
        result = ntohl(result);

        if (result < 0) {
            printf("\nFailed to delete directory\n");
        } else {
            printf("\nDirectory deleted\n");
        }
    }
}

void change_dir() {
    char buf[256] = "CHD";
    int16_t len;
    int result;

    // Send CHD request to server
    send_string(buf); 
    bzero(buf, sizeof(buf));

    printf("\nEnter directory path to navigate to: ");
    get_and_send_info();

    // Receive result from server
    if (recv(s, &result, sizeof(int), 0) == -1) {
        perror("\nReceive error!");
        close(s);
        exit(1);
    }
    result = ntohl(result);

    if (result == -2) {
        printf("\nThe directory does not exist on the server.\n");
    } else if (result == -1) {
        printf("\nError in changing directory.\n");
    } else {
        printf("\nChanged current directory.\n");
    }
}

void delete_file() {
    char buf[256] = "DEL";
    int16_t len;
    int result;

    //send DEL request to server
    send_string(buf);
    bzero(buf, sizeof(buf));

    printf("\nEnter file to delete: ");
    get_and_send_info();

    //receive result from server: 1 if file exists and -1 if not
    if (recv(s, &result, sizeof(int), 0) == -1) {
        perror("\nReceive error!");
        close(s);
        exit(1);
    }
    result = ntohl(result);

    if (result == -1) {
        printf("\nThe file does not exist on the server.\n");
    } else if (result == 1) {
        //call function to actually delete the file now that it exists
        delete_file_helper();
    } 
}

//Only call once the server has ensured that the file exists
//This function actually tells the server to delete the file
void delete_file_helper() {
    char buf[256];
    int result;

    printf("\nDo you want to delete the file? (Yes/No)\n");
    
    scanf("%s", buf);
    getchar();

    // Send response to server
    send_string(buf);

    if (!strcmp(buf, "No")) {
        printf("\nDelete abandoned by the user!\n");
    } else if (!strcmp(buf, "Yes")) {
        // Receive result from server
        if (recv(s, &result, sizeof(int), 0) == -1) {
            perror("\nReceive error!");
            close(s);
            exit(1);
        }
        result = ntohl(result);

        if (result < 0) {
            printf("\nFailed to delete file\n");
        } else {
            printf("\nFile deleted\n");
        }
    }
}

void print_md5sum(unsigned char *md) {
    int i;
    for (i=0; i<16; i++) {
        printf("%02x", md[i]);
    }
    printf("\n");
}
