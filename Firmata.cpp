
// Firmata.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "Firmata.h"

#define RECEIVEBUFFERSIZE	256
#define SENDBUFFERSIZE		256
#define DIGITALPINCOUNT		13
#define ANALOGPINCOUNT		6
#define DATABUFFERSIZE		256

extern "C" {

#pragma region Tmp
	// This is an example of an exported variable
	FIRMATA_API int nMovements = 0;

	// This is an example of an exported function.
	FIRMATA_API int fnMovements(void)
	{
		return 42;
	}


	struct threadData {
		int m_id;
		threadData() : m_id(0) {}
	};
#pragma endregion

	Firmata::Firmata() {
		receiveBuffer = new char[RECEIVEBUFFERSIZE];
		receiveBufferSize = 0;

		sendBuffer = new char[SENDBUFFERSIZE];

		digital = new bool[DIGITALPINCOUNT];
		for (int i = 0; i < DIGITALPINCOUNT; i++)
			digital[i] = false;

		analog = new unsigned short[ANALOGPINCOUNT];
		for (int i = 0; i < ANALOGPINCOUNT; i++)
			analog[i] = 0;

		i2cData = new char[DATABUFFERSIZE];
		for (int i = 0; i < DATABUFFERSIZE; i++)
			i2cData[i] = 0;
		dataSize = 0;

		ready = false;
		i2cConfigured = false;
	}

	Firmata::~Firmata() {
		delete receiveBuffer;
		delete sendBuffer;
		delete digital;
		delete analog;
		delete i2cData;
	}

	bool Firmata::ReadFromConnection() {
		bool retry = true;
		while (retry) {
			retry = false;
			ReadBuffer();

			if (receiveBufferSize > 0)
				retry = ProcessBuffer();
		}

		return (receiveBufferSize > 0);
	}

	void Firmata::Reset() {
		sprintf_s(msgBuf, 256, "SYSTEM RESET\n");
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xFF;
		SendOnConnection(1);
	}

	void Firmata::RequestVersionReport() {
		sprintf_s(msgBuf, 256, "REQUEST VERSION REPORT\n");
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF9;
		SendOnConnection(1);
	}

	void Firmata::QueryFirmware() {
		sprintf_s(msgBuf, 256, "REPORT FIRMWARE\n");
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF0;
		sendBuffer[1] = (char)0x79;
		sendBuffer[2] = (char)0xF7;
		SendOnConnection(3);
	}

	void Firmata::SetDigitalPinMode(char pin, Firmata::PinMode mode) {
		if (mode == Input || mode == InputPullup) {
			sendBuffer[0] = (char)((pin < 8) ? 0xD0 : 0xD1);
			sendBuffer[1] = (char)0x01;
			SendOnConnection(2);

			sendBuffer[0] = (char)0xF4;
			sendBuffer[1] = (char)pin;
			sendBuffer[2] = (char)((mode == Input) ? 0x00 : 0x0B);
			SendOnConnection(3);
		}
		else {
			sendBuffer[0] = (char)((pin < 8) ? 0xD0 : 0xD1);
			sendBuffer[1] = (char)0x00;
			SendOnConnection(2);

			sendBuffer[0] = (char)0xF4;
			sendBuffer[1] = (char)pin;
			sendBuffer[2] = (char)0x01;
			SendOnConnection(3);
		}
	}

	void Firmata::DigitalWrite(char pin, bool value) {
		sprintf_s(msgBuf, 256, "SET DIGITAL PIN VALUE: %d %d\n", pin, value);
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF5;
		sendBuffer[1] = (char)pin;
		sendBuffer[2] = (char)(value ? 1 : 0);
		SendOnConnection(3);
	}

	bool Firmata::DigitalRead(char pin) {
		return digital[pin];
	}

	void Firmata::SetAnalogPinMode(char pin, Firmata::PinMode mode) {
		if (mode == Analog) {
			sendBuffer[0] = (char)(0xC0 | (pin & 0x0F));
			sendBuffer[1] = (char)0x01;
			SendOnConnection(2);
		}
		else {
			sendBuffer[0] = (char)(0xC0 | (pin & 0x0F));
			sendBuffer[1] = (char)0x00;
			SendOnConnection(2);
		}
	}

	unsigned short Firmata::AnalogRead(char pin) {
		return analog[pin];
	}


	void Firmata::ReadBuffer() {
		// skip until command byte with highest bit set
		int maxTries = 10;
		receiveBufferSize = 0;
		receiveBuffer[0] = 0;
		bool isCmd = false;
		do {
			receiveBuffer[0] = ReadByte();

			isCmd = (receiveBuffer[0] & 0x80) != 0;
		} while (!isCmd && maxTries-- > 0);
		//sprintf_s(msgBuf, 256, "\n");
		//OutputDebugStringA(msgBuf);

		if (!isCmd && maxTries <= 0)
			return;

		char cmd = receiveBuffer[0] & 0xF0;
		ready = true;

		//OutputDebugStringA(" cmd");

		switch (cmd) {
		case (char)0x90:
		case (char)0xE0:
			Read3Bytes();
			break;
		case (char)0xF0:
			ReadingFx(receiveBuffer[0]);
		}

		//OutputDebugStringA("\n");
	}

	void Firmata::Read3Bytes() {
		receiveBuffer[1] = ReadByteBlocking();
		receiveBuffer[2] = ReadByteBlocking();
		receiveBufferSize = 3;
	}

	void Firmata::ReadUntil(char endByte) {
		receiveBufferSize = 1;
		do {
			receiveBuffer[receiveBufferSize] = ReadByteBlocking();
			receiveBufferSize++;
		} while ((char)receiveBuffer[receiveBufferSize - 1] != endByte && receiveBufferSize < RECEIVEBUFFERSIZE);
	}

	void Firmata::ReadingFx(char cmd) {
		switch (cmd) {
		case (char)0xF9:
			Read3Bytes();
			break;
		case (char)0xF0:
			ReadUntil((char)0xF7);
			break;
		}
	}

	bool Firmata::ProcessBuffer() {
		char cmd = receiveBuffer[0] & 0xF0;
		switch (cmd) {
		case (char)0xE0:
			ProcessAnalogInput();
			break;
		case (char)0x90:
			Process8DigitalInput();
			break;
		case (char)0xF0:
			return ProcessBytesFx(receiveBuffer[0]);
			break;
		}
		return false;
	}

	void Firmata::ProcessAnalogInput() {
		char pin = (char)receiveBuffer[0] & 0x0F;
		char msb = receiveBuffer[2] & 0x7F;
		char lsb = receiveBuffer[1] & 0x7F;
		unsigned short value = ((unsigned short)msb << 7) + lsb;

		sprintf_s(msgBuf, 256, "ANALOG INPUT %d: %d\n", pin, value);
		OutputDebugStringA(msgBuf);


		if (pin < ANALOGPINCOUNT)
			analog[pin] = value;
	}

	void Firmata::Process8DigitalInput() {
		char port = (char)receiveBuffer[0] & 0x0F;
		char values = (((char)receiveBuffer[2] & 0x01) << 7) + (char)receiveBuffer[1];
		for (int i = 0; i < 8; i++) {
			char pin = port * 8 + i;
			if (pin < DIGITALPINCOUNT) {
				char value = ((values >> i) & 0x01) == 0x01;
				digital[pin] = value;
			}
		}
	}

	bool Firmata::ProcessBytesFx(char cmd) {
		switch (cmd) {
		case (char)0xF0:
			return ProcessBytesF0(cmd);
			break;
		case (char)0xF9:
			ProcessBytesF9(cmd);
			break;
		default:
			sprintf_s(msgBuf, 256, "ProcessBytesFx: Unknown message 0x%02hhX ", cmd);
			OutputDebugStringA(msgBuf);
			break;
		}
		return false;
	}

	bool Firmata::ProcessBytesF0(char cmd) {
		char cmd2 = receiveBuffer[1];
		switch (cmd2) {
		case (char)0x71: // STRING_DATA
			ProcessBytes0x71();
			return true;
		case (char)0x77: // I2C_REPLY
			ProcessBytes0x77();
			break;
		case (char)0x079: // REPORT_FIRMWARE
			ProcessBytes0x79();
			break;
		default:
			sprintf_s(msgBuf, 256, "ProcessBytesF0: Unknown message 0x%02hhX ", cmd2);
			OutputDebugStringA(msgBuf);
			break;
		}
		return false;
	}

	void Firmata::ProcessBytesF9(char cmd) {
		sprintf_s(msgBuf, 256, "PROTOCOL VERSION %d.%d\n", receiveBuffer[1], receiveBuffer[2]);
		OutputDebugStringA(msgBuf);
	}

	// Process STRING_DATA
	void Firmata::ProcessBytes0x71() {
		OutputDebugStringA("STRING_DATA: ");

		int i = 2;
		while ((char)receiveBuffer[i] != (char)0xF7) {
			sprintf_s(msgBuf, 256, "%c", receiveBuffer[i]);
			OutputDebugStringA(msgBuf);
			i += 1;
		}
		OutputDebugStringA("\n");
	}

	void Firmata::ProcessBytes0x77() {
		dataAddress = ((receiveBuffer[3] & 0x01) << 7) + receiveBuffer[2];
		dataRegister = ((receiveBuffer[5] & 0x01) << 7) + receiveBuffer[4];

		sprintf_s(msgBuf, 256, "I2C_REPLY: 0x%02hhX 0x%02hhX -", dataAddress, dataRegister);
		OutputDebugStringA(msgBuf);

		int n = 0;
		char byte = receiveBuffer[6 + n * 2];
		while (byte != (char)0xF7 && n < DATABUFFERSIZE) {
			n++;
			byte = receiveBuffer[6 + n * 2];
		}

		for (int j = 0; j < n; j++) {
			int index = 6 + j * 2;
			int value = ((receiveBuffer[index + 1] & 0x01) << 7) + receiveBuffer[index];
			i2cData[j] = (char)value;

			sprintf_s(msgBuf, 256, " 0x%02hhX", i2cData[j]);
			OutputDebugStringA(msgBuf);
		}
		dataSize = n;
		OutputDebugStringA("\n");
	}

	void Firmata::ProcessBytes0x79() {
		sprintf_s(msgBuf, 256, "REPORT_FIRMWARE %d.%d ", receiveBuffer[2], receiveBuffer[3]);
		OutputDebugStringA(msgBuf);

		int i = 4;
		while ((char)receiveBuffer[i] != (char)0xF7) {
			sprintf_s(msgBuf, 256, "%c", receiveBuffer[i]);
			OutputDebugStringA(msgBuf);
			i += 1;
		}
		OutputDebugStringA("\n");
	}

#pragma region I2C
	void Firmata::I2C_Config() {
		if (i2cConfigured)
			return;

		sprintf_s(msgBuf, 256, "I2C_CONFIG\n");
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF0; // START_SYSEX
		sendBuffer[1] = (char)0x78; // I2C_CONFIG
		sendBuffer[2] = (char)0xF7;
		SendOnConnection(3);

		i2cConfigured = true;
	}

	void Firmata::I2C_WriteByte(char address, char reg, char data) {
		sprintf_s(msgBuf, 256, "I2C_WRITE: 0x%02hhX 0x%02hhX - 0x%02hhX\n", address, reg, data);
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF0; // START_SYSEX
		sendBuffer[1] = (char)0x76; // I2C_REQUEST
		sendBuffer[2] = (char)address;
		sendBuffer[3] = (char)0x00; // I2C_WRITE;
		sendBuffer[4] = (char)(reg & 0x7F);
		sendBuffer[5] = (char)(reg >> 7);
		sendBuffer[6] = (char)(data & 0x7F);
		sendBuffer[7] = (char)(data >> 7);
		sendBuffer[8] = (char)0xF7;
		SendOnConnection(9);

	}

	void Firmata::I2C_ReadBytes(char address, char reg, char nbytes) {
		sprintf_s(msgBuf, 256, "I2C_READ_ONCE: 0x%02hhX 0x%02hhX [%d]\n", address, reg, nbytes);
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF0; // START_SYSEX
		sendBuffer[1] = (char)0x76; // I2C_REQUEST
		sendBuffer[2] = (char)address;
		sendBuffer[3] = (char)0x08; // I2C_READ_ONCE;
		sendBuffer[4] = (char)(reg & 0x7F);
		sendBuffer[5] = (char)(reg >> 7);
		sendBuffer[6] = (char)(nbytes & 0x7F);
		sendBuffer[7] = (char)(nbytes >> 7);
		sendBuffer[8] = (char)0xF7;
		SendOnConnection(9);
	}

	void Firmata::I2C_StartStream(char address, char reg, char nbytes) {
		sprintf_s(msgBuf, 256, "I2C_READ_CONTINUOUSLY: 0x%02hhX 0x%02hhX [%d]\n", address, reg, nbytes);
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF0; // START_SYSEX
		sendBuffer[1] = (char)0x76; // I2C_REQUEST
		sendBuffer[2] = (char)address;
		sendBuffer[3] = (char)0x10; // I2C_READ_CONTINUOUSLY;
		sendBuffer[4] = (char)(reg & 0x7F);
		sendBuffer[5] = (char)(reg >> 7);
		sendBuffer[6] = (char)(nbytes & 0x7F);
		sendBuffer[7] = (char)(nbytes >> 7);
		sendBuffer[8] = (char)0xF7;
		SendOnConnection(9);
	}

	void Firmata::I2C_StopStream(char address, char reg, char nbytes) {
		sprintf_s(msgBuf, 256, "I2C_STOP_READING: 0x%02hhX 0x%02hhX [%d]\n", address, reg, nbytes);
		OutputDebugStringA(msgBuf);

		sendBuffer[0] = (char)0xF0; // START_SYSEX
		sendBuffer[1] = (char)0x76; // I2C_REQUEST
		sendBuffer[2] = (char)address;
		sendBuffer[3] = (char)0x18; // I2C_STOP_READING;
		sendBuffer[4] = (char)(reg & 0x7F);
		sendBuffer[5] = (char)(reg >> 7);
		sendBuffer[6] = (char)(nbytes & 0x7F);
		sendBuffer[7] = (char)(nbytes >> 7);
		sendBuffer[8] = (char)0xF7;
		SendOnConnection(9);
	}
#pragma endregion


#pragma region External
	FIRMATA_API bool Firmata_ReadFromConnection(Firmata *connection) {
		if (connection == NULL)
			return false;

		bool read = connection->ReadFromConnection();
		return read;
	}

	FIRMATA_API void Firmata_Reset(Firmata *connection) {
		if (connection == NULL)
			return;

		connection->Reset();
	}

	FIRMATA_API void Firmata_QueryFirmware(Firmata *connection) {
		if (connection == NULL)
			return;

		connection->QueryFirmware();
	}

	FIRMATA_API void Firmata_DigitalPinMode(Firmata *connection, int pin, Firmata::PinMode mode) {
		if (connection == NULL)
			return;

		connection->SetDigitalPinMode(pin, mode);
	}

	FIRMATA_API void Firmata_DigitalWrite(Firmata *connection, int pin, bool value) {
		if (connection == NULL)
			return;

		connection->DigitalWrite(pin, value);
	}

	FIRMATA_API bool Firmata_DigitalRead(Firmata *connection, int pin) {
		if (connection == NULL)
			return false;

		return connection->DigitalRead(pin);
	}

	FIRMATA_API void Firmata_AnalogPinMode(Firmata *connection, int pin, Firmata::PinMode mode) {
		if (connection == NULL)
			return;

		connection->SetAnalogPinMode(pin, mode);
	}

	FIRMATA_API unsigned short Firmata_AnalogRead(Firmata *connection, int pin) {
		if (connection == NULL)
			return 0;

		return connection->AnalogRead(pin);
	}

#pragma region I2C
	FIRMATA_API void Firmata_I2C_Config(Firmata *connection) {
		if (connection == NULL)
			return;

		connection->I2C_Config();
	}

	FIRMATA_API void Firmata_I2C_WriteByte(Firmata *connection, char address, char reg, char data) {
		if (connection == NULL)
			return;

		connection->I2C_WriteByte(address, reg, data);
	}

	FIRMATA_API void Firmata_I2C_RequestBytes(Firmata *connection, char address, char reg, char nbytes) {
		if (connection == NULL)
			return;

		connection->I2C_ReadBytes(address, reg, nbytes);
	}

	FIRMATA_API unsigned int Firmata_I2C_ReadBytes(Firmata *connection, char *data) {
		if (connection == NULL)
			return 0;

		connection->dataSize = 0;
		do {
			connection->ReadFromConnection();
		} while (connection->dataSize == 0);
		data = connection->i2cData;
		return connection->dataSize;
	}

	FIRMATA_API I2C_Data Firmata_I2C_ReceiveData(Firmata *connection) {
		I2C_Data result = {};
		if (connection == NULL)
			return result;

		result.address = connection->dataAddress;
		result.reg = connection->dataRegister;
		result.dataSize = connection->dataSize;
		result.data = connection->i2cData;

		connection->dataSize = 0;

		return result;
	}
	FIRMATA_API void Firmata_I2C_StartStream(Firmata *connection, char address, char reg, char nbytes) {
		if (connection == NULL)
			return;

		connection->I2C_StartStream(address, reg, nbytes);
	}

	FIRMATA_API void Firmata_I2C_StopStream(Firmata *connection, char address, char reg, char nbytes) {
		if (connection == NULL)
			return;
		connection->I2C_StopStream(address, reg, nbytes);
	}
#pragma endregion
#pragma endregion


}

