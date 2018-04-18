extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include "IMU.h"

	class BNO055 : public IMU {
	public:
		void Init(Firmata *connection);
		Quat ProcessQuaternion();
	};

	FIRMATA_API bool BNO055_Present(Firmata *connection);

	FIRMATA_API BNO055 *BNO055_Init(Firmata *connection);
	FIRMATA_API void BNO055_Dispose(BNO055 *bno055);
}