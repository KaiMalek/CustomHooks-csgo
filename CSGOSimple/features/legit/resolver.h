#pragma once
#include "../../valve_sdk/math/Vector.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "log.h"


namespace resolver
{
	struct shot
	{
		Vector start{}, end{};
		uint32_t hitgroup{};
		int hitbox{};
		float time{}, damage{};
		bool confirmed{}, impacted{}, skip{}, manual{};

		struct server_info_t
		{
			std::vector<Vector> inpacts{};
			uint32_t hitgroup{}, damage{}, index{};
		}server_info;
	};
}

class x_resolver
{
public:
	static void resolver(C_BasePlayer* player);
	static void register_shot(resolver::shot&& s);
	static void on_player_hurt(IGameEvent* event);
	static void on_bullet_impact(IGameEvent* event);
	static void on_weapon_fire(IGameEvent* event);
	static void on_render_start();
	static void Listener();
private:
	static void resolve_shot(resolver::shot& shot);

	//inline static std::deque<resolver::shot> shots = {};
};
