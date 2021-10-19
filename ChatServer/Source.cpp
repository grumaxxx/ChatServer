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

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void sendHello(SOCKET& sock) {
	const char* json = "{\"id\":\"1\",\"command\":\"HELLO\",\"auth_method\":\"plain_text\"}";
	send(sock, json, strlen(json), NULL);
	std::cout << "SEND HELLO" << std::endl;
}

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

		std::cout << socketCount << std::endl;

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
				recv(client, buf, DEFAULT_BUFLEN, 0);

				rapidjson::Document document;
				document.Parse(buf, jsonLength(buf));

				std::string comAnswer = document["command"].GetString();

				if (comAnswer == "HELLO") {

					sendHello(client);

					//handShakeFlag = true;
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
						}
						else {
							serv.addUser(login, password);
							std::cout << "connected user with login: " << login << std::endl;
							std::cout << "password is: " << password << std::endl;

							serv.clients.emplace_back(Server::StatusCode::logged, sock, login);

							std::string loginCommand = "{\"id\":\"1\",\"command\":\"login\",\"status\":\"ok\",\"session\":\"";
							loginCommand += std::to_string(sock) + "\"}";
							send(sock, loginCommand.c_str(), strlen(loginCommand.c_str()), NULL);
						}
					}
					// Send message to other clients, and definiately NOT the listening socket

					if (command == "message") {
						std::string incomeMsg = document["body"].GetString();
						std::cout << "income message: " << incomeMsg << std::endl;

						for (int i = 0; i < master.fd_count; i++)
						{
							SOCKET outSock = master.fd_array[i];
							if (outSock != serv._listening && outSock != sock)
							{
								std::string msg = "{\"id\":\"1\",\"command\":\"";
								msg += incomeMsg + "\",\"body\":\"ok\",\"session\":\"123123\"}";
								send(outSock, msg.c_str(), strlen(msg.c_str()), NULL);
							}
						}
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(serv._server, &master);
	closesocket(serv._server);

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


int foo() {

	char buf[4096];
	ZeroMemory(buf, 4096);

	Server serv;

	bool handShakeFlag = false;

	if (serv.isConnected()) std::cout << "CONNECTED" << std::endl;

	while (serv.isConnected()) {

		while (!handShakeFlag) {

			recv(serv._server, buf, DEFAULT_BUFLEN, 0);
			
			rapidjson::Document document;
			document.Parse(buf, jsonLength(buf));

			std::string comAnswer = document["command"].GetString();

			if (comAnswer == "HELLO") {

				sendHello(serv._server);

				handShakeFlag = true;
			}
		}

		int bytesIn = recv(serv._server, buf, DEFAULT_BUFLEN, 0);

		if (bytesIn > 0) {

			rapidjson::Document document;
			document.Parse(buf, jsonLength(buf));

			std::string command = document["command"].GetString();
			
			if (command == "login") {

				if (serv.checkUser(document["login"].GetString())) {
					if (serv.checkPass(document["login"].GetString(), document["password"].GetString())) {
						std::cout << "connected user with login:" << document["login"].GetString() << std::endl;
					}
				}
				else {
					serv.addUser(document["login"].GetString(), document["password"].GetString());
					std::cout << "connected user with login:" << document["login"].GetString() << std::endl;
					std::cout << "password is:" << document["password"].GetString() << std::endl;

					const char* loginCommand = "{\"id\":\"1\",\"command\":\"login\",\"status\":\"ok\",\"session\":\"123123\"}";
					send(serv._server, loginCommand, strlen(loginCommand), NULL);
				}
			}

			if (command == "message") {
				std::cout << "income message: " << document["body"].GetString() << std::endl;
				//ECHO
				const char* loginCommand = "{\"id\":\"1\",\"command\":\"message\",\"body\":\"ok\",\"session\":\"123123\"}";
				send(serv._server, loginCommand, strlen(loginCommand), NULL);

			}
		}
	}

	system("pause");
	return 0;
}