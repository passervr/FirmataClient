extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include "IMU.h"

	class MPU6050 : public IMU {
	public:
		void Init(Firmata *connection);
		Quat ProcessQuaternion();
	};

	FIRMATA_API bool MPU6050_Present(Firmata *connection);

	FIRMATA_API MPU6050 *MPU6050_Init(Firmata *connection);
	FIRMATA_API void MPU6050_Dispose(MPU6050 *sensor);
}