extern "C" {
#ifdef FIRMATA_EXPORTS
#define FIRMATA_API __declspec(dllexport)
#else
#define FIRMATA_API __declspec(dllimport)
#endif

#pragma once
#include "IMU.h"

	FIRMATA_API IMU *IMU_Init(Firmata *connection);
}

