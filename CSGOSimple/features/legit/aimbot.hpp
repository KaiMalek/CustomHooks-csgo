#pragma once

#include "../../options.hpp"
#include "../../valve_sdk/csgostructs.hpp"

class CLegitbot
{
public:
	CLegitbot()
	{
		current_punch = last_punch = last_corrected = { 0, 0, 0 };
		target = nullptr;

		lastShotTick = shotsFired = 0;

		shot_delay_time = kill_delay_time = 0;
		shot_delay = kill_delay = false;
	}

	void Run(CUserCmd* cmd);
	bool IsEnabled(CUserCmd* cmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);

	legitbot_s settings;

	float GetFov();

private:
	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	void RCS(QAngle& angle, C_BasePlayer* target, CUserCmd* cmd);
	bool IsSilent();
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int& bestBone, float& fov, QAngle& angles);

	C_BasePlayer* target = nullptr;

	int lastShotTick;
	int shotsFired;

	bool shot_delay = false;
	int shot_delay_time = 0;

	bool kill_delay = false;
	int kill_delay_time = 0;

	QAngle current_punch = { 0, 0, 0 };
	QAngle last_punch = { 0, 0, 0 };
	QAngle last_corrected = { 0, 0, 0 };
};

inline CLegitbot* g_Legitbot;