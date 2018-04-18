#include "stdafx.h"
#include <math.h>
#include "Quaternion.h"

Quaternion::Quaternion() {
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}

Quaternion::Quaternion(float _x, float _y, float _z, float _w) {
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

Quaternion::~Quaternion() {}

const Quaternion Quaternion::identity = Quaternion(0, 0, 0, 1);