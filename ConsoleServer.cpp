#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <fstream>
#include <string>

#define WIN32_LEAN_AND_MEAN

using namespace std;

int shutDown(ADDRINFO* addrResult, SOCKET& Connection)
{
	closesocket(Connection);
	Connection = INVALID_SOCKET;
	freeaddrinfo(addrResult);
	WSACleanup();
	return 1;
}

void createFile(string path, char* recvBuffer)
{
	ofstream out;
	path += "file.txt";
	out.open(path);
	if (out.is_open())
	{
		cout << "Saving file...\n";
		out << recvBuffer << endl;
	}
	cout << "File has been created" << endl;
	return;
}

string getFile(string path)
{
	path += "file.txt";
	string file;
	ifstream in(path);
	if (in.is_open())
	{
		cout << "Sending file...\n";
		getline(in, file);
	}
	else
	{
		cout << "Error" << endl;
		return string();
	}
	in.close();
	return file;
}

int main()
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	ADDRINFO hints;
	ADDRINFO* addrResult = nullptr;
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKET ListenSocket = INVALID_SOCKET;
	string fileStorage = "G:/VS/ConsoleServer/Files/";

	const int bufferSize = 100000;
	char sendBuffer[bufferSize];
	char recvBuffer[bufferSize];

	int iResult = WSAStartup(DLLVersion, &wsaData);
	if (iResult != 0)
	{
		cout << "Error" << endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(nullptr, "1111", &hints, &addrResult);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		cout << "Socket creation failed" << endl;
		return shutDown(addrResult, ListenSocket);
	}

	iResult = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to bind socket" << endl;
		return shutDown(addrResult, ListenSocket);
	}

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Listening socket failed" << endl;
		return shutDown(addrResult, ListenSocket);
	}
	cout << "Connected\n";

	ClientSocket = accept(ListenSocket, nullptr, nullptr);
	if (ClientSocket == INVALID_SOCKET)
	{
		cout << "Accepting socket failed" << endl;
		return shutDown(addrResult, ClientSocket);
	}

	closesocket(ListenSocket);

	ZeroMemory(recvBuffer, sizeof(recvBuffer));
	iResult = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
	if (iResult > 0)
	{
		cout << "Received " << iResult << " bytes\n";
		createFile(fileStorage, recvBuffer);

	}

	ZeroMemory(recvBuffer, sizeof(recvBuffer));
	iResult = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
	if (iResult > 0)
	{
		if (strcmp(recvBuffer, "Client is ready\n") == 0)
		{
			string file = getFile(fileStorage);
			if (file.empty())
			{
				strcpy_s(sendBuffer, string("Error path").c_str());
				iResult = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
				return shutDown(addrResult, ClientSocket);
			}
			strcpy_s(sendBuffer, file.c_str());
			iResult = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
		}
	}

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Shutdown client socket failed" << endl;
		return shutDown(addrResult, ClientSocket);
	}

	shutDown(addrResult, ClientSocket);
	cout << "Connection closed" << endl;
	return 0;
}