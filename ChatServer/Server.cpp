#include "Server.h"
#pragma warning(disable:4996) 

Server::Server()
{
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

	_listening = socket(AF_INET, SOCK_STREAM, NULL);
	bind(_listening, (SOCKADDR*)&hint, sizeof(hint));
	listen(_listening, SOMAXCONN);

	//_server = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
}

void Server::addUser(std::string login, std::string pass)
{
	_passwords[login] = pass;
}

bool Server::isConnected()
{
	return _server > 0;
}

bool Server::checkUser(std::string login)
{
	return _passwords.find(login) != _passwords.end();
}

bool Server::checkPass(std::string login, std::string pass)
{
	return 	_passwords[login] == pass;
}
