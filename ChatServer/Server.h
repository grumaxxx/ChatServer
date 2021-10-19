#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>

class Server
{
public:
	Server();

	void addUser(std::string login, std::string pass);
	bool isConnected();
	bool checkUser(std::string login);
	bool checkPass(std::string login, std::string pass);
	void jsonParse();



	SOCKET _server;	
	SOCKET _listening;

	enum class StatusCode {
		notLogged = -1,
		logged = 1
	};

	struct client {
		client(StatusCode status, int id, std::string login) :
			_status(status),
			_id(id),
			_login(login)
		{};

		StatusCode _status = StatusCode::notLogged;
		int _id = -1;
		std::string _login;
	};

	std::vector<client> clients;
private:
	std::unordered_map<std::string, std::string> _passwords;

};

#endif