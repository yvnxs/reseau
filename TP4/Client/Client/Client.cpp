/*
Authors: Yanis Khati , Nadir Faci
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <strstream>
#include <fstream>
#include <string>   

using namespace std;
// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")


char ip[256];			// IP du serveur
string portServeur;			// Port d'�coute

// Fonction qui demande au client les informations du serveur
void initialiserClient() {


	cout << "Veuillez entrer l'adresse IP du serveur" << endl;
	cin.get(ip, 256);

	cout << "Veuillez entrer un port du serveur entre 10000 et 10050" << endl;
	cin >> portServeur;

	while ((atoi(portServeur.c_str()) < 10000) | (atoi(portServeur.c_str()) > 10050))
	{
		cout << "Numero de port errone. Veuillez entrer un port d'ecoute entre 10000 et 10050" << endl;
		cin >> portServeur;
	}
}



int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET leSocket;// = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	char reponseEnvoyee[200];
	char questionRecue[500];
	int iResult;

	//--------------------------------------------
	// InitialisATION de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Erreur de WSAStartup: %d\n", iResult);
		return 1;
	}
	// On va creer le socket pour communiquer avec le serveur
	leSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (leSocket == INVALID_SOCKET) {
		printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}
	//--------------------------------------------
	// On va chercher l'adresse du serveur en utilisant la fonction getaddrinfo.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;        // Famille d'adresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // Protocole utilis� par le serveur


	const char* port;
	char *host;
	
	initialiserClient(); // On recueuille les informations du serveur

	port = portServeur.c_str();
	host = ip;

	bool clientConnecte = false;

	while (!clientConnecte)  
	{
		// getaddrinfo obtient l'adresse IP du host donn�
		iResult = getaddrinfo(host, port, &hints, &result);
		if (iResult != 0) {
			printf("Erreur de getaddrinfo: %d\n", iResult);
			WSACleanup();
			return 1;

		}
			//---------------------------------------------------------------------		
			//On parcours les adresses retournees jusqu'a trouver la premiere adresse IPV4
			while ((result != NULL) && (result->ai_family != AF_INET))
				result = result->ai_next;

			//	if ((result != NULL) &&(result->ai_family==AF_INET)) result = result->ai_next;  

			//-----------------------------------------
			if (((result == NULL) || (result->ai_family != AF_INET))) {
				freeaddrinfo(result);
				printf("Impossible de recuperer la bonne adresse\n\n");
				WSACleanup();
				system("pause");
				return 1;
			}

			sockaddr_in *adresse;
			adresse = (struct sockaddr_in *) result->ai_addr;
			//----------------------------------------------------
			printf("Adresse trouvee pour le serveur %s : %s\n\n", host, inet_ntoa(adresse->sin_addr));
			printf("Tentative de connexion au serveur %s avec le port %s\n\n", inet_ntoa(adresse->sin_addr), port);



			// On va se connecter au serveur en utilisant l'adresse qui se trouve dans
			// la variable result.
			iResult = connect(leSocket, result->ai_addr, (int)(result->ai_addrlen));
			if (iResult == SOCKET_ERROR) {
				printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
				system("pause");
				return 1;
			}

			else
				clientConnecte = true; // le client est connect�, on sort de la boucle
	
	}  //----while(clientConnecte)-----



	//------------------------------
	// Maintenant, on va recevoir l' information envoy�e par le serveur
	iResult = recv(leSocket, questionRecue, sizeof(questionRecue), 0);  
		if (iResult > 0) {
			
			
			if ((string)questionRecue == "Sondage expire")       // Si le sondage est expir�, on quitte
			{
				cout << "Desole, le sondage est expire!" << endl;				
				closesocket(leSocket);
				WSACleanup();
				system("pause");

				return 1;
			}


			printf("Connecte au serveur %s:%s\n\n", host, port);
			freeaddrinfo(result);

			cout << "La question recue est: " << questionRecue << endl;
		}
		else
		{
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}

		

	//----------------------------
	// Demander � l'usager la reponse a envoyer au serveur

	cin.ignore();
	printf("Saisir une reponse a envoyer au professeur (max. 200 caracteres): ");
	gets_s(reponseEnvoyee);

	int n = sizeof(reponseEnvoyee) / sizeof(char);

	while ((n < 0) | (n>200))
	{
	    cout << "La taile de la reponse est inad�quate. Veuillez saisir de nouveau votre reponse (max. 200 caract�res)" << endl;
		cin >> reponseEnvoyee;
		n = sizeof(reponseEnvoyee) / sizeof(char);
	}


	//-----------------------------
	// Envoyer la r�ponse au serveur
	iResult = send(leSocket, reponseEnvoyee, sizeof(reponseEnvoyee), 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}	

	// cleanup
	closesocket(leSocket);
	WSACleanup();

	system("pause");
	return 0;
}