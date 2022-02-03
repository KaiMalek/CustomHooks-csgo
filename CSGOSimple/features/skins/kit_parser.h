#pragma once
#include <vector>
#include <map>
#include <xstring>
struct paint_kit
{
	int id;
	int rarity;
	std::string name;
	std::string name_short;
	std::vector<std::string> weaponName;
	auto operator < (const paint_kit& other) const -> bool
	{
		return name < other.name;
	}
};
extern bool skins_parsed;
extern std::vector<paint_kit> k_skins;
extern std::vector<paint_kit> k_gloves;
extern auto initialize_kits() -> void;