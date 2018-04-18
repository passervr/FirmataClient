#include "BNO055.h"

#define ADDRESS 0x29

Quat BNO055::ProcessQuaternion() {
	char *data = connection->i2cData;
	unsigned int dataSize = connection->dataSize;
	float wReading = (float)ToShort(data[1], data[0]) / 16384; // 1 quaternion = 2^14 LSB
	float xReading = (float)ToShort(data[3], data[2]) / 16384; // 1 quaternion = 2^14 LSB
	float yReading = (float)ToShort(data[5], data[4]) / 16384; // 1 quaternion = 2^14 LSB
	float zReading = (float)ToShort(data[7], data[6]) / 16384; // 1 quaternion = 2^14 LSB
	Quat q = Quaternion(xReading, yReading, zReading, wReading);
	return q;
}

void BNO055::Init(Firmata *firmataConnection) {
	connection = firmataConnection;
	address = ADDRESS;
	accelerometer = 0x08;		// ACC_DATA_X_LSB
	gyroscope = 0x14;
	rotationRegister = 0x20;	// QUA_W_LSB

	if (connection == NULL)
		return;

	connection->I2C_Config();

	connection->I2C_WriteByte(address, 0x07, 0x00);	// PAGE_ID = 0
	connection->I2C_WriteByte(address, 0x3D, 0x0C);	// OPR_MODE = NDOF
}

FIRMATA_API bool BNO055_Present(Firmata *connection) {
	if (connection == NULL)
		return false;

	// Make sure I2C has been configured
	connection->I2C_Config();

	// WhoAmI
	connection->I2C_ReadBytes(ADDRESS, 0x00, 1);	// CHIP_ID
	int i = 0;
	while (i < 10) {
		if (Firmata_ReadFromConnection(connection)) {
			// CHIP_ID == 0xA0 ?
			if (connection->i2cData[0] == (char)0xA0)
				return true;
		}
		OutputDebugStringA(".");
		Sleep(10);
		i++;
	}

	return false;
	//// CHIP_ID == 0xA0 ?
	//return (connection->i2cData[0] == (char)0xA0);
}

FIRMATA_API BNO055 *BNO055_Init(Firmata *connection) {
	if (connection == NULL)
		return NULL;

	BNO055 *bno055 = new BNO055();
	bno055->Init(connection);
	return bno055;
}

FIRMATA_API void BNO055_Dispose(BNO055 *bno055) {
	delete bno055;
}