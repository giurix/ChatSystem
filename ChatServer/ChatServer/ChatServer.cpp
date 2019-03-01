#include <iostream>			
#include <winsock2.h>		
#include <ws2tcpip.h>		
#include <string>			
#include <thread>			
#include <vector>			


#define IP_ADDRESS "127.0.0.1"  //LocalHost
#define DEFAULT_PORT "54000"	//Port used
#define DEFAULT_BUFLEN 512	

#pragma comment (lib, "Ws2_32.lib")		// WinSocket Library used



struct client_type	// Struct containing client id and socket used, type client_type
{
	int id;
	SOCKET socket;
};

const char OPTION_VALUE = 1;// Constant char variable
const int MAX_CLIENTS = 3;	// Max number of clients connected at the same time


int process_client(client_type &new_user, std::vector<client_type> &user_array, std::thread &thread)  //Process client fuction used for processing clients connected 
{

	char tempmsg[DEFAULT_BUFLEN] = "";
	std::string msg = "";

	while (1)		//Infinite loop awaiting connections
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (new_user.socket != 0) // looks for connected clients
		{							// gets socket with the message and saves it in clientResult
			int userResult = recv(new_user.socket, tempmsg, DEFAULT_BUFLEN, 0);

			if (userResult != SOCKET_ERROR) // Checks for errors. If no errors continues to the next if statement
			{
				if (strcmp("", tempmsg)) // Checking if a message (not empty one) was sent
					msg = "User #" + std::to_string(new_user.id) + " : " + tempmsg;

				std::cout << msg.c_str() << std::endl; //prints and sends message
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (user_array[i].socket != INVALID_SOCKET) // checks if user's socket is valid
						if (new_user.id != i)
							userResult = send(user_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0); // - message is sent
				}
			}
			else
			{		//Else statement for error handling
				msg = "User #" + std::to_string(new_user.id) + " has left";

				std::cout << msg << std::endl;


				closesocket(new_user.socket);
				closesocket(user_array[new_user.id].socket);
				user_array[new_user.id].socket = INVALID_SOCKET; // closing sockets


				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (user_array[i].socket != INVALID_SOCKET)
						userResult = send(user_array[i].socket, msg.c_str(), strlen(msg.c_str()), 0);	//sends disconnection message to users
				}

				break;
			}
		}
	}

	thread.detach();

	return 0;
}

int main()
{
	WSADATA wsaData;
	struct addrinfo hints;
	struct addrinfo *server = NULL;
	SOCKET server_socket = INVALID_SOCKET;
	std::string msg = "";
	std::vector<client_type> user(MAX_CLIENTS);
	int num_clients = 0;
	int temp_id = -1;
	std::thread my_thread[MAX_CLIENTS];

	WSAStartup(MAKEWORD(2, 2), &wsaData);//Makes certain that Winsock is supported on the system




	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	//Setting up server

	getaddrinfo(static_cast<LPCTSTR>(IP_ADDRESS), DEFAULT_PORT, &hints, &server);
	server_socket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	bind(server_socket, server->ai_addr, (int)server->ai_addrlen);
	listen(server_socket, SOMAXCONN);
	std::cout << "Server is now open";
	std::cout << std::endl;


	//Initializing the user list
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		user[i] = { -1, INVALID_SOCKET };
	}

	while (1)
	{

		SOCKET incoming = INVALID_SOCKET;
		incoming = accept(server_socket, NULL, NULL);
		// socket created for accepting users

		if (incoming == INVALID_SOCKET) continue; // continue acception connections even if there is a problem with one of the user connections
		num_clients = -1;


		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (user[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				user[i].socket = incoming;
				user[i].id = i;
				temp_id = i;
			}
			// giving a user a temportary id
			if (user[i].socket != INVALID_SOCKET)
				num_clients++;
		}

		if (temp_id != -1)
		{
			//Sending the id to the user
			std::cout << "User #" << user[temp_id].id << " has come aboard." << std::endl;
			msg = std::to_string(user[temp_id].id);
			send(user[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);

			//Creating a thread so that multiple users can access at the same time
			my_thread[temp_id] = std::thread(process_client, std::ref(user[temp_id]), std::ref(user), std::ref(my_thread[temp_id]));
		}
		else
		{
			msg = "Full server";
			send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	}

	closesocket(server_socket);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		my_thread[i].detach();
		closesocket(user[i].socket);
	}
	WSACleanup();
	// Closing sockets and cleaning up Winsock
	system("break");
	return 0;
}