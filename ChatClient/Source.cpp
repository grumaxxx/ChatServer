#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>
#include <string>
#include <thread>


#pragma warning(disable: 4996)

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

void process_client(SOCKET sock, int UUID) {
	while (1) {
		char buff[512];
		int bytes = recv(sock, buff, 512, 0);
		if (bytes > 0) {

			rapidjson::Document document;
			document.Parse(buff, jsonLength(buff));

			std::string comAnswer = document["command"].GetString();

			if (comAnswer == "message") {
				std::string incomeMsg = document["body"].GetString();
				std::string user = document["sender login"].GetString();
				std::cout << user << ": " << incomeMsg << std::endl;

				std::string mess = "{\"id\":\"" + std::to_string(UUID) + "\",\"command\":\"message_reply\","
					+ "\"status\":\"ok\",\"client id\":\"" + std::to_string(UUID) + "\"}";
				send(sock, mess.c_str(), strlen(mess.c_str()), NULL);
			}
		}
	}
}



int main(int argc, char* argv[]) {

	char buff[512];

	const char* hello = "{\"id\":\"1\",\"command\":\"HELLO\"}";

	int UUID;

	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(54000);
	addr.sin_family = AF_INET;

	SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}
	std::cout << "Welcome to myChatServer\n";
	
	bool handShakeFlag = false;
	bool logged = false;

	send(Connection, hello, strlen(hello), NULL);
	
	do{
		recv(Connection, buff, 512, 0);

		rapidjson::Document document;
		document.Parse(buff, jsonLength(buff));

		std::string comAnswer = document["command"].GetString();

		if (comAnswer == "HELLO") {
			handShakeFlag = true;
			
			std::string login;
			std::string password;

			std::cout << "Please enter your login: " << std::endl;
			std::getline(std::cin, login);

			if (login.size() == 0) {
				std::cout << "Login can't be empty, please try again: " << std::endl;
				std::getline(std::cin, login);
			}

			std::cout << "Please enter your password: " << std::endl;
			std::getline(std::cin, password);

			if (password.size() == 0) {
				std::cout << "Password can't be empty, please try again: " << std::endl;
				std::getline(std::cin, login);
			}

			std::string auth = "{\"id\":\"1\",\"command\":\"login\",\"login\":\"" + login +
				"\",\"password\":\"" + password + "\"}";
				
			send(Connection, auth.c_str(), strlen(auth.c_str()), NULL);
		}
	} while (!handShakeFlag);

	while (!logged) {

		recv(Connection, buff, 512, 0);

		rapidjson::Document document;
		document.Parse(buff, jsonLength(buff));

		std::string comAnswer = document["command"].GetString();

		if (comAnswer == "login") {
			std::cout << "status is: " << document["status"].GetString() << std::endl;
			std::cout << "session id: " << document["session"].GetString() << std::endl;

			std::string status = document["status"].GetString();

			if (status == "ok") {
				std::cout << "Now you can write in chat" << std::endl;
				logged = true;
				UUID = std::stoi(document["session"].GetString());
			}
		}
	}

	std::thread my_thread(process_client, Connection, UUID);

	bool running = true;
	while (running) {


		std::string body;
		std::getline(std::cin, body);

		if (body[0] == '\\') {
			if (body == "\\quit") {
				//quit
				running = false;
				std::cout << "disconnected from server" << std::endl;
			}
			else if (body == "\\ping") {
				//ping
			}
		}
		else {
			std::string mess = "{\"id\":\"" + std::to_string(UUID) + "\",\"command\":\"message\",\"body\":\""
				+ body + "\",\"session\":\"" + std::to_string(UUID) + "\"}";
			send(Connection, mess.c_str(), strlen(mess.c_str()), NULL);
		}
		

	}

	my_thread.detach();

	system("pause");
	return 0;
}