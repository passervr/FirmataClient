extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include "Firmata.h"

	class BendSensor {
	public:
		Firmata * connection;

		char pin;
		float minValue;
		float maxValue;
		float range;

		BendSensor(Firmata *connection, char minValue, int maxValue, int pin);
		~BendSensor();

		float Read();
	};

	FIRMATA_API BendSensor *BendSensor_Start(Firmata *connection, char pin, int minValue, int maxValue);
	FIRMATA_API void BendSensor_Stop(BendSensor *sensor);

	FIRMATA_API float BendSensor_Read(BendSensor *sensor);
}