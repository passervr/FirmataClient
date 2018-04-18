#include "IMU.h"
//#include "BNO055.h"


short Sensor::ToShort(char b1, char b2) {
	unsigned short s1 = (unsigned short)(b1 << 8);
	short v = (short)(s1 | b2);
	return v;
}

FIRMATA_API void Sensor_WhoAmI(Sensor *sensor) {
	sensor->connection->I2C_ReadBytes(sensor->address, 0x00, 1);
}

FIRMATA_API void Sensor_WriteByte(Sensor *sensor, char reg, char data) {
	if (sensor == NULL || sensor->connection == NULL)
		return;

	sensor->connection->I2C_WriteByte(sensor->address, reg, data);
}

FIRMATA_API void Sensor_RequestData(Sensor *sensor, char reg, char nbytes) {
	Firmata_I2C_RequestBytes(sensor->connection, sensor->address, reg, nbytes);
}

FIRMATA_API void Sensor_StartStream(Sensor *sensor, char reg, char nbytes) {
	if (reg == 0)
		return;

	Firmata_I2C_StartStream(sensor->connection, sensor->address, reg, nbytes);
}

FIRMATA_API void Sensor_StopStream(Sensor *sensor, char reg, char nbytes) {
	if (reg == 0)
		return;

	Firmata_I2C_StopStream(sensor->connection, sensor->address, reg, nbytes);
}

FIRMATA_API void IMU_RequestRotation(IMU *sensor) {
	Sensor_RequestData(sensor, sensor->rotationRegister, 8);
}

FIRMATA_API void IMU_StartRotationStream(IMU *sensor) {
	Sensor_StartStream(sensor, sensor->rotationRegister, 8);
}

FIRMATA_API void IMU_StopRotationStream(IMU *sensor) {
	Sensor_StopStream(sensor, sensor->rotationRegister, 8);
}

void IMU::Receive() {
	if (connection == NULL)
		return;

	Firmata_ReadFromConnection(connection);
	if (connection->dataSize <= 0)
		return;

	if (connection->dataRegister == rotationRegister) {
		rotation = ProcessQuaternion();
		newRotation = true;
		connection->dataSize = 0;
	}
}

FIRMATA_API void IMU_Receive(IMU *sensor) {
	if (sensor == NULL)
		return;

	sensor->Receive();
}

FIRMATA_API Quat IMU_GetRotation(IMU *sensor) {
	if (sensor == NULL)
		return Quaternion::identity;

	return sensor->rotation;
}
