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
				serv.newConnection(buf, client);
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
					serv.processComand(buf, sock);
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(serv._listening, &master);
	closesocket(serv._listening);

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];
		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");
}