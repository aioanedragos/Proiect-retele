#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>

void trimiteSir(int, char*);

int main (int argc, char *argv[]) {
	/* exista toate argumentele in linia de comanda? */
	if (argc != 3) {
	  printf ("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
	  return -1;
	}



	struct sockaddr_in server;	// structura folosita pentru conectare
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons(atoi (argv[2]));

	int sd;			// descriptorul de socket
	while(1) {
		char nume[100];		// citesc numele
		bzero (nume, 100);
		printf ("[client]Introduceti un nume: ");
		fflush (stdout);
		int readBytes = read(0, nume, 100);
		nume[readBytes-1] = 0;

		char parola[100];		// citesc parola
		bzero (parola, 100);
		printf ("[client]Introduceti un parola: ");
		fflush (stdout);
		readBytes = read(0, parola, 100);
		parola[readBytes-1] = 0;

		// for (int i = 0; i < readBytes; ++i) {
		// 	printf("DEBUG - %d: %d\n", i, nume[i]);
		// }

		if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
			perror ("[client] Eroare la socket().\n");
			return errno;
		}

		/* ne conectam la server */
		if (connect (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
			perror ("[client]Eroare la connect().\n");
			return errno;
		}

		trimiteSir(sd, nume);
		trimiteSir(sd, parola);

		int raspuns;
		int recvBytes = recv(sd, &raspuns, 4, 0);
		if(raspuns == 0)
			break;
		else
			printf("Logare esuata\n");

		close(sd);
	}

	printf("Logare reusita\n");

	// if (write (sd, nume, 2) <= 0) {
	//   perror ("[client]Eroare la write() spre server.\n");
	//   return errno;
	// }

	// if (read (sd, msg, 100) < 0) {
	//   perror ("[client]Eroare la read() de la server.\n");
	//   return errno;
	// }
	// printf ("[client]Mesajul primit este: %s\n", msg);


	while(1) {
		char comm[512];
		std::cin.getline(comm, sizeof(comm));
		printf("Comanda: %s\n", comm);

		if(send(sd, comm, strlen(comm)+1, 0) == -1) {
			fprintf(stderr, "Eroare la trimitere comanda: %d\n", errno);
			exit(1);
		}

		int offset = 0;
		do{
			int readBytes = recv(sd, comm+offset, 1, 0);
			if(readBytes == -1) {
				fprintf(stderr, "Eroare la primite raspuns: %d\n", errno);
				close(sd);
				exit(1);
			}
			if(readBytes == 0) {
				fprintf(stderr, "Deconectat de la server\n");
				close(sd);
				exit(1);
			}
		}while(comm[offset++] != 0);

		printf("Raspuns: %s\n", comm);
	}

	close (sd);
	return 0;
}


void trimiteSir(int socket, char* str) {
	int offset = 0;
	do{
		if(send(socket, str+offset, 1, 0) == -1) {
			fprintf(stderr, "[client]Eroare la send spre server: %d\n", errno);
			exit(1);
		}
		//printf("offset %d\n", offset);
	}while(str[offset++] != 0);
}