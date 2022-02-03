#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}
template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

struct legitbot_s
{
	bool enabled = false;
	int silent2 = false;

	bool flash_check = false;
	bool smoke_check = false;
	bool autopistol = false;

	float fov = 0.f;
	float silent_fov = 0.f;
	float smooth = 1.f;

	int shot_delay = 0;
	int kill_delay = 0;

	struct
	{
		bool head = false;
		bool chest = false;
		bool hands = false;
		bool legs = false;
	} hitboxes;

	struct
	{
		bool enabled = false;
		int start = 1;
		int type = 0;
		int x = 100;
		int y = 100;
	} rcs;

	struct
	{
		int ticks = 12;
	} backtrack;

	struct
	{
		bool enabled = false;
		int hotkey = 0;
	} autofire;
};

struct weapons
{
	legitbot_s legit;
};

struct statrack_setting
{
	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};
struct item_setting
{
	char name[32] = "Default";
	//bool enabled = false;
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};
class Options
{
public:
	std::map<short, weapons> weapons;
	struct
	{
		/*struct
		{
			std::map<int, profilechanger_settings> profile_items = { };
		}profile;*/
		struct
		{
			bool skin_preview = false;
			bool show_cur = true;

			std::map<int, statrack_setting> statrack_items = { };
			std::map<int, item_setting> m_items = { };
			std::map<std::string, std::string> m_icon_overrides = { };
		}skin;
	}changers;
		// 
		// ESP
		// 
		bool esp_enabled= true;
		bool esp_enemies_only= true;
		bool esp_player_boxes= false;
		bool esp_player_names= false;
		bool esp_player_health= false;
		bool esp_player_armour= false;
		bool esp_player_weapons= false;
	    bool esp_player_snaplines= false;
		bool esp_skeleton = false;
		bool rendercrosshair = false;
		bool dotcrosshair = false;
		bool esp_dropped_weapons= false;
		bool esp_defuse_kit= false;
		bool esp_planted_c4= false;
		bool esp_items= false;

		// 
		// GLOW
		// 
		bool glow_enabled= false;
		bool glow_enemies_only= true;
		bool glow_players= false;
		bool glow_chickens= false;
		bool glow_c4_carrier= false;
		bool glow_planted_c4= false;
		bool glow_defuse_kits= false;
		bool glow_weapons= false;

		//
		// CHAMS
		//
		bool chams_player_enabled= false;
		bool chams_player_enemies_only= true;
		bool chams_player_wireframe= false;
		bool chams_player_flat= false;
		bool chams_player_ignorez= false;
		bool chams_player_glass= false;
		bool chams_arms_enabled= false;
		bool chams_arms_wireframe= false;
		bool chams_arms_flat= false;
		bool chams_arms_ignorez= false;
		bool chams_arms_glass= false;

		//
		// Legit bot stuff
		//
		bool misc_backtrack = false;
		//
		// MISC
		//
		bool misc_bhop= false;
		bool misc_no_hands= false;
		bool misc_showranks= false;
		bool spectator_list = false;
		bool nightmode = false;
		bool Preview = false;
		bool VisibleCrosshair = false;
		bool player_list = false;
		bool leftknife = false;
		bool skeleton = false;
		bool Enable_Region_Changer = false;
		bool region_changer = false;
		bool no_smoke = false;
		bool Presolver = false;
		bool no_flash = false;
		bool m_watermark= false;
		bool aspectratio;
		float aspectvalue = 0.f;
		struct
		{
			bool misc_thirdperson = false;
			float misc_thirdperson_dist = 50.f;
			int hotkey = 0;
		} thirdperson;
		float mat_ambient_light_r= 0.0f;
		float mat_ambient_light_g= 0.0f;
		float mat_ambient_light_b= 0.0f;
		int playerModelT{ 0 };
		int playerModelCT{ 0 };
		bool hitmarker = false;

		bool custom_viewmodel = false;
		int viewmodel_fov{ 68 };
		int viewmodel_offset_x{ 2 };
		int viewmodel_offset_y{ 2 };
		int viewmodel_offset_z{ -2 };

		bool skyboxchanger = false;
		int skybox;

		bool blockbot = false;
		int blockbotkey{ 0 };

		bool spotifyapi = false;
		// 
		// COLORS
		// 
		Color color_esp_ally_visible = { 255, 255, 255 };
		Color color_esp_enemy_visible = { 255, 255, 255 };
		Color color_esp_ally_occluded = { 255, 255, 255 };
		Color color_esp_enemy_occluded = { 255, 255, 255 };
		Color color_esp_crosshair = { 255, 255, 255 };
		Color color_esp_weapons = { 255, 255, 255 };
		Color color_esp_defuse = { 255, 255, 255 };
		Color color_esp_c4 = { 255, 255, 255 };
		Color color_esp_item = { 255, 255, 255 };

		Color color_glow_ally = { 255, 255, 255 };
		Color color_glow_enemy = { 255, 255, 255 };
		Color color_glow_chickens = { 255, 255, 255 };
		Color color_glow_c4_carrier = { 255, 255, 255 };
		Color color_glow_planted_c4 = { 255, 255, 255 };
		Color color_glow_defuse = { 255, 255, 255 };
		Color color_glow_weapons = { 255, 255, 255 };

		Color color_chams_player_ally_visible = { 255, 255, 255 };
		Color color_chams_player_ally_occluded = { 255, 255, 255 };
		Color color_chams_player_enemy_visible = { 255, 255, 255 };
		Color color_chams_player_enemy_occluded = { 255, 255, 255 };
		Color color_chams_arms_visible = { 255, 255, 255 };
		Color color_chams_arms_occluded = { 255, 255, 255 };
		Color color_esp_skeleton = { 255, 255, 255 };
		Color color_watermark = { 255, 255, 255 }; // no menu config cuz its useless

protected:
	//std::vector<ConfigValue<char>*> chars;
	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<float>*> floats;
private:
	//	void SetupValue(char value, std::string category, std::string name);
	void SetupValue(int& value, std::string category, std::string name);
	void SetupValue(bool& value, std::string category, std::string name);
	void SetupValue(float& value, std::string category, std::string name);
	void SetupColor(Color& value, const std::string& name);
	void SetupWeapons();
	void SetupVisuals();
	void SetupMisc();
	void SetupColors();
public:
	void Initialize();
	void LoadCFG(const std::string& szIniFile);
	void SaveCFG(const std::string& szIniFile);
	void DeleteCFG(const std::string& szIniFile);

	std::string folder;
};

inline Options g_Options;
inline bool   g_Unload;

