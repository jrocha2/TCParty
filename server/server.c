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
#include<sys/stat.h>
#include<dirent.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<openssl/md5.h>

#define MAX_PENDING 5
#define MAX_LINE 4096

int new_s; // global variable for socket

int receive_string(char *);
void send_string(char *);
void send_file_in_chunks(char *);
void receive_file_info(char *);
int get_command(char *, int new_s); 
void run_command(char *command);
void send_file();
void get_file();
void list_dir();
void make_dir();
void remove_dir();
void change_dir();
void delete_file();
void delete_file_helper(char *);
void get_md5sum(char *md5sum, char *filename, int32_t file_size); 
void print_md5sum(unsigned char *);
void read_file(char *filename, char *buf);
void send_file_in_chunks(char *);
double create_file_in_chunks(char *, int);
double calculate_throughput(struct timespec *start, struct timespec *end, int);

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
            bzero(command, sizeof(command));
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
int receive_string(char* buffer) {
    int len;
    if ((len = recv(new_s, buffer, sizeof(buffer), 0)) == -1) {
        perror("Receive error!\n");
        exit(1);
    }
    
    //buffer[len] = '\0';
    return len;
}

void send_string(char* buffer) {
    if (send(new_s, buffer, strlen(buffer), 0) == -1) {
        perror("Client send error!\n");
        exit(1);
    }
}

void send_file_in_chunks(char *filename) {
    //use as buffer
    char line[4096];
    int len;
    FILE *file = fopen(filename, "r"); 


    while((len=fread(line, sizeof(char), sizeof(line), file)) > 0) {
        if (send(new_s, line, len, 0) == -1) {
            perror("\nSend error!");
            exit(1);
        }
        bzero(line, sizeof(line));
    }
    fclose(file);
}

// Receives length of file/directory and then the name
void receive_file_info(char* dir) {
    int bytes_read = 0;
    int16_t len;
    char buf[256];

    // Receive length of string
    if (recv(new_s, &len, sizeof(int16_t), 0) == -1) {
        perror("\nReceive error!");
        exit(1);
    }
    len = ntohs(len);

    // Receive string
    bzero(dir, sizeof(dir));
    while (bytes_read < len) {
        bzero(buf, sizeof(buf));
        bytes_read += receive_string(buf);
        strcat(dir, buf);
    }
}

//returns 0 if quit, 1 otherwise
int get_command(char *command, int new_s) {

    if (receive_string(command) == 0) {
        return 0;
    }

    run_command(command);

    return 1;
}

void run_command(char *command ) {
    if (!strcmp(command, "REQ")) {
        send_file();
    } else if (!strcmp(command, "UPL")) {
        get_file();
    } else if (!strcmp(command, "LIS")) {
        list_dir();
    } else if (!strcmp(command, "MKD")) {
        make_dir();
    } else if (!strcmp(command, "RMD")) {
        remove_dir();
    } else if (!strcmp(command, "CHD")) {
        change_dir();
    } else if (!strcmp(command, "DEL")) {
        delete_file();
    } else if (!strcmp(command, "XIT")) {
    }
}

void send_file() {
    
    int32_t file_size;
    struct stat st;

    char filename[256];

    receive_file_info(filename);

    //determine if file exists
    if (access(filename, F_OK) != -1) {
        stat(filename, &st);
        file_size = st.st_size;
    } else {
        file_size = -1;
    }

    int file_size_htonl = htonl(file_size);

    //Tell client if the file exists
    if (send(new_s, &file_size_htonl, sizeof(int32_t), 0) == -1) {
        perror("\n Send error!");
        exit(1);
    }

    //if file does not exist, exit function
    if (file_size == -1) {
        return;
    }

    //calculate md5sum
    char md5sum[16];
    get_md5sum(md5sum, filename, file_size);

    //send md5sum
    if (send(new_s, md5sum, 16, 0) == -1) {
        perror("Client send error!\n");
        exit(1);
    }

    //send file
    send_file_in_chunks(filename);
}

void ack() {

    char buf[] = "ack";
    send_string(buf);
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

    return bytes_read;
}

void get_file() {
    int16_t len;
    int32_t file_size;
    char filename[256];
    unsigned char server_md5sum[17], client_md5sum[17];

    bzero(server_md5sum, sizeof(server_md5sum));
    bzero(client_md5sum, sizeof(client_md5sum));
    bzero(filename, sizeof(filename));

    receive_file_info(filename);

    //send ACK
    ack();

    //get file size
    if (recv(new_s, &file_size, sizeof(int32_t), 0) == -1) {
        perror("\nReceive error!");
        close(new_s);
        exit(1);
    }

    file_size = ntohl(file_size);
    
    //get md5sum
    receive_string_with_size(client_md5sum, 16);

    int throughput = create_file_in_chunks(filename, file_size);
    
    //calculate md5sum
    get_md5sum(server_md5sum, filename, file_size);

    int equal = 1;
    int i;

    for (i=0; i<16; i++) {
        if (server_md5sum[i] != client_md5sum[i]) {
            equal = 0;
        }
    }

    //if files do not match, set throughput to -1 and delete file
    if (equal == 0) {
        throughput = -1;
        unlink(filename);
    }

    throughput = htonl(throughput);

    if (send(new_s, &throughput, sizeof(int), 0) == -1) {
        perror("Send error!\n");
        exit(1);
    }
}

//return throughput
double create_file_in_chunks(char *filename, int file_size) {
    char buf[4096];
    int total_bytes_read = 0, len;
    double throughput = 0;
    struct timespec start, end;

    //create new file
    FILE *f = fopen(filename, "w+"); 

    if (f) {
        //get start time
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        while(total_bytes_read < file_size) {
            bzero(buf, sizeof(buf));
            if ((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
                perror("\nReceive error");
                close(new_s);
                exit(1);
            }

            total_bytes_read += len;
            
            //add to file
            fwrite(buf, sizeof(char), len, f);
        }

        //get end time
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);

        fclose(f);

        throughput = calculate_throughput(&start, &end, file_size);
    }

    return throughput;
}

double calculate_throughput(struct timespec *start, struct timespec *end, int file_size) {

    //this is the time change in microseconds
    double time_change = (end->tv_sec - start->tv_sec) * 1000000 + (double)(end->tv_nsec - start->tv_nsec) / 1000; 

    //filesize / time_change is Bytes / microsecond which is Megabytes per second
    return file_size / time_change;
}

void list_dir() {
    DIR *dp;
    struct dirent *ep;
    int size = 0;
    char buf[256];

    dp = opendir("./");
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            bzero(buf, sizeof(buf));
            strcpy(buf, ep->d_name);
            size += strlen(buf) + 1;
        }
        closedir(dp);
    } else {
        perror("Error opening directory");
    }
    size = htonl(size);

    // Send size of directory listing
    if (send(new_s, &size, sizeof(int32_t), 0) == -1) {
        perror("Send error!\n");
        exit(1);
    }

    dp = opendir("./");
    if (dp != NULL) {
        while (ep = readdir(dp)) {
            //send_string(ep->d_name);
            if (send(new_s, ep->d_name, strlen(ep->d_name) + 1, 0) == -1) {
                perror("Client send error!\n");
                exit(1);
            }
        }
        closedir(dp);
    } else {
        perror("Error opening directory");
    }
}

void make_dir() {
    int result, client_result;
    char dir[256];

    // Get directory name
    receive_file_info(dir);

    // Attempt to make directory
    result = mkdir(dir, 0777);
    if (result == 0) {
        client_result = 1;
    } else if (errno == EEXIST) {
        client_result = -2;
    } else {
        client_result = -1;
    }

    client_result = htonl(client_result);

    // Send result back to client
    if (send(new_s, &client_result, sizeof(int), 0) == -1) {
        perror("\nSend error!");
        exit(1);
    }
}

void remove_dir() {
    int result, client_result;
    char buf[256], dir[256];
    struct stat sb;

    // Get directory name
    receive_file_info(dir);

    if (stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        client_result = 1;
    } else {
        client_result = -1;
    }
    client_result = htonl(client_result);

    // Send if directory exists or not
    if (send(new_s, &client_result, sizeof(int), 0) == -1) {
        perror("\nSend error!");
        exit(1);
    }

    if (ntohl(client_result) == -1) {
        return;         // Exit function if dir does not exits
    }

    // Receive deletion confirmation
    bzero(buf, sizeof(buf));
    receive_string(buf);

    if (!strcmp(buf, "No")) {
        return;
    } else {
        result = rmdir(dir);
        if (result == 0) {
            client_result = 1;
        } else {
            client_result = -1;
        }
        client_result = htonl(client_result);

        // Send result to client
        if (send(new_s, &client_result, sizeof(int), 0) == -1) {
            perror("\nSend error!");
            exit(1);
        }
    }
}

void change_dir() {
    int result, client_result;
    char dir[256];

    // Get directory name
    receive_file_info(dir);

    result = chdir(dir);
    if (result == 0) {
        client_result = 1;
    } else if (errno == ENOENT) {
        client_result = -2;
    } else {
        client_result = -1;
    }
    client_result = htonl(client_result);

    // Send result to client
    if (send(new_s, &client_result, sizeof(int), 0) == -1) {
        perror("\nSend error!");
        exit(1);
    }
}

void delete_file() {
    int result, file_exists;

    char filename[256];

    receive_file_info(filename);

    //determine if file exists
    if (access(filename, F_OK) != -1) {
        file_exists = 1;
    } else {
        file_exists = -1;
    }

    file_exists = htonl(file_exists);

    //Tell client if the file exists
    if (send(new_s, &file_exists, sizeof(int), 0) == -1) {
        perror("\n Send error!");
        exit(1);
    }

    //exit function if file does not exist
    if (file_exists == -1) {
        return;
    } else {
        //call helper functiont to actunally delete file
        delete_file_helper(filename);
    }
}

//Called within delete_file() after it is ensure that filename exists
void delete_file_helper(char *filename) {

    char buf[256];
    int result, client_result;
    int confirmation;

    if (recv(new_s, &confirmation, sizeof(int), 0) == -1) {
        perror("Receive error!\n");
        exit(1);
    }

    confirmation = ntohl(confirmation);

    //if user does not want to delete, abandon
    if (confirmation == 0) {
        return;
    }

    //If yes, delete file
    result = unlink(filename);
    if (result == 0) {
        client_result = 1;
    } else {
        client_result = -1;
    }
    client_result = htonl(client_result);

    // Send result to client
    if (send(new_s, &client_result, sizeof(int), 0) == -1) {
        perror("\nSend error!");
        exit(1);
    }
}

void get_md5sum(char *md5sum, char *filename, int32_t file_size) {
    int file_descript = open(filename, O_RDONLY);
    char *file_buffer = mmap(0, file_size, PROT_READ, MAP_SHARED, file_descript, 0);
    MD5 ((unsigned char *) file_buffer, file_size, md5sum);
}

void read_file(char *filename, char *buf) {
	long length;
	FILE *f = fopen(filename, "r");

	if (f) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		fread(buf, 1, length, f);
		fclose(f);
	}
}

void print_md5sum(unsigned char *md) {
    int i;
    for (i=0; i<16; i++) {
        printf("%02x", md[i]);
    }
    printf("\n");
}
