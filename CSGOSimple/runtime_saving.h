#pragma once
#include "valve_sdk/math/Vector.hpp"


struct HitmarkerInfoStruct
{
	float HitTime = 0.f;
	float Damage = 0.f;
};

class runtime_saving
{
public:
	HitmarkerInfoStruct hitmarkerinformation;
};

inline runtime_saving saving;
