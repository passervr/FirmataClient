#ifndef QUATERNION_H
#define QUATERNION_H
#pragma once
//struct Vector3;

extern "C" {
	typedef struct Quat {
		float x;
		float y;
		float z;
		float w;
	} Quat;
}


	struct Quaternion : Quat {
		float x;
		float y;
		float z;
		float w;

		Quaternion();
		Quaternion(float _x, float _y, float _z, float _w);
		~Quaternion();

		const static Quaternion identity;
	};
#endif