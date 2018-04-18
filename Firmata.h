// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MOVEMENTS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MOVEMENTS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include <windows.h>
#include "Quaternion.h"

	extern FIRMATA_API int nMovements;

	class Firmata {
	public:
		Firmata();
		~Firmata();

		char *receiveBuffer;
		int receiveBufferSize;

		char *sendBuffer;

		bool connected;
		bool ready;

		bool *digital;
		unsigned short *analog;

		char dataAddress;
		char dataRegister;
		char *i2cData;
		unsigned int dataSize;

		enum PinMode {
			Disabled,
			Input,
			Output,
			InputPullup,
			Analog
		};

		virtual void SendOnConnection(int n) = 0;
		bool ReadFromConnection();
		virtual char ReadByte() = 0;
		virtual char ReadByteBlocking() = 0;

		void Reset();
		void RequestVersionReport();
		void QueryFirmware();

		void SetDigitalPinMode(char pin, PinMode mode);
		void DigitalWrite(char pin, bool value);
		bool DigitalRead(char pin);

		void SetAnalogPinMode(char pin, PinMode mode);
		unsigned short AnalogRead(char pin);

		void I2C_Config();
		void I2C_WriteByte(char address, char reg, char data);
		void I2C_ReadBytes(char address, char reg, char nbytes);
		void I2C_StartStream(char address, char reg, char nbytes);
		void I2C_StopStream(char address, char reg, char nbytes);

	protected:
		bool i2cConfigured = false;
		void ReadBuffer();
		void Read3Bytes();
		void ReadUntil(char endByte);
		void ReadingFx(char cmd);

		bool ProcessBuffer();
		void ProcessAnalogInput();
		void Process8DigitalInput();

		bool ProcessBytesFx(char cmd);
		bool ProcessBytesF0(char cmd);
		void ProcessBytesF9(char cmd);

		void ProcessBytes0x71();
		void ProcessBytes0x77();
		void ProcessBytes0x79();

		char *msgBuf = new char[256];
	};

	FIRMATA_API bool Firmata_ReadFromConnection(Firmata *connection);

	FIRMATA_API void Firmata_Reset(Firmata *connection);
	FIRMATA_API void Firmata_QueryFirmware(Firmata *connection);

	FIRMATA_API void Firmata_DigitalPinMode(Firmata *connection, int pin, Firmata::PinMode mode);
	FIRMATA_API void Firmata_DigitalWrite(Firmata *connection, int pin, bool value);
	FIRMATA_API bool Firmata_DigitalRead(Firmata *connection, int pin);

	FIRMATA_API void Firmata_AnalogPinMode(Firmata *connection, int pin, Firmata::PinMode mode);
	FIRMATA_API unsigned short Firmata_AnalogRead(Firmata *connection, int pin);

	FIRMATA_API void Firmata_I2C_Config(Firmata *connection);
	FIRMATA_API void Firmata_I2C_WriteByte(Firmata *connection, char address, char reg, char data);
	FIRMATA_API void Firmata_I2C_RequestBytes(Firmata *connection, char address, char reg, char nbytes);
	FIRMATA_API unsigned int Firmata_I2C_ReadBytes(Firmata *connection, char *data);

	typedef struct {
		char			address;
		char			reg;
		unsigned int	dataSize;
		char*			data;
	} I2C_Data;
	FIRMATA_API I2C_Data Firmata_I2C_ReceiveData(Firmata *connection);
	FIRMATA_API void Firmata_I2C_StartStream(Firmata *connection, char address, char reg, char nbytes);
	FIRMATA_API void Firmata_I2C_StopStream(Firmata *connection, char address, char reg, char nbytes);

}