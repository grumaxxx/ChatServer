#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>
#include "Server.h"
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")

#pragma warning(disable:4996) 

int jsonLength(char* buff) {
	int len = 0;
	for (size_t i = 0; i < strlen(buff); ++i) {
		if (buff[i] == '}') {
			len = i + 1;
			break;
		}
	}
	return len;
}

void main()
{
	Server serv;
	int UUID = 0;

	fd_set master;
	FD_ZERO(&master);
	FD_SET(serv._listening, &master);

	bool running = true;
	while (running)
	{
		char buf[4096];
		ZeroMemory(buf, 4096);

		fd_set copy = master;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == serv._listening)
			{
				// Accept a new connection
				SOCKET client = accept(serv._listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				recv(client, buf, 4096, 0);

				rapidjson::Document document;
				document.Parse(buf, jsonLength(buf));

				std::string comAnswer = document["command"].GetString();

				if (comAnswer == "HELLO") {
					serv.sendHello(client);
				}
			}
			else // It's an inbound message
			{


				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);

				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					rapidjson::Document document;
					document.Parse(buf, jsonLength(buf));

					std::string command = document["command"].GetString();

					if (command == "login") {

						std::string login = document["login"].GetString();
						std::string password = document["password"].GetString();

						if (serv.checkUser(login)) {
							if (serv.checkPass(login, password)) {
								std::cout << "connected user with login:" << login << std::endl;
							}
							else {
								serv.addUser(login, password, Server::StatusCode::notLogged, sock);
							}
						}
						else {
							
							serv.addUser(login, password, Server::StatusCode::logged, sock); 
							
							std::cout << "connected user with login: " << login << std::endl;
							std::cout << "password is: " << password << std::endl;

							serv.sendLogin(sock, login);

						}
					}
					if (command == "message") {
						std::string incomeMsg = document["body"].GetString();
						int id = std::stoi(document["id"].GetString());
						std::cout << "income message: " << incomeMsg << std::endl;

						serv.sendToAll(sock, incomeMsg, id);
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(serv._listening, &master);
	closesocket(serv._listening);

	// Message to let users know what's happening.
	std::string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");
}