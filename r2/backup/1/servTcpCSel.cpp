#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>

using namespace std;

#define PORT 2728
#define MAX_CONN 32

int sayHello(int);
void connectareClient(int, pollfd[], int&);
int tratareClient(int);
void eliminaClient(pollfd[], int&, int);
int logare(int);
const char* executa(const char*);
bool maiSuntComenzi(const char*, const char*[], int);
char* interpretareComanda(const char*&, const char*[], const int);


bool si_si(const char* nume_comanda, char* const args[]) {
	int pid = fork();
	if(pid == 0) { // copil
		execvp(nume_comanda, args);
		exit(0);
	}
	int status;
	waitpid(pid, &status, 0);
	return WIFEXITED(status); // todo: macro pt exit code
}

void sau_sau(const char* nume_comanda, char* const args[]) {
	int pid = fork();
	if(pid == 0) { // copil
		execvp(nume_comanda, args);
		exit(0);
	}
	int status;
	waitpid(pid, &status, 0);
	// return WIFEXITED(status);
}

void sau() {

}

void mai_mic() {

}

void mai_mare() {

}

void punct_si_virgula() {

}

void debugPrintStr(const char* s) {
	printf("debugPrintStr: %s\n", s);
	for (int i = 0; i < strlen(s); ++i) {
		printf("%d -> %c\n", s[i], s[i]);
	}
	printf("\n");
}

void trim(char* &s) {
	for(int i = 0; i < strlen(s); i++) {
		if(s[i] != ' ')
			break;
		s++;
		i--;
	}

	for(int i = strlen(s)-1; i >= 0; i--) {
		if(s[i] != ' ')
			break;
		s[i] = 0;
	}
}

int main () {
	signal(SIGPIPE, SIG_IGN);

	pollfd clienti[MAX_CONN];

	// struct sockaddr_in from;
	// int client;		/* descriptori de socket */
	// int fd;			 descriptor folosit pentru 
	// 			   parcurgerea listelor de descriptori 

	int listener;
	if ((listener = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
	  perror ("[server] Eroare la socket().\n");
	  return errno;
	}

	int optval = 1; /* optiune folosita pentru setsockopt()*/ 
	/*setam pentru socket optiunea SO_REUSEADDR */ 
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,&optval, sizeof(optval));

	struct sockaddr_in listener_sockaddr_in;	/* structurile pentru listener */
	bzero (&listener_sockaddr_in, sizeof(listener_sockaddr_in));
	listener_sockaddr_in.sin_family = AF_INET;
	listener_sockaddr_in.sin_addr.s_addr = htonl (INADDR_ANY);
	listener_sockaddr_in.sin_port = htons (PORT);

	if (bind (listener, (struct sockaddr*) &listener_sockaddr_in, sizeof(struct sockaddr)) == -1) {
	  perror ("[server] Eroare la bind().\n");
	  return errno;
	}

	/* punem serverul sa asculte daca vin clienti sa se conecteze */
	if (listen (listener, 16) == -1) {
		perror ("[server] Eroare la listen().\n");
		return errno;
	}

	int nfds = 0;
	clienti[nfds].fd = listener;
	clienti[nfds].events = POLLIN;
	clienti[nfds].revents = 0;
	nfds++;

	printf ("[server] Asteptam la portul %d...\n", PORT);

	while(poll(clienti, nfds, -1)) {
		if(clienti[0].revents & POLLIN) {
			connectareClient(clienti[0].fd, clienti, nfds);
		}
		else if(clienti[0].revents & POLLHUP) {
			fprintf(stderr, "Eroare listener inchis: %d\n", errno);
			exit(1);
		}

		for(int i = 1; i < nfds; i++) {
			if(clienti[i].revents & POLLIN) {
				if(0 != tratareClient(clienti[i].fd)) {
					eliminaClient(clienti, nfds, i);
				}
			}
			else if(clienti[i].revents & POLLHUP) {
				eliminaClient(clienti, nfds, i);
			}
		}
	}
	return 0;
}

void connectareClient(int listener, pollfd clienti[], int &nfds) {
	struct sockaddr_in newClient_sockaddr_in;
	bzero (&newClient_sockaddr_in, sizeof(newClient_sockaddr_in));
	socklen_t len = sizeof(newClient_sockaddr_in);

	int newClient = accept(listener, (sockaddr*)&newClient_sockaddr_in, &len);
	if(newClient == -1) {
		fprintf(stderr, "Accept a esuat cu eroarea: %d\n", errno);
	}

	if(0 != logare(newClient)) {
		printf("Utilizator invalid\n");
		int raspuns = 1;
		send(newClient, &raspuns, 4, 0);
		close(newClient);
		return;
	}

	int raspuns = 0;
	send(newClient, &raspuns, 4, 0);

	printf("Client nou\n");

	if(nfds > MAX_CONN) {
		fprintf(stderr, "Nr maxim de clienti depasit\n");
		close(newClient);
		return;
	}
	clienti[nfds].fd = newClient;
	clienti[nfds].events = POLLIN;
	clienti[nfds].revents = 0;
	nfds++;
}

int tratareClient(int client) {
	char comanda[512];
	int offset = 0;

	do {
		int recvBytes = recv(client, comanda+offset, 1, 0);
		// printf("DEBUG - recvBytes: %d -> %c\n", recvBytes, comanda[offset]);

	// printf("iteratia %d\n", offset);
	// for(int i = 0; i < 512; i++) {
	// 	printf("%d|", comanda[i]);
	// }
	// printf("\n");

		if(recvBytes == -1) {
			fprintf(stderr, "Eroare la recv %d\n", errno);
			return 1;
		}
		if(recvBytes == 0) {
			fprintf(stderr, "Clientul s-a deconectat neanuntat\n");
			return 1;
		}
	}while(comanda[offset++] != 0);

	// for(int i = 0; i < 512; i++) {
	// 	printf("%d|", comanda[i]);
	// }
	// printf("\n");

	printf("Comanda: %s(%ld)\n", comanda, strlen(comanda));

	const char* raspuns = executa(comanda);
	if(raspuns != NULL && send(client, raspuns, strlen(raspuns)+1, 0) == -1) {
		fprintf(stderr, "Eroare la trimitere raspuns: %d\n", errno);
		return 1;
	}
	else {
		const char* msg = "Eroare la parsare comanda";
		send(client, msg, strlen(msg) + 1, 0);
		// return 1;
	}

	return 0;
}

void eliminaClient(pollfd clienti[], int &nfds, int i) {
	for(int j = i; j < nfds; j++) {
		clienti[i] = clienti[++i];
	}
	nfds--;
}

int logare(int client) {
	int offset = 0;
	char nume[100];

	do {
		int recvBytes = recv(client, nume+offset, 1, 0);
		// printf("DEBUG - recvBytes: %d -> %c\n", recvBytes, comanda[offset]);

		// printf("iteratia %d\n", offset);
		// for(int i = 0; i < 512; i++) {
		// 	printf("%d|", comanda[i]);
		// }
		// printf("\n");

		if(recvBytes == -1) {
			fprintf(stderr, "Eroare la recv %d\n", errno);
			return 1;
		}
		if(recvBytes == 0) {
			fprintf(stderr, "Clientul s-a deconectat neanuntat\n");
			return 1;
		}
	}while(nume[offset++] != 0);

	offset = 0;
	char parola[100];

	do {
		int recvBytes = recv(client, parola+offset, 1, 0);
		// printf("DEBUG - recvBytes: %d -> %c\n", recvBytes, comanda[offset]);

		// printf("iteratia %d\n", offset);
		// for(int i = 0; i < 512; i++) {
		// 	printf("%d|", comanda[i]);
		// }
		// printf("\n");

		if(recvBytes == -1) {
			fprintf(stderr, "Eroare la recv %d\n", errno);
			return 1;
		}
		if(recvBytes == 0) {
			fprintf(stderr, "Clientul s-a deconectat neanuntat\n");
			return 1;
		}
	}while(parola[offset++] != 0);

	// TODO validare...
	return 0;
}

const char* executa(const char* comanda) {
	const char* operatori[] = { "&&", "||", "|", "<", ">", ";" }; //n>
	const int nrOperatori = 6;

	// int count = 0;
	// while(strstr(comanda, operatori[count]) != NULL) {

	// }

	// if(count == 2) { // "|"

	// }



	while(maiSuntComenzi(comanda, operatori, nrOperatori)) {
		//TODO: sotcketpair();
		if(interpretareComanda(comanda, operatori, nrOperatori) == NULL)
			return NULL;
	}
	interpretareComanda(comanda, operatori, nrOperatori);
	return NULL;

}

char* interpretareComanda(const char* &comanda, const char* operatori[], const int nrOperatori) {
	const char* capat_comanda = comanda+strlen(comanda);
	int operatorCurent = -1;
	for(int i = 0; i < nrOperatori; i++) {
		const char* t = strstr(comanda, operatori[i]);
		fprintf(stderr, "debug - operatori[%d] - %s\n", i, t);
		if(t < capat_comanda && t != NULL) {
			capat_comanda = t;
			operatorCurent = i;
		}
	}

	fprintf(stderr, "comanda: %s   capat_comanda: %s    operatorCurent: %d\n", comanda, capat_comanda, operatorCurent);

	char* args[100];
	args[0] = NULL;
	int nrArgumente = 0;

	char* cpy_comanda = strndup(comanda, capat_comanda - comanda);
	{//parsare argumente
		trim(cpy_comanda);
		char* saveptr, *token = strtok_r(cpy_comanda, " ", &saveptr);
		args[0] = strdup(token);
		args[1] = NULL;
		nrArgumente = 1;
		while((token = strtok_r(NULL, " ", &saveptr)) != NULL){
			printf("token: \"%s\"\n", token);
			args[nrArgumente++] = strdup(token);
			args[nrArgumente] = NULL;
		}
	}

	printf("Nume comanda: %s    ", args[0]);
	for (int i = 0; i <= nrArgumente; i++){
		printf("%s ", args[i]);
	}
	printf("\n");

	switch(operatorCurent) {
		case 0: // &&
			if(si_si(args[0], args) == false){
				return NULL;
			}
			else break;
		case 1: // ||
			sau_sau(args[0], args);
			break;
		case 2: // |
			break;
		case 3: // <
			break;
		case 4: // >
			break;
		case 5: // ;
			break;
		// case 6: // n>
		// 	break;
		default:
			//printf("Eroare operatorCurent: %d\n", operatorCurent);
			if(si_si(args[0], args) == false){
				return NULL;
			}
	}

	if(operatorCurent != -1)
		comanda = capat_comanda + strlen(operatori[operatorCurent]);

	printf("\n\n");

	return strdup("ok");
}


bool maiSuntComenzi(const char* comanda, const char* operatori[], int dim_operatori) {
	for(int i = 0; i < dim_operatori; i++) {
		if(strstr(comanda, operatori[i]))
			return true;
	}

	return false;
}