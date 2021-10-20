#include "Server.h"
#pragma warning(disable:4996) 

Server::Server()
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	_listening = socket(AF_INET, SOCK_STREAM, NULL);
	bind(_listening, (SOCKADDR*)&hint, sizeof(hint));
	listen(_listening, SOMAXCONN);
}

void Server::addUser(std::string login, std::string pass, StatusCode status, SOCKET sock)
{
	_currUUID++;
	_passwords[login] = pass;
	_nicknames[_currUUID] = login;
	clients.emplace_back(status, _currUUID, sock, login);
}

bool Server::isConnected()
{
	return _listening > 0;
}

bool Server::checkUser(std::string login)
{
	return _passwords.find(login) != _passwords.end();
}

bool Server::checkPass(std::string login, std::string pass)
{
	return 	_passwords[login] == pass;
}

void Server::sendHello(SOCKET sock)
{
	const char* json = "{\"id\":\"1\",\"command\":\"HELLO\",\"auth_method\":\"plain_text\"}";
	send(sock, json, strlen(json), NULL);
}

void Server::sendLogin(SOCKET sock, std::string login)
{
	std::string loginCommand = "{\"id\":\"" + std::to_string(_currUUID)
		+ "\",\"command\":\"login\",\"status\":\"ok\",\"session\":\"" + std::to_string(_currUUID) + "\"}";
	send(sock, loginCommand.c_str(), strlen(loginCommand.c_str()), NULL);
}

void Server::sendLoginError(SOCKET sock, std::string login)
{
	std::string loginCommand = "{\"id\":\"" + std::to_string(_currUUID)
		+ "\",\"command\":\"login\",\"status\":\"failed\",\"message\":\"incorrect password\"}";
	send(sock, loginCommand.c_str(), strlen(loginCommand.c_str()), NULL);
}

void Server::sendToAll(SOCKET sock, std::string str, int id)
{
	for (auto& v : clients) {
		if (v._status == Server::StatusCode::logged) {
			if (sock != v._sock) {
				std::string msg = "{\"id\":\"" + std::to_string(v._id) +
					"\",\"command\":\"message\",\"body\":\""
					+ str + "\",\"sender login\":\"" + getLogin(id)
					+ "\",\"session\":\"" + std::to_string(v._id) + "\"}";
				send(v._sock, msg.c_str(), strlen(msg.c_str()), NULL);
			}
		}
		else {
			//send error
		}
	}
}

void Server::sendPong(SOCKET sock, int id)
{
	if (clients[id]._status == StatusCode::logged) {
		std::string pong = "{\"id\":\"" + std::to_string(id)
			+ "\",\"command\":\"ping_reply\",\"status\":\"ok\"}";
		int bytes = send(sock, pong.c_str(), pong.size() + 1, NULL);
	}
	else {
		std::string pong = "{\"id\":\"" + std::to_string(id)
			+ "\",\"command\":\"ping_reply\",\"status\":\"failed\",\"message\":\"error: not logged\"}";
		int bytes = send(sock, pong.c_str(), pong.size() + 1, NULL);
	}
}

void Server::sendLogout(SOCKET sock, int id)
{
	std::string logout = "{\"id\":\"" + std::to_string(id)
		+ "\",\"command\":\"logout_reply\",\"status\":\"ok\"}";
	send(sock, logout.c_str(), logout.size() + 1, NULL);
	//closesocket(sock);
}

void Server::newConnection(char* buff, SOCKET sock)
{
	recv(sock, buff, 4096, 0);

	rapidjson::Document document;
	document.Parse(buff, jsonLength(buff));

	std::string comAnswer = document["command"].GetString();

	if (comAnswer == "HELLO") {
		sendHello(sock);
	}
}

void Server::processComand(char* buff, SOCKET sock)
{
	rapidjson::Document document;
	document.Parse(buff, jsonLength(buff));

	std::string command = document["command"].GetString();
	int id = std::stoi(document["id"].GetString());

	if (command == "login") {

		std::string login = document["login"].GetString();
		std::string password = document["password"].GetString();
		
		if (checkUser(login)) {
			if (checkPass(login, password)) {
				std::cout << "connected user with login:" << login << std::endl;
				sendLogin(sock, login);
			}
			else {
				std::cout << "incorrect password" << std::endl;
				sendLoginError(sock, login);
			}
		}
		else {

			addUser(login, password, Server::StatusCode::logged, sock);

			std::cout << "connected user with login: " << login << std::endl;
			std::cout << "password is: " << password << std::endl;

			sendLogin(sock, login);

		}
	}
	if (command == "message") {
		std::string incomeMsg = document["body"].GetString();
		std::cout << "income message: " << incomeMsg << std::endl;

		sendToAll(sock, incomeMsg, id);
	}
	else if (command == "message_reply") {
		std::string status = document["status"].GetString();
		if (status != "ok") {
			std::cout << "Message does'n received" << std::endl;
		}
	}
	else if (command == "ping") {
		sendPong(sock, id);
	}
	else if (command == "logout") {
		sendLogout(sock, id);
	}
	
}

std::string Server::getLogin(int id)
{
	return _nicknames[id];
}
