#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>

#define BUFFER_SIZE 4096


void send_file(int socket_fd, const char *filename) {
    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    
    while ((bytes_read = read(file_fd, buffer, BUFFER_SIZE)) > 0) {

        ssize_t total_bytes_sent = 0;
        while (total_bytes_sent < bytes_read) {
            ssize_t bytes_sent = send(socket_fd, 
                                    buffer + total_bytes_sent, 
                                    bytes_read - total_bytes_sent, 
                                    0);
            
            if (bytes_sent < 0) {
                perror("Error sending file");
                close(file_fd);
                return;
            }
            total_bytes_sent += bytes_sent;
        }
    }

    if (bytes_read < 0) {
        perror("Error reading from file");
    }

    close(file_fd);
    printf("File sent successfully.\n");
}

int main(int argc, char **argv) {
        if (argc !=3) {
                printf("Usage: %s <dst_ip> <dst_port>\n", argv[0]);
                return 0;
        }
        // socket
        int cfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (cfd == -1) {
                perror("socket()");
                return -1;
        }
        struct sockaddr_in saddr;
        saddr.sin_port = htons(atoi(argv[2]));
        saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = inet_addr(argv[1]);

        // connet
        if (connect(cfd, (struct sockaddr*) &saddr, sizeof(struct sockaddr_in)) == -1) {
                perror("connect()");
                return -1;
        }
        #define MAX_MSG 256
	#define MAX_ARQ 8192
        char requisicao[MAX_MSG], resp[MAX_MSG], arq[MAX_ARQ];
        bzero(requisicao, MAX_MSG);
        printf("<telnet teletype>: ");
        fgets(requisicao, MAX_MSG, stdin);
	requisicao[strcspn(requisicao, "\n")] = '\0';
	
	char req_copy[MAX_MSG];
	strcpy(req_copy, requisicao);

	char* delimiters = " \n";
	char* tok = strtok(req_copy, delimiters);
        char* filename;
        if(tok){
                filename = strtok(NULL, delimiters);     
                if ((strcmp(tok, "GET") == 0)) {
                        if(filename != NULL) {
                                char clientDir[MAX_MSG] = "cliente/cache/";
                                strcat(clientDir, filename);
        
                                int fdTest = open(clientDir, O_RDONLY);
                                
                                // se entrar aqui significa que existe o arquivo em cache
                                if (fdTest != -1){
                                        struct stat st;
                                        if (fstat(fdTest, &st) == 0){
                                                time_t last_modify = st.st_mtime;
                                                bzero(requisicao, MAX_MSG);
                                                sprintf(requisicao, "GET %s %ld", filename, last_modify);
                                        } 
                                        close(fdTest);
                                } else {
                                        bzero(requisicao, MAX_MSG);
                                        sprintf(requisicao, "GET %s", filename);
                                }
                        }
                }
        }

        int ns, nr;
        // write
        ns = write(cfd, requisicao, MAX_MSG);
        if (ns == -1) {
                perror("write()");
                return -1;
        }
        
        // Send file for PUT command
        if (strcmp(tok, "PUT") == 0 && filename != NULL) {
                send_file(cfd, filename);
        }
        
        // read
	bzero(resp, MAX_MSG);
        nr = read(cfd, resp, MAX_MSG);
        if (nr == -1) {
                perror("read()");
                return -1;
        }
        char* delimiter2 = "\n";
	char* tokRes = strtok(resp, delimiter2); 
	if(tokRes){
                printf("\n%s\n", resp);
                if(strcmp(tokRes, "304 Not Modified") == 0){
                        char* tokTimestamp = strtok(NULL, delimiter2);
                        if (tokTimestamp != NULL) {
                                time_t timestampValue = (time_t)atol(tokTimestamp);
                                struct tm *tempo_local;
                                tempo_local = localtime(&timestampValue);
                                printf("Last-Modified: %s", asctime(tempo_local));
                            }
                }
                else if (strcmp(tokRes, "200 OK") == 0) {
                        int nt;
                        if (strcmp(tok, "GET") == 0) {
                                char path_file[200] = "cliente/cache/";
                                strcat(path_file, filename);
                                
                                FILE *file = fopen(path_file, "r");
                                if (file != NULL) {
                                        fclose(file);
                                        if(remove(path_file) != 0){
                                                perror("Erro ao remover o arquivo antigo da cache");
                                        }
                                        else{
                                                nt = read(cfd, arq, MAX_ARQ);
                                                if (nt == -1) {
                                                        perror("read()");
                                                        return -1;
                                                }
                                                arq[nt] = '\0';
                                                printf("\n%s", arq);
                        
                                                FILE* f = fopen(path_file, "wb");
                                                if (f == NULL) {
                                                        perror("Erro ao receber o arquivo");
                                                }
                                                fprintf(f, "%s\n", arq);   
                                        }
                                }
                                else{
                                        nt = read(cfd, arq, MAX_ARQ);
                                        if (nt == -1) {
                                                perror("read()");
                                                return -1;
                                        }
                                        arq[nt] = '\0';
                                        printf("\n%s", arq);
                
                                        FILE* f = fopen(path_file, "wb");
                                        if (f == NULL) {
                                                perror("Erro ao receber o arquivo");
                                        }
                                        fprintf(f, "%s\n", arq);
                                }

                        }
                }
        }
        // close
        close(cfd);
        return 0;
}



