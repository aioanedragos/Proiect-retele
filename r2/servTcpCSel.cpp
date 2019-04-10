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
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include "tinyxml2.h"

using namespace std;
using namespace tinyxml2;

#define PORT 2728
#define MAX_CONN 32

void connectareClient(int, pollfd[], int&);
int tratareClient(int);
void eliminaClient(pollfd[], int&, int);
int logare(int);
void executa(int, const char*);
bool maiSuntComenzi(const char*, const char*[], int);
char* interpretareComanda(int, const char*&, const char*[], const int,int []);


/*
A; B    # Run A and then B, regardless of success of A
A && B  # Run B if and only if A succeeded
A || B  # Run B if and only if A failed
*/

void criptare(char* input){
	for(int i = 0;i< strlen(input); i++){
		input[i]++;
	}
}

void decriptare(char* input){
		for(int i = 0;i< strlen(input); i++){
		input[i]--;
	}
}


int si_si(int client, const char* nume_comanda, char* const args[],int p[], const char* fisier = NULL, int tip_fisier = 0) {
	int pipe_rez[2];
	pipe(pipe_rez);

	int pid = fork();
	if(pid == 0) { // copil
		if(fisier != NULL) {
			if(tip_fisier == 0) { // input
				int fd = open(fisier,  O_RDONLY);
				dup2(fd, 0); // stdin = 0
				dup2(pipe_rez[1],1);
				dup2(pipe_rez[1],2);
			}
			else if(tip_fisier == 1) { // output
				int fd = creat(fisier, 0655);
				dup2(fd, 1); // stdout = 1
			}
		}
		else {
			dup2(pipe_rez[1],1);
			dup2(pipe_rez[1],2);

		}
		close(pipe_rez[0]);
		if(p != NULL){
			close(p[1]);
			dup2(p[0],0);
		}
		execvp(nume_comanda, args);
		exit(0);
	}
	if(p != NULL){
		close(p[1]);
		close(p[0]);
	}

	close(pipe_rez[1]);
	char raspuns[1024];
	int r = read(pipe_rez[0], raspuns, 1024);
	raspuns[r] = 0;
	//printf("raspuns si_si: %s\n", raspuns);
	criptare(raspuns);
	send(client, raspuns, strlen(raspuns)+1, 0);

	int status;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status); // todo: macro pt exit code
}

int sau(const char* nume_comanda, char* const args[],int p[]) {
	int pid = fork();
	if(pid == 0) { // copil
		close(p[0]);
		dup2(p[1],1);
		execvp(nume_comanda, args);
		exit(0);
	}
	close(p[1]);
	int status;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

int sau_sau(int client, const char* nume_comanda, char* const args[], int p[],const char* fisier = NULL, int tip_fisier = 0) {
	int pipe_rez[2];
	pipe(pipe_rez);

	int pid = fork();
	if(pid == 0) { // copil
		if(fisier != NULL) {
			if(tip_fisier == 0) { // input
				int fd = open(fisier,  O_RDONLY);
				dup2(fd, 0); // stdin = 0
				dup2(pipe_rez[1],1);
				dup2(pipe_rez[1],2);
			}
			else if(tip_fisier == 1) { // output
				int fd = creat(fisier, 0655);
				dup2(fd, 1); // stdout = 1
			}
		}
		else {
			dup2(pipe_rez[1],1);
			dup2(pipe_rez[1],2);
		}
		close(pipe_rez[0]);
		if(p != NULL){
			close(p[1]);
			dup2(p[0],0);
		}
		execvp(nume_comanda, args);
		exit(0);
	}
	if(p != NULL){
		close(p[1]);
		close(p[0]);
	}

	close(pipe_rez[1]);
	char raspuns[1024];
	int r = read(pipe_rez[0], raspuns, 1024);
	raspuns[r] = 0;
	//printf("raspuns si_si: %s\n", raspuns);
	criptare(raspuns);
	send(client, raspuns, strlen(raspuns)+1, 0);

	int status;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status); // todo: macro pt exit code
}

int punct_si_virgula(int client, const char* nume_comanda, char* const args[], int p[],const char* fisier = NULL, int tip_fisier = 0) {
	int pipe_rez[2];
	pipe(pipe_rez);

	int pid = fork();
	if(pid == 0) { // copil
		if(fisier != NULL) {
			if(tip_fisier == 0) { // input
				int fd = open(fisier,  O_RDONLY);
				dup2(fd, 0); // stdin = 0
				dup2(pipe_rez[1],1);
				dup2(pipe_rez[1],2);
			}
			else if(tip_fisier == 1) { // output
				int fd = creat(fisier, 0655);
				dup2(fd, 1); // stdout = 1
			}
		}
		else {
			dup2(pipe_rez[1],1);
			dup2(pipe_rez[1],2);
		}
		close(pipe_rez[0]);
		if(p != NULL){
			close(p[1]);
			dup2(p[0],0);
		}
		execvp(nume_comanda, args);
		exit(0);
	}
	if(p != NULL){
		close(p[1]);
		close(p[0]);
	}

	close(pipe_rez[1]);
	char raspuns[1024];
	int r = read(pipe_rez[0], raspuns, 1024);
	raspuns[r] = 0;
	//printf("raspuns si_si: %s\n", raspuns);
	criptare(raspuns);
	send(client, raspuns, strlen(raspuns)+1, 0);

	int status;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status); // todo: macro pt exit code
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

	if(0 == logare(newClient)) {
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
	decriptare(comanda);
	printf("Comanda: %s(%ld)\n", comanda, strlen(comanda));
	executa(client, comanda);
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

	// TODO validare.
	tinyxml2::XMLDocument doc;
	doc.LoadFile("SavedData.xml");
	int valid = 0;
	for( const tinyxml2::XMLElement* child = doc.FirstChildElement("Utilizator")->FirstChildElement("data"); 
		 child; 
		 child = child->NextSiblingElement())
	{
		if(child->Attribute("Username") && child->Attribute("Parola")){
			
			const char* nume_xml;
			nume_xml = child->Attribute("Username");
			const char* parola_xml;
			parola_xml = child->Attribute("Parola");
			std::cout << "Username = "        << nume_xml   <<' '    << strlen(nume_xml) <<"Nume: " <<nume <<std::endl;
			if(strcmp(nume_xml,nume) == 0 && strcmp(parola_xml,parola) == 0)
				valid = 1;
}

		// assuming you want UID as an unsigned:
		// unsigned uid = 0;
		// child->QueryAttribute( "UID", &uid);
		// std::cout << "UID = "        << uid        << std::endl;

		std::cout << std::endl;
	}
	if(valid == 1)
		return 1;
	return 0;

	

 // 	while(!fisier_logare.eof()) {
	// 	char u[32];
	// 	fisier_logare.getline(u, 32);
	// 	printf("n: %s    nume: %s\n", u, nume);

	// 	char p[32];
	// 	fisier_logare.getline(p, 32);
	// 	printf("p: %s    parola: %s\n", p, parola);

	// 	if(strcmp(u,nume) == 0 && strcmp(p,parola) == 0){
	// 		return 1; 
	// 	}	
	// }

	//XML
	//tinyxml2::XMLDocument xml_doc;
	// XMLDocument xmlDoc("logat.xml");
	// XMLNode *pNode=NULL;
	// XMLElement *pRootElm=xmlDoc.NewElement("utilizator");
	// pNode=xmlDoc.InsertEndChild(pRootElm);
	// pRootElm=pNode->ToElement();

	// XMLElement *pChildElm=xmlDoc.NewElement("map");
	
	// 	char* u8node;
	// 	pChildElm->SetAttribute("mapid","nume");
	// 	pChildElm->SetAttribute("name","nume1");
	// 	pChildElm->SetAttribute("filename","nume2");
	// 	pChildElm->SetAttribute("md5","nume3");
	// 	pNode=pRootElm->InsertEndChild(pChildElm);
		
	
	// xmlDoc.SaveFile("logat.xml");



	//Terminare xml




	// terminare
	//return 0;
}

void executa(int client, const char* comanda) {
	const char* operatori[] = { "&&", "||", "|", ";" }; // "<", ">" n>
	const int nrOperatori = 4;

	char* mesaj = NULL;
	while(maiSuntComenzi(comanda, operatori, nrOperatori)) {
		int p[2];
		if(pipe(p) == -1){
			printf("Eroare la pipe\n");
			exit(1);
		}
		if(mesaj != NULL) {
			write(p[0], mesaj, strlen(mesaj));
		}
		if((mesaj = interpretareComanda(client, comanda, operatori, nrOperatori,p)) == NULL) {
			const char * final = "*";
			send(client, final, strlen(final) + 1, 0);
			return;
		}
	}
	int p[2];
	if(pipe(p) == -1){
		printf("Eroare la pipe\n");
		exit(1);
	}
	if(mesaj != NULL) {
		printf("mesaj: %s\n", mesaj);
		write(p[0], mesaj, strlen(mesaj));
	}
	interpretareComanda(client, comanda, operatori, nrOperatori,p);

	const char * final = "*";
	send(client, final, strlen(final) + 1, 0);
}

char* interpretareComanda(const int client, const char* &comanda, const char* operatori[], const int nrOperatori,int p[]) {
	const char* capat_comanda = comanda+strlen(comanda);
	int operatorCurent = -1;
	for(int i = 0; i < nrOperatori; i++) {
		const char* t = strstr(comanda, operatori[i]);
		fprintf(stderr, "debug - operatori[%d] - %s\n", i, t);
		if(t < capat_comanda && t != NULL) {
			operatorCurent = i;

			// char* cpy_comanda = strdup(t);
			// if(operatorCurent == 3 || operatorCurent == 4) {
			// 	capat_comanda = comanda + (strtok(cpy_comanda+strlen("< "), " ") - cpy_comanda);
			// 	printf("debug - < > - capat_comanda: %s\n", capat_comanda);
			// }
			// else
				capat_comanda = t;
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

	char* input_file = NULL, *output_file = NULL;

	const char* operatori_tabela_descriptori[] = { "<", ">" };
	const int nr_operatori_tabela_descriptori = 2;

	for(int i = 1;; i++) {
		if(args[i] == NULL)
			break;

		if(strcmp(args[i], operatori_tabela_descriptori[0]) == 0) {
			input_file = strdup(args[i+1]);
			args[i+1] = NULL;
			args[i] = NULL;
			break;
		}
		else if(strcmp(args[i], operatori_tabela_descriptori[1]) == 0) {
			output_file = strdup(args[i+1]);
			args[i+1] = NULL;
			args[i] = NULL;
			break;
		}
	}

	printf("Nume comanda: %s    ", args[0]);
	for (int i = 0; i <= nrArgumente; i++){
		printf("%s ", args[i]);
	}
	printf("\n");


	char raspuns[1024];


	int cod_exit, r;
	switch(operatorCurent) {
		case 0: // &&
			if(input_file != NULL)
				cod_exit = si_si(client, args[0], args, p,input_file, 0);
			else if(output_file != NULL)
				cod_exit = si_si(client, args[0], args,p, output_file, 1);
			else
				cod_exit = si_si(client, args[0], args,p);

			printf("Cod exit: %d\n", cod_exit);

			if(cod_exit != 0){
				return NULL;
			}
			else break;
		case 1: // ||
			if(input_file != NULL)
				cod_exit = sau_sau(client,args[0], args,p, input_file, 0);
			else if(output_file != NULL)
				cod_exit = sau_sau(client,args[0], args,p, output_file, 1);
			else
				cod_exit = sau_sau(client,args[0], args,p);

			if(cod_exit == 0) {
				return NULL;
			}
			break;
		case 2: // |
			
			sau(args[0], args,p);

			r = read(p[0],raspuns,1024);
			raspuns[r] = 0;
			printf("raspuns: %s\n", raspuns);
			close(p[0]);

			if(operatorCurent != -1)
			comanda = capat_comanda + strlen(operatori[operatorCurent]);

			printf("\n\n");

			return strdup(raspuns);
		case 5: // ;
			if(input_file != NULL)
				cod_exit = punct_si_virgula(client,args[0], args,p, input_file, 0);
			else if(output_file != NULL)
				cod_exit = punct_si_virgula(client,args[0], args, p,output_file, 1);
			else
				cod_exit = punct_si_virgula(client,args[0], args,p);
			break;
		default:
			//printf("Nici un operatorCurent gasit\n");
			if(input_file != NULL)
				cod_exit = si_si(client, args[0], args, NULL, input_file, 0);
			else if(output_file != NULL)
				cod_exit = si_si(client, args[0], args, NULL, output_file, 1);
			else
				cod_exit = si_si(client, args[0], args, NULL);

			if(cod_exit != 0){
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