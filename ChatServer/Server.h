#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>
#include <string_view>

class Server
{
public:
	Server();

	SOCKET _listening;

	enum class StatusCode {
		notLogged = -1,
		logged = 1
	};

	struct client {
		client(StatusCode status, int id, SOCKET sock, std::string login) :
			_status(status),
			_id(id),
			_sock(sock),
			_login(login)
		{};

		StatusCode _status = StatusCode::notLogged;
		int _id = -1;
		SOCKET _sock = -1;
		std::string _login;
	};

	void addUser(std::string login, std::string pass, StatusCode status, SOCKET sock);
	
	bool isConnected();
	
	bool checkUser(std::string login);
	bool checkPass(std::string login, std::string pass);
	
	void sendHello(SOCKET sock);
	void sendLogin(SOCKET sock, std::string login);
	void sendLoginError(SOCKET sock, std::string login);
	void sendToAll(SOCKET sock, std::string str, int id);
	void sendPong(SOCKET sock, int id);
	void sendLogout(SOCKET sock, int id);

	void newConnection(char* buff, SOCKET sock);
	void processComand(char* buff, SOCKET sock);

	std::string getLogin(int id);

	static int jsonLength(char* buff) {
		int len = 0;
		for (size_t i = 0; i < strlen(buff); ++i) {
			if (buff[i] == '}') {
				len = i + 1;
				break;
			}
		}
		return len;
	}

	std::vector<client> clients;
private:
	int _currUUID = -1;
	std::unordered_map<std::string, std::string> _passwords;
	std::unordered_map<int, std::string> _nicknames;

};

#endif