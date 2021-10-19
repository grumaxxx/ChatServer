#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <document.h>
#include <writer.h>
#include <stringbuffer.h>
#include <string>

#pragma warning(disable: 4996)

int main(int argc, char* argv[]) {

	char buff[512];

	const char* hello = "{\"id\":\"1\",\"command\":\"HELLO\"}";


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
	std::cout << "CONNECTED\n";
	
	bool handShakeFlag = false;
	bool logged = false;


	send(Connection, hello, strlen(hello), NULL);
	do
	{
		recv(Connection, buff, 512, 0);

		if (*buff == '{') {

			int len = 0;
			for (size_t i = 0; i < strlen(buff); ++i) {
				if (buff[i] == '}') {
					len = i + 1;
					break;
				}
			}

			rapidjson::Document document;
			document.Parse(buff, len);

			std::string comAnswer = document["command"].GetString();

			if (comAnswer == "HELLO") {
				handShakeFlag = true;
				std::cout << "RECEIVE HELLO" << std::endl;
			
				const char* auth = "{\"id\":\"1\",\"command\":\"login\",\"login\":\"ASD\",\"password\":\"123123\"}";

				send(Connection, auth, strlen(auth), NULL);
			}
		}
	} while (!handShakeFlag);

	while (!logged) {

		recv(Connection, buff, 512, 0);

		int len = 0;
		for (size_t i = 0; i < strlen(buff); ++i) {
			if (buff[i] == '}') {
				len = i + 1;
				break;
			}
		}

		rapidjson::Document document;
		document.Parse(buff, len);

		std::string comAnswer = document["command"].GetString();

		if (comAnswer == "login") {
			std::cout << "status is: " << document["status"].GetString() << std::endl;
			std::cout << "session id: " << document["session"].GetString() << std::endl;

			std::string status = document["status"].GetString();

			if (status == "ok") {
				logged = true;
			}
		}
	}

	while (1) {

		std::string body;
		std::getline(std::cin, body);

		std::string mess = "{\"id\":\"1\",\"command\":\"message\",\"body\":\"";
		mess = mess + body + "\",\"session\":\"123123\"}";
		send(Connection, mess.c_str(), strlen(mess.c_str()), NULL);


	}

	system("pause");
	return 0;
}