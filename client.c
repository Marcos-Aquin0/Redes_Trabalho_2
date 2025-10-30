#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>



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
        char requisicao[MAX_MSG], resp[MAX_MSG];
        bzero(requisicao, MAX_MSG);
        printf("<telnet teletype>: ");
        fgets(requisicao, MAX_MSG, stdin);

        //get arquivo, verifica cache

        //verifica data d arquivo e envia junto com o get
        //le a resposta, se nao modificou, só utiliza do cache
        //se modificou ira receber outro arquivo para substituir
        
        //put arquivo, envia o arquivo pelo socket e espera resposta
        
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
        // close
        close(cfd);
        return 0;
}



