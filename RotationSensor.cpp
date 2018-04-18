#include "RotationSensor.h"
#include "BNO055.h"
#include "MPU6050.h"

IMU *IMU_Init(Firmata *connection) {
	if (BNO055_Present(connection)) {
		OutputDebugStringA("Found BNO055 sensor\n");
		return BNO055_Init(connection);
	}
	OutputDebugStringA("No BNO055 found\n");

	if (MPU6050_Present(connection)) {
		OutputDebugStringA("Found MPU6050 sensor\n");
		return MPU6050_Init(connection);
	}
	OutputDebugStringA("No MPU6050 found\n");

	return NULL;
}


