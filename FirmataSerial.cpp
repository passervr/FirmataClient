extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include <stdio.h>
#include <windows.h>
#include "FirmataSerial.h"

#define DIGITALPINCOUNT		13
#define ANALOGPINCOUNT		6

	#pragma region Read
	bool FirmataSerial::ReadByteNonBlocking(HANDLE handle, char *buffer) {
		buffer[0] = 0;
		DWORD bytesRead;
		if (ReadFile(handle, buffer, 1, &bytesRead, NULL) != 1)
			return false;

		if (bytesRead != 1)
			return false;

		//sprintf_s(msgBuf, 256, " 0x%02hhX", buffer[0]);
		//OutputDebugStringA(msgBuf);

		return true;
	}

	char FirmataSerial::ReadByte() {
		// Memory leak!
		char *buffer = new char[1];

		ReadByteNonBlocking(handle, buffer);
		return buffer[0];
	}

	char FirmataSerial::ReadByteBlocking() {
		// Memory leak!
		char *buffer = new char[1];

		while (!ReadByteNonBlocking(handle, buffer))
			Sleep(1);

		return buffer[0];
	}
#pragma endregion

#pragma region Write
	void FirmataSerial::SendOnConnection(int n) {
		DWORD bytesSend;

		//while (!ready)
		//	ReadFromConnection();
		ReadFromConnection();

		//OutputDebugStringA("send:");
		//for (int i = 0; i < n; i++) {
		//	sprintf_s(msgBuf, 256, " 0x%02hhX", sendBuffer[i]);
		//	OutputDebugStringA(msgBuf);
		//}
		//OutputDebugStringA("\n");

		if (!WriteFile(handle, (void*)sendBuffer, n, &bytesSend, 0))
			ClearCommError(handle, &errors, &status);
	}

	//void FirmataSerial2::DigitalWrite(char pin, bool value) {
	//	sendBuffer[0] = (char)0xF5;
	//	sendBuffer[1] = (char)pin;
	//	sendBuffer[2] = (char)(value ? 1 : 0);
	//	SendOnConnection(3);
	//}
#pragma endregion

//#pragma region I2C
//	void FirmataSerial::I2C_Config() {
//		sendBuffer[0] = (char)0xF0; // START_SYSEX
//		sendBuffer[1] = (char)0x78; // I2C_CONFIG
//		sendBuffer[2] = (char)0xF7;
//		SendOnConnection(3);
//	}
//
//	void FirmataSerial::I2C_WriteByte(char address, char reg, char i2cData) {
//		sendBuffer[0] = (char)0xF0; // START_SYSEX
//		sendBuffer[1] = (char)0x76; // I2C_REQUEST
//		sendBuffer[2] = (char)address;
//		sendBuffer[3] = (char)0x00; // I2C_WRITE;
//		sendBuffer[4] = (char)(reg & 0x7F);
//		sendBuffer[5] = (char)(reg >> 7);
//		sendBuffer[6] = (char)(i2cData & 0x7F);
//		sendBuffer[7] = (char)(i2cData >> 7);
//		sendBuffer[8] = (char)0xF7;
//		SendOnConnection(9);
//
//	}
//
//	void FirmataSerial::I2C_ReadBytes(char address, char reg, char nbytes) {
//		sendBuffer[0] = (char)0xF0; // START_SYSEX
//		sendBuffer[1] = (char)0x76; // I2C_REQUEST
//		sendBuffer[2] = (char)address;
//		sendBuffer[3] = (char)0x08; // I2C_READ_ONCE;
//		sendBuffer[4] = (char)(reg & 0x7F);
//		sendBuffer[5] = (char)(reg >> 7);
//		sendBuffer[6] = (char)(nbytes & 0x7F);
//		sendBuffer[7] = (char)(nbytes >> 7);
//		sendBuffer[8] = (char)0xF7;
//		SendOnConnection(9);
//	}
//
//	void FirmataSerial::I2C_StartStream(char address, char reg, char nbytes) {
//		sendBuffer[0] = (char)0xF0; // START_SYSEX
//		sendBuffer[1] = (char)0x76; // I2C_REQUEST
//		sendBuffer[2] = (char)address;
//		sendBuffer[3] = (char)0x10; // I2C_READ_CONTINUOUSLY;
//		sendBuffer[4] = (char)(reg & 0x7F);
//		sendBuffer[5] = (char)(reg >> 7);
//		sendBuffer[6] = (char)(nbytes & 0x7F);
//		sendBuffer[7] = (char)(nbytes >> 7);
//		sendBuffer[8] = (char)0xF7;
//		SendOnConnection(9);
//	}
//
//	void FirmataSerial::I2C_StopStream(char address, char reg, char nbytes) {
//		sendBuffer[0] = (char)0xF0; // START_SYSEX
//		sendBuffer[1] = (char)0x76; // I2C_REQUEST
//		sendBuffer[2] = (char)address;
//		sendBuffer[3] = (char)0x18; // I2C_STOP_READING;
//		sendBuffer[4] = (char)(reg & 0x7F);
//		sendBuffer[5] = (char)(reg >> 7);
//		sendBuffer[6] = (char)(nbytes & 0x7F);
//		sendBuffer[7] = (char)(nbytes >> 7);
//		sendBuffer[8] = (char)0xF7;
//		SendOnConnection(9);
//	}
//#pragma endregion

#pragma region External
	char *msgBuf = new char[256];

	FIRMATA_API FirmataSerial *Firmata_ConnectSerial(char *port, int speed) {
		FirmataSerial *connection = new FirmataSerial();

		char szPortName[16];
		sprintf_s(szPortName, 16, "\\\\.\\%s", port);
		connection->connected = false;
		connection->handle = CreateFileA(
			static_cast<LPCSTR>(szPortName),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (connection->handle == INVALID_HANDLE_VALUE) {
			const DWORD error = GetLastError();
			sprintf_s(msgBuf, 256, " %d", error);
			OutputDebugStringA(msgBuf);
			return NULL;
		}

		COMMTIMEOUTS comTimeOut;
		comTimeOut.ReadIntervalTimeout = MAXDWORD;
		comTimeOut.ReadTotalTimeoutMultiplier = MAXDWORD;
		comTimeOut.ReadTotalTimeoutConstant = 0;
		SetCommTimeouts(connection->handle, &comTimeOut);

		DCB dcbSerialParameters = { 0 };
		if (!GetCommState(connection->handle, &dcbSerialParameters)) {
			//printf("failed to get current serial parameters");
			return NULL;
		}

		dcbSerialParameters.BaudRate = speed;
		dcbSerialParameters.ByteSize = 8;
		dcbSerialParameters.StopBits = ONESTOPBIT;
		dcbSerialParameters.Parity = NOPARITY;
		dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

		if (!SetCommState(connection->handle, &dcbSerialParameters)) {
			// printf("ALERT: could not set Serial port parameters\n");
			return NULL;
		}

		connection->connected = true;
		PurgeComm(connection->handle, PURGE_RXCLEAR | PURGE_RXCLEAR);

		OutputDebugStringA("Connected to server.\n");
		connection->RequestVersionReport();
		return connection;
	}

	FIRMATA_API void Firmata_DisconnectSerial(FirmataSerial *connection) {
		if (!connection->connected || connection == NULL)
			return;

		CloseHandle(connection->handle);
		connection->connected = false;
		delete connection;
	}

	//FIRMATA_API bool Firmata_ReadFromConnection(FirmataSerial *connection) {
	//	return connection->ReadFromConnection();
	//}
//#pragma region I2C
//	FIRMATA_API void Firmata_I2C_Config(FirmataSerial *connection) {
//		connection->I2C_Config();
//	}
//
//	FIRMATA_API void Firmata_I2C_WriteByte(FirmataSerial *connection, char address, char reg, char i2cData) {
//		connection->I2C_WriteByte(address, reg, i2cData);
//	}
//
//	FIRMATA_API void Firmata_I2C_RequestBytes(FirmataSerial *connection, char address, char reg, char nbytes) {
//		connection->I2C_ReadBytes(address, reg, nbytes);
//	}
//
//	FIRMATA_API unsigned int Firmata_I2C_ReadBytes(FirmataSerial *connection, char *i2cData) {
//		connection->dataSize = 0;
//		do {
//			connection->ReadFromConnection();
//		} while (connection->dataSize == 0);
//		i2cData = connection->i2cData;
//		return connection->dataSize;
//	}
//
//	FIRMATA_API I2C_Data Firmata_I2C_ReceiveData(FirmataSerial *connection) {
//		I2C_Data result = {};
//		result.address = connection->dataAddress;
//		result.reg = connection->dataRegister;
//		result.dataSize = connection->dataSize;
//		result.i2cData = connection->i2cData;
//
//		connection->dataSize = 0;
//
//		return result;
//	}
//	FIRMATA_API void Firmata_I2C_StartStream(FirmataSerial *connection, char address, char reg, char nbytes) {
//		connection->I2C_StartStream(address, reg, nbytes);
//	}
//
//	FIRMATA_API void Firmata_I2C_StopStream(FirmataSerial *connection, char address, char reg, char nbytes) {
//		connection->I2C_StopStream(address, reg, nbytes);
//	}
//#pragma endregion
#pragma endregion
}