#include "MPU6050.h"

#define ADDRESS1 0x68
#define ADDRESS2 0x69
#define ADDRESS ADDRESS1

Quat MPU6050::ProcessQuaternion() {
	char *data = connection->i2cData;
	unsigned int dataSize = connection->dataSize;
	float wReading = (float)ToShort(data[1], data[0]) / 16384; // 1 quaternion = 2^14 LSB
	float xReading = (float)ToShort(data[3], data[2]) / 16384; // 1 quaternion = 2^14 LSB
	float yReading = (float)ToShort(data[5], data[4]) / 16384; // 1 quaternion = 2^14 LSB
	float zReading = (float)ToShort(data[7], data[6]) / 16384; // 1 quaternion = 2^14 LSB
	Quat q = Quaternion(xReading, yReading, zReading, wReading);
	return q;
}

void MPU6050::Init(Firmata *firmataConnection) {
	connection = firmataConnection;

	address = ADDRESS;

	accelerometer = 0x08;		// ACCEL_XOUT_H
	gyroscope = 0x14;			// GYRO_XOUT_H

	if (connection == NULL)
		return;

	connection->I2C_Config();

	// Clear sleep mode bit (6), enable all sensors
	connection->I2C_WriteByte(address, 0x6B, 0x00);	// PWR_MGMT_1 =

	// Delay 100 ms for PLL to get established on x-axis gyro; should check for PLL ready interrupt
	// get stable time source
	Sleep(100);

	// Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
	// Configure Gyro and Accelerometer
	// Disable FSYNC and set accelerometer and gyro bandwidth to 44 and 42 Hz, respectively; 
	// DLPF_CFG = bits 2:0 = 010; this sets the sample rate at 1 kHz for both
	// Maximum delay is 4.9 ms which is just over a 200 Hz maximum rate
	connection->I2C_WriteByte(address, 0x6B, 0x01);	// PWR_MGMT_1 =

	Sleep(200);

	connection->I2C_WriteByte(address, 0x1A, 0x03);	// CONFIG =

	// Set sample rate = gyroscope output rate/(1 + SMPLRT_DIV)
	connection->I2C_WriteByte(address, 0x19, 0x04);	// SMPLRT_DIV =

	// Use a 200 Hz rate; the same rate set in CONFIG above

	// Configure Interrupts and Bypass Enable
	// Set interrupt pin active high, push-pull, and clear on read of INT_STATUS, enable I2C_BYPASS_EN so additional chips 
	// can join the I2C bus and all can be controlled by the Arduino as master
	connection->I2C_WriteByte(address, 0x37, 0x22); // INT_PIN_CFG =

	// Enable i2cData ready (bit 0) interrupt
	connection->I2C_WriteByte(address, 0x38, 0x01); // INT_ENABLE =

	Sleep(100);
}

FIRMATA_API bool MPU6050_Present(Firmata *connection) {
	if (connection == NULL)
		return false;

	// Make sure I2C has been configured
	connection->I2C_Config();

	// WhoAmI
	connection->I2C_ReadBytes(ADDRESS, 0x75, 1);	// WHO_AM_I
	int i = 0;
	while (i < 10 && !Firmata_ReadFromConnection(connection)) {
		OutputDebugStringA(".");
		Sleep(100);
		i++;
	}

	// WHO_AM_I == upper 6 bits of the 7-bit ADDRESS ?
	return (connection->i2cData[0] == (char)(ADDRESS & 0x7E));
}

FIRMATA_API MPU6050 *MPU6050_Init(Firmata *connection) {
	if (connection == NULL)
		return NULL;

	MPU6050 *sensor = new MPU6050();
	sensor->Init(connection);
	return sensor;
}

FIRMATA_API void MPU6050_Dispose(MPU6050 *sensor) {
	delete sensor;
}