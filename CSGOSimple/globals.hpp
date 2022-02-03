#include "options.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "singleton.hpp"

namespace globals
{
	extern float CurtimeAnimation;
	extern int chockepack;
	extern CUserCmd* pCmd;
	extern 	bool got_feik_matrix;
	extern 	matrix3x4_t feik_matrix[128];
	extern Vector fakeAngle;
	extern Vector realAngle;
	extern 	Vector aimangle;
	extern int aaside;
	extern     float curtime;
	extern bool should_setup_local_bones;
	extern bool bSendPacket;
	extern float last_doubletap;
	extern float doubletap_delta;
	extern bool bAimbotting;
}