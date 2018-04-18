#include "BendSensor.h"



BendSensor::BendSensor(Firmata *connection, char pin, int minValue, int maxValue) {
	this->connection = connection;
	this->minValue = (float)minValue;
	this->maxValue = (float)maxValue;
	this->range = (float) (maxValue - minValue);
	this->pin = pin;

	Firmata_AnalogPinMode(connection, pin, Firmata::PinMode::Analog);
}


BendSensor::~BendSensor(){
	Firmata_AnalogPinMode(connection, pin, Firmata::PinMode::Disabled);
}

float BendSensor::Read() {
	float analogValue = (float)Firmata_AnalogRead(connection, pin);

	float value = (float)(analogValue - minValue) / range;
	return value;
}

FIRMATA_API BendSensor *BendSensor_Start(Firmata *connection, char pin, int minValue, int maxValue) {
	BendSensor *sensor = new BendSensor(connection, pin, minValue, maxValue);
	return sensor;
}

FIRMATA_API void BendSensor_Stop(BendSensor *sensor) {
	delete sensor;
}

FIRMATA_API float BendSensor_Read(BendSensor *sensor) {
	return sensor->Read();
}
