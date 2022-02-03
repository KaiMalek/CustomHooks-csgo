#include "aimbot.hpp"
#include "autowall.hpp"

#include "../../helpers/input.hpp"
#include "../../helpers/utils.hpp"
#include "../../helpers/math.hpp"

float CLegitbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}

bool CLegitbot::IsLineGoesThroughSmoke(Vector startPos, Vector endPos)
{
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector, Vector))Utils::PatternScan2("client.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(startPos, endPos);
}

bool CLegitbot::IsEnabled(CUserCmd* cmd)
{
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer)
		return false;

	if (!g_LocalPlayer->IsAlive())
		return false;

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsGun())
		return false;

	settings = g_Options.weapons[weapon->m_Item().m_iItemDefinitionIndex()].legit;

	if (!settings.enabled)
		return false;

	if (!weapon->HasBullets())
		return false;

	return (cmd->buttons & IN_ATTACK);

	return (cmd->buttons & IN_ATTACK) || (settings.autofire.enabled && GetAsyncKeyState(settings.autofire.hotkey));
}

void CLegitbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	auto smooth_value = max(1.0f, settings.smooth);

	Vector current, aim;

	Math::AngleVectors(currentAngle, current);
	Math::AngleVectors(aimAngle, aim);

	const Vector delta = aim - current;
	const Vector smoothed = current + delta / smooth_value;

	Math::VectorAngles(smoothed, angle);
}

void CLegitbot::RCS(QAngle& angle, C_BasePlayer* target, CUserCmd* cmd)
{
	if (!settings.rcs.enabled || shotsFired < settings.rcs.start - 1)
		return;

	if (!settings.rcs.x && !settings.rcs.y)
		return;

	auto x = settings.rcs.x;
	auto y = settings.rcs.y;

	static auto weapon_recoil_scale = g_CVar->FindVar("weapon_recoil_scale");
	float scale = weapon_recoil_scale->GetFloat();

	if (scale != 2.f)
	{
		x = scale * x / 2.f;
		y = scale * y / 2.f;
	}

	x = scale *= 1.26f;
	y = scale;

	if (x <= 0 && y <= 0)
		return;

	QAngle punch = { };

	if (target)
		punch = { current_punch.pitch * x,  current_punch.yaw * y, 0 };
	else if (settings.rcs.type == 0)
		punch = { (current_punch.pitch - last_punch.pitch) * x, (current_punch.yaw - last_punch.yaw) * y, 0 };

	if ((punch.pitch != 0.f || punch.yaw != 0.f) && g_LocalPlayer->m_aimPunchAngle().roll == 0.f)
	{
		angle -= punch;
		angle.Normalize();
	}

	last_corrected = punch;
}

bool CLegitbot::IsSilent()
{
	if (settings.silent2 == 2)
		return !(shotsFired > 0 || !settings.silent2 || !settings.silent_fov);
	if (settings.silent2 == 1)
		return !(!settings.silent2 || !settings.silent_fov);
	if (settings.silent2 == 0)
		return !(shotsFired > 0 || !settings.silent2 || !settings.silent_fov);
}

float CLegitbot::GetFov()
{
	if (IsSilent())
		return settings.silent_fov;

	return settings.fov;
}

C_BasePlayer* CLegitbot::GetClosestPlayer(CUserCmd* cmd, int& bestBone, float& bestFov, QAngle& bestAngles)
{
	if (target && !kill_delay && settings.kill_delay > 0 && !target->IsAlive())
	{
		target = nullptr;
		kill_delay = true;
		kill_delay_time = int(GetTickCount()) + settings.kill_delay;
	}
	if (kill_delay)
	{
		if (kill_delay_time <= int(GetTickCount()))
			kill_delay = false;
		else
			return nullptr;
	}

	target = nullptr;

	std::vector<int> hitboxes;

	if (settings.hitboxes.head)
		hitboxes.emplace_back(HITBOX_HEAD);

	if (settings.hitboxes.chest)
	{
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);
		hitboxes.emplace_back(HITBOX_CHEST);
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);
	}

	if (settings.hitboxes.hands)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOREARM);
		hitboxes.emplace_back(HITBOX_LEFT_HAND);
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);

		hitboxes.emplace_back(HITBOX_RIGHT_FOREARM);
		hitboxes.emplace_back(HITBOX_RIGHT_HAND);
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
	}

	if (settings.hitboxes.legs)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);

		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);
	}

	const Vector eyePos = g_LocalPlayer->GetEyePos();

	for (auto i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player || !player->IsAlive() || !player->IsPlayer() || player->m_bGunGameImmunity())
			continue;

		if (!player->IsEnemy())
			continue;

		for (const auto hitbox : hitboxes)
		{
			Vector hitboxPos = player->GetHitboxPos(hitbox);
			QAngle ang;
			Math::VectorAngles(hitboxPos - eyePos, ang);
			const float fov = GetFovToPlayer(cmd->viewangles + last_punch * 2.f, ang);

			if (fov > GetFov())
				continue;

			if (!g_LocalPlayer->CanSeePlayer(player, player->abs_origin()))
				continue;

			if (settings.smoke_check && IsLineGoesThroughSmoke(eyePos, hitboxPos))
				continue;

			if (bestFov > fov)
			{
				bestBone = hitbox;
				bestAngles = ang;
				bestFov = fov;
				target = player;
			}
		}
	}

	return target;
}

void CLegitbot::Run(CUserCmd* cmd)
{
	if (int(GetTickCount()) > lastShotTick + 50)
		shotsFired = 0;

	if (!IsEnabled(cmd))
	{
		last_punch = { 0, 0, 0 };
		shot_delay = false;
		kill_delay = false;
		target = nullptr;
		return;
	}

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;

	if (settings.flash_check && g_LocalPlayer->IsFlashed())
		return;

	auto angles = cmd->viewangles;
	const auto current = angles;

	float fov = FLT_MAX;

	int bestBone = -1;

	if (GetClosestPlayer(cmd, bestBone, fov, angles))
	{
		if ((!settings.silent2 || settings.silent2 == 2) && !shot_delay && settings.shot_delay > 0 && !shotsFired)
		{
			shot_delay = true;
			shot_delay_time = int(GetTickCount()) + settings.shot_delay;
		}

		if (fov <= .5)
			shot_delay = false;

		if (shot_delay && shot_delay_time <= int(GetTickCount()))
			shot_delay = false;

		if (shot_delay)
			cmd->buttons &= ~IN_ATTACK;

		if (settings.autofire.enabled && GetAsyncKeyState(settings.autofire.hotkey))
			cmd->buttons |= IN_ATTACK;
	}

	current_punch = g_LocalPlayer->m_aimPunchAngle();
	{
		if (cmd->buttons & IN_ATTACK)
			RCS(angles, target, cmd);
	}
	last_punch = current_punch;

	if (!IsSilent() && target)
		Smooth(current, angles, angles);

	Math::FixAngles(angles);
	cmd->viewangles = angles;
	if (!IsSilent())
		g_EngineClient->SetViewAngles(&angles);

	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol() && settings.autopistol)
	{
		const float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		const float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;

		if (next_shot > 0)
			cmd->buttons &= ~IN_ATTACK;
	}

	if (cmd->buttons & IN_ATTACK)
	{
		lastShotTick = GetTickCount();
		shotsFired++;
	}
}