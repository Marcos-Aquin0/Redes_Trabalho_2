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
	if (tok && (strcmp(tok, "GET") == 0)) {
		char* filename = strtok(NULL, delimiters);
		if(filename != NULL) {
			char clientDir[MAX_MSG] = "cliente/";
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
	//get arquivo, verifica cache

        //verifica data d arquivo e envia junto com o get
        //le a resposta, se nao modificou, só utiliza do cache
        //se modificou ira receber outro arquivo para substituir
        
        //put arquivo, envia o arquivo pelo socket e espera resposta
        //verifica se o arquivo existe
        
        //delete arquivo e espera resposta

        //outro comando nao faz nada

        int ns, nr;
        // write
        ns = write(cfd, requisicao, MAX_MSG);
        if (ns == -1) {
                perror("write()");
                return -1;
        }
        // read
	bzero(resp, MAX_MSG);
        nr = read(cfd, resp, MAX_MSG);
        if (nr == -1) {
                perror("read()");
                return -1;
        }
	printf("\n%s", resp);
	char* tokRes = strtok(resp, delimiters); 
	if (strcmp(tokRes, "200") == 0) {
		int nt;
		if (strcmp(tok, "GET") == 0) {
			nt = read(cfd, arq, MAX_ARQ);
			if (nt == -1) {
				perror("read()");
				return -1;
			}
			arq[nt] = '\0';
			printf("\n%s", arq);
		
			FILE* f = fopen("cliente/arquivo_upload.txt", "wb");
			if (f == NULL) {
				perror("Erro ao receber o arquivo");
			}
			fprintf(f, "%s\n", arq);
		}
	}
        // close
        close(cfd);
        return 0;
}



