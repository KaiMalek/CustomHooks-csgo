#include "resolver.h"
#include <random>



static std::random_device rd;
static std::mt19937 rng(rd());


float flAngleMod(float flAngle)
{
	return ((360.0f / 65536.0f) * ((int32_t)(flAngle * (65536.0f / 360.0f)) & 65535));
}

float ApproachAngle(float flTarget, float flValue, float flSpeed)
{
	flTarget = flAngleMod(flTarget);
	flValue = flAngleMod(flValue);

	float delta = flTarget - flValue;


	if (flSpeed < 0)
		flSpeed = -flSpeed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > flSpeed)
		flValue += flSpeed;
	else if (delta < -flSpeed)
		flValue -= flSpeed;
	else
		flValue = flTarget;

	return flValue;

}

static auto GetSmoothedVelocity = [](float min_delta, Vector a, Vector b)
{
	Vector delta = a - b;
	float delta_length = delta.Length();

	if (delta_length <= min_delta)
	{
		Vector result;
		if (-min_delta <= delta_length)
		{
			return a;
		}
		else
		{
			float iradius = 1.0f / (delta_length + FLT_EPSILON);
			return b - ((delta * iradius) * min_delta);
		}
	}
	else
	{
		float iradius = 1.0f / (delta_length + FLT_EPSILON);
		return b + ((delta * iradius) * min_delta);
	}
};

void x_resolver::resolver(C_BasePlayer* player)
{
	if (!player || !player->GetPlayerAnimState())
		return;

	auto animState = player->GetPlayerAnimState();

	if (!animState)
		return;

	Vector velocity = player->m_vecVelocity();
	float spd = velocity.LengthSqr();
	if (spd > std::powf(1.2f * 260.0f, 2.f))
	{
		Vector velocity_normalized = velocity.Normalized();
		velocity = velocity_normalized * (1.2f * 260.0f);
	}

	float Resolveyaw = animState->m_flGoalFeetYaw;

	auto delta_time = fmaxf(g_GlobalVars->curtime - animState->m_flLastClientSideAnimationUpdateTime, 0.f);

	float deltatime = fabs(delta_time);
	float stop_to_full_running_fraction = 0.f;
	bool is_standing = true;
	float v25 = std::clamp(player->m_flDuckAmount() + animState->m_fLandingDuckAdditiveSomething, 0.0f, 1.0f);
	float v26 = animState->m_fDuckAmount;
	float v27 = deltatime * 6.0f;
	float v28;

	if ((v25 - v26) <= v27)
	{
		if (-v27 <= (v25 - v26))
			v28 = v25;
		else
		 v28 = v26 + v27;
		
	}
	else
	{
		v28 = v27 + v27;
	}

	float flDuckAmount = std::clamp(v28, 0.0f, 1.0f);

	Vector animationVelocity = velocity;
	float speed = std::fminf(animationVelocity.Length(), 260.0f);

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)
		return;

	auto wpndata = weapon->GetCSWeaponData();

	if (!wpndata)
		return;

	float flMaxMovementSpeed = 260.0f;
	if (weapon) {
		flMaxMovementSpeed = std::fmaxf(wpndata->flMaxPlayerSpeed, 0.001f);
	}

	float flRunningSpeed = speed / (flMaxMovementSpeed * 0.520f);
	float flDuckingSpeed_2 = speed / (flMaxMovementSpeed * 0.340f);

	flRunningSpeed = std::clamp(flRunningSpeed, 0.0f, 1.0f);

	float flYawModifier = (((stop_to_full_running_fraction * -0.3f) - 0.2f) * flRunningSpeed) + 1.0f;
	if (flDuckAmount > 0.0f) {
		float flDuckingSpeed = std::clamp(flDuckingSpeed_2, 0.0f, 1.0f);
		flYawModifier += (flDuckAmount * flDuckingSpeed) * (0.5f - flYawModifier);
	}

	float flMaxBodyYaw = *reinterpret_cast<float*>(&animState->pad10[512]);
	float flMinBodyYaw = *reinterpret_cast<float*>(&animState->pad10[516]);



	float flEyeYaw = player->m_angEyeAngles().yaw;

	float flEyeDiff = std::remainderf(flEyeYaw - Resolveyaw, 360.f);

	if (flEyeDiff <= flMaxBodyYaw) {
		if (flMinBodyYaw > flEyeDiff)
			Resolveyaw = fabs(flMinBodyYaw) + flEyeYaw;
	}
	else {
		Resolveyaw = flEyeYaw - fabs(flMaxBodyYaw);
	}

	if (speed > 0.1f || fabs(velocity.z) > 100.0f) {
		Resolveyaw = ApproachAngle(
			flEyeYaw,
			Resolveyaw,
			((stop_to_full_running_fraction * 20.0f) + 30.0f)
			* deltatime);
	}
	else {
		Resolveyaw = ApproachAngle(
			player->m_flLowerBodyYawTarget(),
			Resolveyaw,
			deltatime * 100.0f);
	}

	if (stop_to_full_running_fraction > 0.0 && stop_to_full_running_fraction < 1.0)
	{
		const auto interval = g_GlobalVars->interval_per_tick * 2.f;

		if (is_standing)
			stop_to_full_running_fraction = stop_to_full_running_fraction - interval;
		else
			stop_to_full_running_fraction = interval + stop_to_full_running_fraction;

		stop_to_full_running_fraction = std::clamp(stop_to_full_running_fraction, 0.f, 1.f);
	}

	if (speed > 135.2f && is_standing)
	{
		stop_to_full_running_fraction = fmaxf(stop_to_full_running_fraction, .0099999998f);
		is_standing = false;
	}

	if (speed < 135.2f && !is_standing)
	{
		stop_to_full_running_fraction = fminf(stop_to_full_running_fraction, .99000001f);
		is_standing = true;
	}

	auto& log = player_log::Get().get_log(player->EntIndex());

	switch (log.m_nShots % 5)
	{
	case 0:
		break;
	case 1:
		animState->m_flGoalFeetYaw += 58.0f;
		break;
	case 2:
		animState->m_flGoalFeetYaw -= 58.0f;
		break;
	case 3:
		animState->m_flGoalFeetYaw += 29.0f;
		break;
	case 4:
		animState->m_flGoalFeetYaw -= 29.0f;
		break;
	default:
		break;
	}
}






