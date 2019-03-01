#include <winsock2.h>	
#include <ws2tcpip.h>	
#include <iostream>		
#include <string>		
#include <thread>		
#pragma comment (lib, "Ws2_32.lib")		


using namespace std;

#define IP_ADDRESS "127.0.0.1"		//LocalHost
#define DEFAULT_PORT "54000"		//Port used
#define DEFAULT_BUFLEN 512	


struct client_type	//Struct containing client id and socket used, type client_type
{
	SOCKET socket;
	int id;
	char recvMessage[DEFAULT_BUFLEN];
};

int process_client(client_type &new_user); //Declaring of client process function
int main();

int process_client(client_type &new_user) 
{
	while (1) //Infinite loop awaiting connections
	{
		memset(new_user.recvMessage, 0, DEFAULT_BUFLEN);

		if (new_user.socket != 0)
		{	//Checking for client connection and getting socket and buffer lenght
			int clientResult = recv(new_user.socket, new_user.recvMessage, DEFAULT_BUFLEN, 0);
			if (clientResult != SOCKET_ERROR) 
				cout << new_user.recvMessage << endl;

			else //Error handling for connection
			{
				cout << "recv() failed: " << WSAGetLastError() << endl;
				break;
			}
		}
	}
	

	if (WSAGetLastError() == WSAECONNRESET)
		cout << "The server has disconnected" << endl;

	return 0;
}

int main()
{
	WSAData wsaData;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	string sent_message = "";
	client_type user = { INVALID_SOCKET, -1, "" };
	int clientResult = 0;
	string message;

	cout << "Opening client\n";
	clientResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (clientResult != 0) { //Checking for client
		cout << "WSAStartup() error: " << clientResult << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	// adding info to client result
	clientResult = getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &result);
	if (clientResult != 0) { 
		cout << "getaddrinfo() failed with the following error: " << clientResult << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	//for loop looking for address
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// socket for connecting to server
		user.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (user.socket == INVALID_SOCKET) { //error message for socket fail
			cout << "socket() failed with the following error: " << WSAGetLastError() << endl;
			WSACleanup();
			system("pause");
			return 1;
		}

		// connecting 
		clientResult = connect(user.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (clientResult == SOCKET_ERROR) {
											closesocket(user.socket);
											user.socket = INVALID_SOCKET;
											continue;
											}
		break;
	}

	freeaddrinfo(result);

	if (user.socket == INVALID_SOCKET) { // Error handling for invalid socket
		cout << "Failed to connect to server!" << endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	cout << "Connected" << endl;

	// receiving information from server
	recv(user.socket, user.recvMessage, DEFAULT_BUFLEN, 0);
	message = user.recvMessage;

	if (message != "Server is full") 
	{
		user.id = atoi(user.recvMessage);

		thread my_thread(process_client, ref(user));

		while (1) //Loop awaiting connection 
		{
			getline(cin, sent_message);
			clientResult = send(user.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0); //Sending info with message

			
			
			if (clientResult <= 0) //Error handling for number of clients
			{
				cout << "send() has failed: " << WSAGetLastError() << endl;
				break;
			}

		}

		my_thread.detach();
	}
	else
		cout << user.recvMessage << endl;

	cout << "Closing socket" << endl;
	clientResult = shutdown(user.socket, SD_SEND); // Closing socket
	if (clientResult == SOCKET_ERROR) { //Error handling
		cout << "shutdown() failed with the following error: " << WSAGetLastError() << endl;
		closesocket(user.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	//Closing
	closesocket(user.socket);
	WSACleanup();
	system("pause");
	return 0;
}