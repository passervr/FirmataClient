#include "stdafx.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#include "FirmataWifi.h"

#pragma comment(lib, "Ws2_32.lib")

#pragma region Read

bool FirmataWifi::ReadByteNonBlocking(char *buffer) {
	int iResult = recv(connectSocket, buffer, 1, 0);
	if (iResult > 0) {
		//sprintf_s(msgBuf, 256, " 0x%02hhX", buffer[0]);
		//OutputDebugStringA(msgBuf);

		return true;
	}
	else {
		return false;
	}
}
char FirmataWifi::ReadByte() {
	char *buffer = new char[1];

	ReadByteNonBlocking(buffer);
	return buffer[0];
}

char FirmataWifi::ReadByteBlocking() {
	char *buffer = new char[1];

	while (!ReadByteNonBlocking(buffer))
		Sleep(1);

	return buffer[0];
}
#pragma endregion

#pragma region Write
void FirmataWifi::SendOnConnection(int n) {
	//sprintf_s(msgBuf, 256, " sending: ");
	//OutputDebugStringA(msgBuf);
	//for (int i = 0; i < n; i++) {
	//	sprintf_s(msgBuf, 256, " 0x%02hhX", sendBuffer[i]);
	//	OutputDebugStringA(msgBuf);
	//}
	//sprintf_s(msgBuf, 256, "\n");
	//OutputDebugStringA(msgBuf);



	// Send an initial buffer
	int iResult = send(connectSocket, sendBuffer, n, 0);
	if (iResult == SOCKET_ERROR) {
		sprintf_s(msgBuf, 256, "send failed: %d\n", WSAGetLastError());
		OutputDebugStringA(msgBuf);

		closesocket(connectSocket);
		WSACleanup();
		return;
	}

	while (!ready)
		ReadFromConnection();

	//sprintf_s(msgBuf, 256, "Bytes Sent: %ld\n", iResult);
	//OutputDebugStringA(msgBuf);
}
#pragma endregion

#pragma region External
char *msgBuf = new char[256];

FIRMATA_API FirmataWifi *Firmata_ConnectWifi(char *address, int port) {
	FirmataWifi *connection = new FirmataWifi();
	connection->connected = false;

	WSADATA wsaData;

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		sprintf_s(msgBuf, 256, "WSAStartup failed: %d\n", iResult);
		OutputDebugStringA(msgBuf);
		return false;
	}

	// Create socket for connecting to server
	connection->connectSocket = INVALID_SOCKET;
	connection->connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connection->connectSocket == INVALID_SOCKET) {
		sprintf_s(msgBuf, 256, "socket function failed with error: %ld\n", WSAGetLastError());
		OutputDebugStringA(msgBuf);
		return false;
	}

	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	InetPtonA(AF_INET, address, &clientService.sin_addr.s_addr);
	clientService.sin_port = htons(port);
	

	// Connect to server.
	iResult = connect(connection->connectSocket, (SOCKADDR *)& clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		sprintf_s(msgBuf, 256, "connect function failed with error: %ld\n", WSAGetLastError());
		OutputDebugStringA(msgBuf);
		iResult = closesocket(connection->connectSocket);
		if (iResult == SOCKET_ERROR) {
			sprintf_s(msgBuf, 256, "closesocket function failed with error: %ld\n\n", WSAGetLastError());
			OutputDebugStringA(msgBuf);
		}
		WSACleanup();
		return false;
	}

	//set the socket in non-blocking
	unsigned long iMode = 1;
	iResult = ioctlsocket(connection->connectSocket, FIONBIO, &iMode);
	if (iResult != NO_ERROR) {
		sprintf_s(msgBuf, 256, "ioctlsocket failed with error: %ld\n", iResult);
		OutputDebugStringA(msgBuf);
	}

	OutputDebugStringA("Connected to server.\n");
	connection->RequestVersionReport();
	return connection;
}

FIRMATA_API void Firmata_DisconnectWifi(FirmataWifi *connection) {
	if (connection == NULL)
		return;

	int iResult = closesocket(connection->connectSocket);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	WSACleanup();
	return;
}
#pragma endregion