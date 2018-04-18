extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include <windows.h>
#include "Firmata.h"

	class FirmataWifi : public Firmata {
	public:
		SOCKET connectSocket;
	private:

		char ReadByte();
		char ReadByteBlocking();
		bool ReadByteNonBlocking(char *buffer);

		void SendOnConnection(int n);
		//bool ReadFromConnection();
	};

	FIRMATA_API FirmataWifi *Firmata_ConnectWifi(char *address, int port);
	FIRMATA_API void Firmata_DisconnectWifi(FirmataWifi *connection);
}