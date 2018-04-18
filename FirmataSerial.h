extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include <windows.h>
#include "Firmata.h"

	class FirmataSerial : public Firmata {
	public:
		HANDLE handle;
	private:
		COMSTAT status;
		DWORD errors;

		char ReadByte();
		char ReadByteBlocking();
		bool ReadByteNonBlocking(HANDLE handle, char *buffer);

		void SendOnConnection(int n);
		//bool ReadFromConnection();
	};

	FIRMATA_API FirmataSerial *Firmata_ConnectSerial(char *port, int speed);
	FIRMATA_API void Firmata_DisconnectSerial(FirmataSerial *connection);
}
