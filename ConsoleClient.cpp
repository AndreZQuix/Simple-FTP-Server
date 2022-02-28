#include <iostream>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
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

string getFile()
{
	cout << "Type the full file path\n";
	string filePath;
	getline(cin, filePath);
	string file;
	
	ifstream in(filePath);
	if (in.is_open())
	{
		cout << "Sending file...\n";
		while (getline(in, file)) {}
	}
	else cout << "Error" << endl;

	in.close();
	return file;
}

void downloadFile(char *file, string path)
{
	ofstream out;
	path += "/downloaded_file.txt";
	out.open(path);
	if (out.is_open())
	{
		cout << "Saving file...\n";
		out << file << endl;
	}
	cout << "File has been saved: " << path << endl;
	return;
}

int main()
{
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	ADDRINFO hints;
	ADDRINFO* addrResult = nullptr;
	SOCKET Connection = INVALID_SOCKET;

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

	iResult = getaddrinfo("127.0.0.1", "1111", &hints, &addrResult);
	if (iResult != 0)
	{
		cout << "getaddrinfo failed with error: " << iResult << endl;
		WSACleanup();
		return 1;
	}

	Connection = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (Connection == INVALID_SOCKET)
	{
		cout << "Socket creation failed" << endl;
		return shutDown(addrResult, Connection);
	}

	iResult = connect(Connection, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Unable to connect to server" << endl;
		return shutDown(addrResult, Connection);
	}

	cout << "Connected\n";

	strcpy_s(sendBuffer, getFile().c_str());

	iResult = send(Connection, sendBuffer, (int)strlen(sendBuffer), 0);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Send failed, error: " << iResult << endl;
		return shutDown(addrResult, Connection);
	}

	cout << "Sent: " << iResult << " bytes\n";

	ZeroMemory(recvBuffer, sizeof(recvBuffer));
	ZeroMemory(sendBuffer, sizeof(sendBuffer));

	cout << "Type the path to save the file\n";
	string path;
	getline(cin, path);

	strcpy_s(sendBuffer, string("Client is ready\n").c_str());
	iResult = send(Connection, sendBuffer, (int)strlen(sendBuffer), 0);

	iResult = recv(Connection, recvBuffer, sizeof(recvBuffer), 0);
	downloadFile(recvBuffer, path);

	//do
	//{
	//	ZeroMemory(recvBuffer, sizeof(recvBuffer));
	//	ZeroMemory(sendBuffer, sizeof(sendBuffer));
	//	iResult = recv(Connection, recvBuffer, sizeof(recvBuffer), 0);
	//	if (iResult > 0)
	//	{
	//		iResult = recv(Connection, recvBuffer, sizeof(recvBuffer), 0);
	//		downloadFile(recvBuffer);
	//	}
	//	else if (iResult == 0)
	//		cout << "Connection closed" << endl;
	//	else
	//		cout << "Recv failed with error" << endl;
	//} while (iResult > 0);

	shutDown(addrResult, Connection);
	return 0;
}
