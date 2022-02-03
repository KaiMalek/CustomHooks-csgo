#include "globals.hpp"

namespace globals
{
	bool bAimbotting = false;
	bool bSendPacket = true;
	float last_doubletap;
	float doubletap_delta;
	bool should_setup_local_bones = false;
	float curtime = 0.f;
	bool got_feik_matrix = false;
	matrix3x4_t feik_matrix[128];
	Vector fakeAngle = Vector(0, 0, 0);
	Vector realAngle = Vector(0, 0, 0);
	Vector aimangle = Vector(0, 0, 0);
	CUserCmd* pCmd;
	int aaside = 1;
	int chockepack = 0;
	float CurtimeAnimation = 0.f;

}



















































