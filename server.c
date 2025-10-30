#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#define MAX_REQ 64536
#define MAX_CONN 1000

struct targs {
	pthread_t tid;
	int cfd;
	struct sockaddr_in caddr;
};
typedef struct targs targs;

targs tclients[MAX_CONN + 3];

void init(targs *tclients, int n){
	int i;
	for (i = 0; i< MAX_CONN + 3; i++) {
		tclients[i].cfd = -1;
	}
}

void *handle_client(void *args) {
		int cfd = *(int *)args;
		// recv
		int nr, ns;
		char requisicao[MAX_REQ], resposta[MAX_REQ];
		bzero(requisicao, MAX_REQ);
		nr = recv(tclients[cfd].cfd, requisicao, MAX_REQ, 0);
		if (nr < 0) {
			perror("erro no recv()");
			return NULL;
		}

		printf("Recebeu do cliente (%s:%d): '%s'\n", inet_ntoa(tclients[cfd].caddr.sin_addr),
				ntohs(tclients[cfd].caddr.sin_port), requisicao);


		const char delimiters[] = " \n";
		char *command = strtok(requisicao, delimiters);
		char *filename = strtok(NULL, delimiters);
		char *timestamp = strtok(NULL, delimiters);
		
		
		if (strcmp(command, "GET") == 0) {
			strcpy(resposta, "200 OK\n");
			// "304 NOT MODIFIED\n"
			// "404 NOT FOUND\n"
			// "500 INTERNAL SERVER ERROR\n"
			//encaminhar arquivo junto
		
		} 
		else if (strcmp(command, "PUT") == 0) {
			if(filename == NULL){
				strcpy(resposta, "400 Bad Request\n");
			} else {
				char *old_filename = "cliente/";
				strcat(old_filename, filename);
				char path_file[200] = "servidor/";
				strcat(path_file, filename);
				if (rename(old_filename, path_file) == 0) {
					strcpy(resposta, "201 CREATED\n");
				} else {
					strcpy(resposta, "500 INTERNAL SERVER ERROR\n");
				}
			}
			
			strcpy(resposta, "200 OK\n");
			//se o arquivo ja existe, substituir
			// "201 CREATED\n"
			// "404 NOT FOUND\n"
			// "500 INTERNAL SERVER ERROR\n"
			//receber e salvar o arquivo

		} 
		else if (strcmp(command, "DELETE") == 0) {
			if(filename == NULL){
				strcpy(resposta, "400 Bad Request\n");
			} else {
				//apagar o arquivo
				char path_file[200] = "servidor/";
				strcat(path_file, filename);
				
				FILE* file = fopen(path_file, "r");
				if(file != NULL){
					fclose(file);
					if (remove(path_file) == 0){
						strcpy(resposta, "200 OK\n");
					} else {
						strcpy(resposta, "500 INTERNAL SERVER ERROR\n");
					}
				} else {
					strcpy(resposta, "404 NOT FOUND\n");
				}
			}
		} 
		else {
			strcpy(resposta, "400 BAD REQUEST\n");
		}

		// send
		ns = send(cfd, resposta, nr, 0);
		if (ns < 0){
			perror("erro no send()");
			return NULL;
		}
		// close
		close(tclients[cfd].cfd);

		tclients[cfd].cfd = -1;
		return NULL;
}

int main(int argc, char **argv) {

	if (argc != 2){
		printf("Uso: %s <porta> \n", argv[0]);
		return 0;
	}

	init(tclients, MAX_CONN+3);

	struct sockaddr_in saddr;

	// socket
	int sl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// bind
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(argv[1]));
	saddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sl, (const struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) < 0) {
		perror("erro no bind()");
		return -1;
	}

	if (listen(sl, 1000) < 0) {
		perror("erro no listen()");
		return -1;
	}

	// accept
	int cfd, addr_len, i;
	struct sockaddr_in caddr;
	addr_len = sizeof(struct sockaddr_in);

	while(1) {

		cfd = accept(sl, (struct sockaddr *)&caddr, (socklen_t *)&addr_len);
		if (cfd == 1) {
			perror("erro no accept()");
			return -1;
		}
		tclients[cfd].cfd = cfd;
		tclients[cfd].caddr = caddr;
		pthread_create(&tclients[cfd].tid, NULL, handle_client, (void*)&cfd);
	}
	for (i = 0; i< MAX_CONN + 3; i++) {
		if (tclients[i].cfd == -1) continue;
		pthread_join(tclients[i].tid, NULL);
	}
	return 0;
}
 
