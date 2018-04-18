extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include "Firmata.h"

	class Sensor {
	public:
		Firmata *connection;
		char	address;

		short ToShort(char b1, char b2);

	};
	FIRMATA_API void Sensor_WriteByte(Sensor *sensor, char reg, char data);
	FIRMATA_API void Sensor_WhoAmI(Sensor *sensor);


	class IMU : public Sensor {
	public:
		char	accelerometer;
		char	gyroscope;
		char	rotationRegister;

		Quat	rotation;
		bool	newRotation;
		virtual Quat	ProcessQuaternion() = 0;

		void Receive();
	};
	//FIRMATA_API Sensor *IMU_Init(char address, char accRegister, char gyroRegister);
	FIRMATA_API void IMU_Receive(IMU *sensor);

	//FIRMATA_API void IMU_SetScale(IMU *sensor, char scale);
	//FIRMATA_API void IMU_StartAccStream(Sensor *sensor);
	//FIRMATA_API void IMU_StopAccStream(Sensor *sensor);

	FIRMATA_API void IMU_RequestRotation(IMU *sensor);
	FIRMATA_API void IMU_StartRotationStream(IMU *sensor);
	FIRMATA_API void IMU_StopRotationStream(IMU *sensor);
	FIRMATA_API Quat IMU_GetRotation(IMU *sensor);

}