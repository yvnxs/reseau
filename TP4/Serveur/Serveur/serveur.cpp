/*
Authors: Yanis Khati , Nadir Faci 
*/

#undef UNICODE



#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <algorithm>
#include <strstream>
#include <iostream>
#include <fstream>
#include <string>   
#include <ctime>

using namespace std;

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

// External functions
extern DWORD WINAPI recevoirReponse(void* sd_);
extern void writeAnswer(char* ip, int port, char* reponse);
extern void intialiserServeur();

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
static struct ErrorEntry {
	int nID;
	const char* pcMessage;

	ErrorEntry(int id, const char* pc = 0) :
		nID(id),
		pcMessage(pc)
	{
	}

	bool operator<(const ErrorEntry& rhs) const
	{
		return nID < rhs.nID;
	}
} gaErrorList[] = {
	ErrorEntry(0,                  "No error"),
	ErrorEntry(WSAEINTR,           "Interrupted system call"),
	ErrorEntry(WSAEBADF,           "Bad file number"),
	ErrorEntry(WSAEACCES,          "Permission denied"),
	ErrorEntry(WSAEFAULT,          "Bad address"),
	ErrorEntry(WSAEINVAL,          "Invalid argument"),
	ErrorEntry(WSAEMFILE,          "Too many open sockets"),
	ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
	ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
	ErrorEntry(WSAEALREADY,        "Operation already in progress"),
	ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
	ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
	ErrorEntry(WSAEMSGSIZE,        "Message too long"),
	ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
	ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
	ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
	ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
	ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
	ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
	ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
	ErrorEntry(WSAEADDRINUSE,      "Address already in use"),
	ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
	ErrorEntry(WSAENETDOWN,        "Network is down"),
	ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
	ErrorEntry(WSAENETRESET,       "Net connection reset"),
	ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
	ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
	ErrorEntry(WSAENOBUFS,         "No buffer space available"),
	ErrorEntry(WSAEISCONN,         "Socket is already connected"),
	ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
	ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
	ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
	ErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
	ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
	ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
	ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
	ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
	ErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
	ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
	ErrorEntry(WSAEPROCLIM,        "Too many processes"),
	ErrorEntry(WSAEUSERS,          "Too many users"),
	ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
	ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
	ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
	ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
	ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
	ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
	ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
	ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
	ErrorEntry(WSANO_DATA,         "No host data of that type was found")
};
const int kNumMessages = sizeof(gaErrorList) / sizeof(ErrorEntry);


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned 
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.
const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0)
{
	// Build basic error string
	static char acErrorBuffer[256];
	ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
	outs << pcMessagePrefix << ": ";

	// Tack appropriate canned message onto end of supplied message 
	// prefix. Note that we do a binary search here: gaErrorList must be
	// sorted by the error constant's value.
	ErrorEntry* pEnd = gaErrorList + kNumMessages;
	ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
	ErrorEntry* it = lower_bound(gaErrorList, pEnd, Target);
	if ((it != pEnd) && (it->nID == Target.nID)) {
		outs << it->pcMessage;
	}
	else {
		// Didn't find error in list, so make up a generic one
		outs << "unknown error";
	}
	outs << " (" << Target.nID << ")";

	// Finish error message off and return it.
	outs << ends;
	acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
	return acErrorBuffer;
}


char* ip;			// IP du serveur
int port;			// Port d'écoute
unsigned int duree; //durée du sondage
char* question; // question du sondage

// Fonction qui demande a l'utilisateur les informations du serveur, 
// la durée du sondage et la question
void intialiserServeur() {


	cout << "Veuillez entrer l'adresse IP du poste" << endl;
	cin >> ip;

	cout << "Veuillez entrer un port d'écoute entre 10000 et 10050" << endl;
	cin >> port;

	while ((port < 10000) | (port > 10050))
	{
		cout << "Numéro de port érroné. Veuillez entrer un port d'écoute entre 10000 et 10050" << endl;
		cin >> port;
	}


	cout << "Veuillez entrer la durée du sondage (s)" << endl;
	cin >> duree;

	while (duree < 0)
	{
		cout << "Duree érronée. Veuillez entrer une durée positive" << endl;
		cin >> duree;
	}

	cout << "Veuillez saisir la question du sondage (maximum 500 caractères)" << endl;
	cin >> question;

	int n = sizeof(question) / sizeof(char);
	while ( (n < 0) | (n>500))
	{
		cout << "La taile de la question est inadéquate. Veuillez saisir de nouveau votre question (maximum 500 caractères)" << endl;
		cin >> duree;
	}

}


// Ecriture des réponses dans le journal et affichage à la console
void writeAnswer(char* ip, int port, char* reponse) 
{
	
	string line = (string)ip + " : " +  to_string(port) + " - " + reponse;
	cout << line << endl;

	ofstream fichier("journal.txt", ios::out);  //déclaration du flux et ouverture du fichier

		if (fichier)  // si l'ouverture a réussi
		{
			fichier << line << endl; // on écrit dans le fichier

			fichier.close();  // on referme le fichier
		}
		else  // sinon
			cerr << "Erreur à l'ouverture !" << endl;
}

//Vérifie si le sondage est toujours actif
bool verifierTimer(int duree, time_t start) {


	if(difftime(time(0), start)<duree)   
		return 1;

	else 
		return 0;

}

time_t startTimer;
int main(void)
{
	//----------------------
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		cerr << "Error at WSAStartup()\n" << endl;
		return 1;
	}

	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	SOCKET ServerSocket;
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET) {
		cerr << WSAGetLastErrorMessage("Error at socket()") << endl;
		WSACleanup();
		return 1;
	}
	char* option = "1";
	setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, option, sizeof(option));


	//----------------------
	// Demande a l'utilisateur les informations du serveur, 
	// la durée du sondage et la question
	intialiserServeur();


	//----------------------

	//Recuperation de l'adresse locale
	hostent *thisHost;
	char hostname[256];
	gethostname(hostname, sizeof(hostname));
	thisHost = gethostbyname(ip);
	char* ip;
	ip = inet_ntoa(*(struct in_addr*) *thisHost->h_addr_list);
	printf("Adresse locale trouvee %s : \n\n", ip);
	sockaddr_in service;
	service.sin_family = AF_INET;

	//service.sin_addr.s_addr = inet_addr("127.0.0.1");
	//	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_addr.s_addr = inet_addr(ip);
	service.sin_port = htons(port);


	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ServerSocket, 30) == SOCKET_ERROR) {
		cerr << WSAGetLastErrorMessage("Error listening on socket.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}

	
	startTimer = time(0); //initialiser le timer 
	bool sondageActif = true;

	printf("En attente des connections des clients sur le port %d...\n\n", ntohs(service.sin_port));

	while (sondageActif) {

		if (!verifierTimer(duree, startTimer))
			sondageActif = false;



		sockaddr_in sinRemote;
		int nAddrSize = sizeof(sinRemote);
		// Create a SOCKET for accepting incoming requests.
		// Accept the connection.
		SOCKET sd = accept(ServerSocket, (sockaddr*)&sinRemote, &nAddrSize);
		if (sd != INVALID_SOCKET) {
			cout << "Connection acceptee De : " <<
				inet_ntoa(sinRemote.sin_addr) << ":" <<
				ntohs(sinRemote.sin_port) << "." <<
				endl;

			DWORD nThreadID;
			CreateThread(0, 0, recevoirReponse, (void*)sd, 0, &nThreadID);
		}
		else {
			cerr << WSAGetLastErrorMessage("Echec d'une connection.") <<
				endl;
			 return 1;
		}
	}

	// No longer need server socket
	closesocket(ServerSocket);

	WSACleanup();
	return 0;

}




//// EchoHandler ///////////////////////////////////////////////////////
// Handles the incoming data by reflecting it back to the sender.

DWORD WINAPI recevoirReponse(void* sd_)
{
	SOCKET sd = (SOCKET)sd_;

	send(sd, question,sizeof(question), 0); //sizeof(question) ou bien 500 ,a verifier

	// Read Data from client
	char readBuffer[200];
	int readBytes;

	readBytes = recv(sd, readBuffer, 200, 0);
	if (readBytes > 0) {
		cout << "Received " << readBytes << " bytes from client." << endl;
		cout << "Received " << readBuffer << " from client." << endl;

		writeAnswer(ip,port,readBuffer);
		
	}
	else if (readBytes == SOCKET_ERROR) {
		cout << WSAGetLastErrorMessage("Echec de la reception !") << endl;
	}
	closesocket(sd);

	return 0;
}
