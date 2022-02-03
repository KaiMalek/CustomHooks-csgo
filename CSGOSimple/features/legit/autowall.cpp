#include "autowall.hpp"

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

float Autowall::GetHitgroupDamageMultiplier(int iHitGroup)
{
	switch (iHitGroup) {
	case HITGROUP_GENERIC:
		return 0.5f;
	case HITGROUP_HEAD:
		return 2.0f;
	case HITGROUP_CHEST:
		return 0.5f;
	case HITGROUP_STOMACH:
		return 0.75f;
	case HITGROUP_LEFTARM:
		return 0.5f;
	case HITGROUP_RIGHTARM:
		return 0.5f;
	case HITGROUP_LEFTLEG:
		return 0.375f;
	case HITGROUP_RIGHTLEG:
		return 0.375f;
	case HITGROUP_GEAR:
		return 0.5f;
	default:
		return 1.0f;
	}
}

void Autowall::ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, ITraceFilter* filter, trace_t* tr)
{
	float smallestFraction = tr->fraction;
	constexpr float maxRange = 60.0f;

	Vector delta(vecAbsEnd - vecAbsStart);
	const float delta_length = delta.Length();
	delta.NormalizeInPlace();

	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);

	for (int i = 1; i <= g_GlobalVars->maxClients; ++i) {
		auto ent = C_BasePlayer::GetPlayerByIndex(i);
		if (!ent || ent->IsDormant() || ent->m_lifeState() != LIFE_ALIVE)
			continue;

		if (filter && !filter->ShouldHitEntity(ent, mask))
			continue;

		auto collideble = ent->GetCollideable();
		auto mins = collideble->OBBMins();
		auto maxs = collideble->OBBMaxs();

		auto obb_center = (maxs + mins) * 0.5f;
		auto extend = (obb_center - vecAbsStart);
		auto rangeAlong = delta.Dot(extend);

		float range;
		if (rangeAlong >= 0.0f) {
			if (rangeAlong <= delta_length)
				range = Vector(obb_center - ((delta * rangeAlong) + vecAbsStart)).Length();
			else
				range = -(obb_center - vecAbsEnd).Length();
		}
		else {
			range = -extend.Length();
		}

		if (range >= 0.0f && range <= maxRange) {
			trace_t playerTrace;
			g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace);
			if (playerTrace.fraction < smallestFraction) {
				*tr = playerTrace;
				smallestFraction = playerTrace.fraction;
			}
		}
	}
}

void Autowall::ScaleDamage(int hitgroup, C_BasePlayer* enemy, float weapon_armor_ratio, float& current_damage)
{
	static auto mp_damage_scale_ct_head = g_CVar->FindVar("mp_damage_scale_ct_head");
	static auto mp_damage_scale_t_head = g_CVar->FindVar("mp_damage_scale_t_head");
	static auto mp_damage_scale_ct_body = g_CVar->FindVar("mp_damage_scale_ct_body");
	static auto mp_damage_scale_t_body = g_CVar->FindVar("mp_damage_scale_t_body");

	auto team = enemy->m_iTeamNum();
	auto head_scale = team == 2 ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();
	auto body_scale = team == 2 ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();

	auto armor_heavy = enemy->m_bHasHeavyArmor();
	auto armor_value = static_cast<float>(enemy->m_ArmorValue());

	if (armor_heavy)
		head_scale *= 0.5f;

	// ref: CCSPlayer::TraceAttack
	switch (hitgroup) {
	case HITGROUP_HEAD:
		current_damage = (current_damage * 4.f) * head_scale;
		break;
	case HITGROUP_CHEST:
	case 8:
		current_damage *= body_scale;
		break;
	case HITGROUP_STOMACH:
		current_damage = (current_damage * 1.25f) * body_scale;
		break;
	case HITGROUP_LEFTARM:
	case HITGROUP_RIGHTARM:
		current_damage *= body_scale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage = (current_damage * 0.75f) * body_scale;
		break;
	default:
		break;
	}

	static auto IsArmored = [](C_BasePlayer* player, int hitgroup) {
		auto has_helmet = player->m_bHasHelmet();
		auto armor_value = static_cast<float>(player->m_ArmorValue());

		if (armor_value > 0.f) {
			switch (hitgroup) {
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case 8:
				return true;
			case HITGROUP_HEAD:
				return has_helmet || player->m_bHasHeavyArmor();
			default:
				return player->m_bHasHeavyArmor();
			}
		}

		return false;
	};

	if (IsArmored(enemy, hitgroup)) {
		auto armor_scale = 1.f;
		auto armor_ratio = (weapon_armor_ratio * 0.5f);
		auto armor_bonus_ratio = 0.5f;

		if (armor_heavy) {
			armor_ratio *= 0.2f;
			armor_bonus_ratio = 0.33f;
			armor_scale = 0.25f;
		}

		float new_damage = current_damage * armor_ratio;
		float estiminated_damage = (current_damage - (current_damage * armor_ratio)) * (armor_scale * armor_bonus_ratio);
		if (estiminated_damage > armor_value)
			new_damage = (current_damage - (armor_value / armor_bonus_ratio));

		current_damage = new_damage;
	}
}

bool Autowall::TraceToExit(Vector& end, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace)
{
	float distance = 0.0f;
	while (distance <= 90.0f) {
		distance += 4.0f;

		end = start + dir * distance;
		auto point_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL);
		if (point_contents & MASK_SHOT_HULL && !(point_contents & CONTENTS_HITBOX))
			continue;

		auto new_end = end - (dir * 4.0f);
		Ray_t ray;
		ray.Init(end, new_end);
		g_EngineTrace->TraceRay(ray, MASK_SHOT, 0, exit_trace);
		if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX) {
			ray.Init(end, start);
			CTraceFilter filter;
			filter.pSkip = exit_trace->hit_entity;
			g_EngineTrace->TraceRay(ray, 0x600400B, &filter, exit_trace);
			if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid) {
				end = exit_trace->endpos;
				return true;
			}
			continue;
		}

		if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid) {
			if (exit_trace->hit_entity) {
				if (enter_trace->hit_entity && enter_trace->hit_entity == g_EntityList->GetClientEntity(0))
					return true;
			}
			continue;
		}

		if (exit_trace->surface.flags >> 7 & 1 && !(enter_trace->surface.flags >> 7 & 1))
			continue;

		if (exit_trace->plane.normal.Dot(dir) <= 1.0f) {
			auto fraction = exit_trace->fraction * 4.0f;
			end = end - (dir * fraction);
			return true;
		}
	}
	return false;
}

bool Autowall::HandleBulletPenetration(CCSWeaponInfo* weaponInfo, FireBulletData& data)
{
	surfacedata_t* enter_surface_data = g_PhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);

	if (data.trace_length > 3000.f || enter_surf_penetration_mod < 0.1f)
		data.penetrate_count = 0;

	if (data.penetrate_count <= 0)
		return false;

	Vector dummy;
	trace_t trace_exit;
	if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
		return false;

	surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	const int exit_material = exit_surface_data->game.material;
	const float exit_surf_penetration_mod = *(float*)((uint8_t*)exit_surface_data + 76);
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier;
	if ((data.enter_trace.contents & CONTENTS_GRATE) != 0 || enter_material == 89 || enter_material == 71) {
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;

	if (enter_material == exit_material) {
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}

	const float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	const float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->flPenetration) * 1.25f);
	float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();

	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;

	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.0f)
		return false;

	data.src = trace_exit.endpos;
	data.penetrate_count--;
	return true;
}

void TraceLine(Vector vecAbsStart, Vector vecAbsEnd, unsigned int mask, C_BasePlayer* ignore, trace_t* ptr)
{
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;
	g_EngineTrace->TraceRay(ray, mask, &filter, ptr);
}

bool Autowall::SimulateFireBullet(C_BaseCombatWeapon* pWeapon, FireBulletData& data)
{
	CCSWeaponInfo* weaponInfo = pWeapon->GetCSWeaponData();
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	data.current_damage = (float)weaponInfo->iDamage;
	while (data.penetrate_count > 0 && data.current_damage >= 1.0f) {
		data.trace_length_remaining = weaponInfo->flRange - data.trace_length;
		Vector end = data.src + data.direction * data.trace_length_remaining;
		TraceLine(data.src, end, MASK_SHOT, g_LocalPlayer, &data.enter_trace);

		CTraceFilter local_filter;
		local_filter.pSkip = g_LocalPlayer;
		ClipTraceToPlayers(data.src, end + data.direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &local_filter, &data.enter_trace);
		if (data.enter_trace.fraction == 1.0f)
			break;

		if (data.enter_trace.hitgroup <= HITGROUP_RIGHTLEG && data.enter_trace.hitgroup > HITGROUP_GENERIC) {
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);

			C_BasePlayer* player = (C_BasePlayer*)data.enter_trace.hit_entity;
			Autowall::ScaleDamage(data.enter_trace.hitgroup, player, weaponInfo->flArmorRatio, data.current_damage);
			return true;
		}

		if (!HandleBulletPenetration(weaponInfo, data))
			break;
	}
	return false;
}

float Autowall::GetDamage(const Vector& point)
{
	float damage = 0.f;
	Vector dst = point;
	FireBulletData data;
	data.src = g_LocalPlayer->GetEyePos();
	data.filter.pSkip = g_LocalPlayer;
	data.direction = dst - data.src;
	data.direction.NormalizeInPlace();
	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon());
	if (!activeWeapon)
		return -1.0f;
	if (SimulateFireBullet(activeWeapon, data))
		damage = data.current_damage;
	return damage;
}
#include "../../helpers/math.hpp"
#include "../../options.hpp"

#define HITGROUP_GENERIC   0
#define HITGROUP_HEAD      1
#define HITGROUP_CHEST     2
#define HITGROUP_STOMACH   3
#define HITGROUP_LEFTARM   4
#define HITGROUP_RIGHTARM  5
#define HITGROUP_LEFTLEG   6
#define HITGROUP_RIGHTLEG  7
#define HITGROUP_GEAR      10

inline bool CGameTrace::DidHitWorld() const
{
	return hit_entity->EntIndex() == 0;
}
inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return hit_entity != NULL && !DidHitWorld();
}

#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD        1
#define HITGROUP_CHEST        2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10
#define DAMAGE_NO		0
#define DAMAGE_EVENTS_ONLY	1
#define DAMAGE_YES		2
#define DAMAGE_AIM		3
#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

void ScaleDamage_1(int hitgroup, C_BasePlayer* enemy, float weapon_armor_ratio, float& current_damage)
{
	int armor = enemy->m_ArmorValue();
	float ratio;

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		current_damage *= 4.f;
		break;
	case HITGROUP_STOMACH:
		current_damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage *= 0.75f;
		break;
	}

	if (armor > 0)
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			if (enemy->m_bHasHelmet())
			{
				ratio = (weapon_armor_ratio * 0.5) * current_damage;
				if (((current_damage - ratio) * 0.5) > armor)
				{
					ratio = current_damage - (armor * 2.0);
				}
				current_damage = ratio;
			}
			break;
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			ratio = (weapon_armor_ratio * 0.5) * current_damage;
			if (((current_damage - ratio) * 0.5) > armor)
			{
				ratio = current_damage - (armor * 2.0);
			}
			current_damage = ratio;
			break;
		}
	}
}