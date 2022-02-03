#include <ShlObj.h>
#include <filesystem>
#include "options.hpp"
#include "valve_sdk/misc/Enums.hpp"
#include "valve_sdk/sdk.hpp"


const std::map<int, const char*> config_names = {
	{WEAPON_CZ75A, "CZ75 Auto"},
	{WEAPON_DEAGLE, "Desert Eagle"},
	{WEAPON_ELITE, "Dual Berettas"},
	{WEAPON_FIVESEVEN, "Five-SeveN"},
	{WEAPON_HKP2000, "P2000"},
	{WEAPON_P250, "P250"},
	{WEAPON_USP_SILENCER, "USP-S"},
	{WEAPON_TEC9, "Tec-9"},
	{WEAPON_REVOLVER, "R8 Revolver"},
	{WEAPON_GLOCK, "Glock-18"},

	{WEAPON_MAG7, "MAG-7"},
	{WEAPON_NOVA, "Nova"},
	{WEAPON_SAWEDOFF, "Sawed-Off"},
	{WEAPON_XM1014, "XM1014"},

	{WEAPON_UMP45, "UMP-45"},
	{WEAPON_P90, "P90"},
	{WEAPON_BIZON, "PP-Bizon"},
	{WEAPON_MAC10, "MAC-10"},
	{WEAPON_MP7, "MP7"},
	{WEAPON_MP9, "MP9"},
	{WEAPON_MP5, "MP5-SD"},

	{WEAPON_M249, "M249"},
	{WEAPON_NEGEV, "Negev"},

	{WEAPON_AK47, "AK-47"},
	{WEAPON_AUG, "AUG"},
	{WEAPON_GALILAR, "Galil AR"},
	{WEAPON_M4A1_SILENCER, "M4A1-S"},
	{WEAPON_M4A1, "M4A4"},
	{WEAPON_SG556, "SG 553"},
	{WEAPON_FAMAS, "FAMAS"},

	{WEAPON_AWP, "AWP"},
	{WEAPON_G3SG1, "G3SG1"},
	{WEAPON_SCAR20, "SCAR-20"},
	{WEAPON_SSG08, "SSG 08"},

	{WEAPON_KNIFE, "Knife"},
	{WEAPON_KNIFE_T, "TKnife"},

	{GLOVE_T_SIDE, "Glove"},
};

const std::map<int, const char*> k_weapon_names = {
	{WEAPON_KNIFE, "knife ct"},
	{WEAPON_KNIFE_T, "knife t"},
	{GLOVE_CT_SIDE, "glove ct"},
	{GLOVE_T_SIDE, "glove t"},
	{61, "usp"},
	{32, "p2000"},
	{4, "glock-18"},
	{2, "dual berettas"},
	{36, "p250"},
	{3, "five-seven"},
	{30, "tec-9"},
	{63, "cz75 auto"},
	{64, "r8 revolver"},
	{1, "deagle"},

	{34, "mp9"},
	{17, "mac-10"},
	{23, "mp5-sd"},
	{33, "mp7"},
	{24, "ump-45"},
	{19, "p90"},
	{26, "pp-bizon"},

	{7, "ak-47"},
	{60, "m4a1-s"},
	{16, "m4a4"},
	{8, "aug"},
	{39, "sg553"},
	{10, "famas"},
	{13, "galil"},

	{40, "ssg08"},
	{38, "scar-20"},
	{9, "awp"},
	{11, "g3sg1"},

	{14, "m249"},
	{28, "negev"},

	{27, "mag-7"},
	{35, "nova"},
	{29, "sawed-off"},
	{25, "xm1014"},
};

void Options::SetupValue(int& value, std::string category, std::string name)
{
	ints.push_back(new ConfigValue<int>(std::move(category), std::move(name), &value));
}
/*void Options::SetupValue(char value, std::string category, std::string name)
{
	chars.push_back(new ConfigValue<char>(std::move(category), std::move(name), &value));
}*/

void Options::SetupValue(bool& value, std::string category, std::string name)
{
	bools.push_back(new ConfigValue<bool>(std::move(category), std::move(name), &value));
}

void Options::SetupValue(float& value, std::string category, std::string name)
{
	floats.push_back(new ConfigValue<float>(std::move(category), std::move(name), &value));
}

void Options::SetupVisuals()
{
	SetupValue(g_Options.esp_enabled, "ESP", "ESP Enabled");
	SetupValue(g_Options.esp_enemies_only, "ESP", "ESP Enemies only");
	SetupValue(g_Options.esp_player_boxes, "ESP", "ESP Player boxes");
	SetupValue(g_Options.esp_player_names, "ESP", "ESP Player names");
	SetupValue(g_Options.esp_player_health, "ESP", "ESP Player health");
	SetupValue(g_Options.esp_player_armour, "ESP", "ESP Player armour");
	SetupValue(g_Options.esp_player_weapons, "ESP", "ESP Player weapons");
	SetupValue(g_Options.esp_player_snaplines, "ESP", "ESP Player snaplines");
	SetupValue(g_Options.rendercrosshair, "ESPp", "ESP Crosshairrr");
	SetupValue(g_Options.dotcrosshair, "ESP", "ESP Crosshair");
	SetupValue(g_Options.esp_dropped_weapons, "ESP", "ESP Dropped weapons");
	SetupValue(g_Options.esp_defuse_kit, "ESP", "ESP Defuse kit");
	SetupValue(g_Options.esp_planted_c4, "ESP", "ESP Planted C4");
	SetupValue(g_Options.esp_items, "ESP", "ESP Items");
	SetupValue(g_Options.glow_enabled, "ESP", "Glow Enabled");
	SetupValue(g_Options.glow_enemies_only, "ESP", "Glow Enemies only");
	SetupValue(g_Options.glow_players, "ESP", "Glow Players");
	SetupValue(g_Options.glow_chickens, "ESP", "Glow Chickens");
	SetupValue(g_Options.glow_c4_carrier, "ESP", "Glow C4 carrier");
	SetupValue(g_Options.glow_planted_c4, "ESP", "Glow Planted C4");
	SetupValue(g_Options.glow_defuse_kits, "ESP", "Glow Defuse kits");
	SetupValue(g_Options.glow_weapons, "ESP", "Glow Weapons");

	SetupValue(g_Options.chams_player_enabled, "ESP", "Chams Player enabled");
	SetupValue(g_Options.chams_player_enemies_only, "ESP", "Chams Player enemies only");
	SetupValue(g_Options.chams_player_wireframe, "ESP", "Chams Player wireframe");
	SetupValue(g_Options.chams_player_flat, "ESP", "Chams Player flat");
	SetupValue(g_Options.chams_player_ignorez, "ESP", "Chams Player ignorez");
	SetupValue(g_Options.chams_player_glass, "ESP", "Chams Player glass");
	
	SetupValue(g_Options.chams_arms_enabled, "ESP", "Chams Arms enabled");
	SetupValue(g_Options.chams_arms_wireframe, "ESP", "Chams Arms wireframe");
	SetupValue(g_Options.chams_arms_flat, "ESP", "Chams Arms flat");
	SetupValue(g_Options.chams_arms_ignorez, "ESP", "Chams Arms ignorez");
	SetupValue(g_Options.chams_arms_glass, "ESP", "Chams Arms glass");
	SetupValue(g_Options.no_smoke, "ESP", "no smoke");
	SetupValue(g_Options.no_flash, "ESP", "no smoke");
	SetupValue(g_Options.Enable_Region_Changer, "ESP", "region chnager enable");
	SetupValue(g_Options.esp_skeleton, "ESP", "ESP Skeleton");
}
void Options::SetupWeapons()
{
	for (auto& [val, key] : config_names)
	{
		auto aimbot = &g_Options.weapons[val].legit;

		if (val != WEAPON_KNIFE && val != WEAPON_KNIFE_T && val != GLOVE_T_SIDE)
		{
			SetupValue(aimbot->enabled, key, "Enabled");
			SetupValue(aimbot->silent2, key, "Silent");

			SetupValue(aimbot->flash_check, key, "Flash Check");
			SetupValue(aimbot->smoke_check, key, "Smoke Check");
			SetupValue(aimbot->autopistol, key, "AutoPistol");

			SetupValue(aimbot->fov, key, "Fov");
			SetupValue(aimbot->silent_fov, key, "pSilent Fov");
			SetupValue(aimbot->smooth, key, "Smooth");

			SetupValue(aimbot->kill_delay, key, "Kill Delay");
			SetupValue(aimbot->shot_delay, key, "Shot Delay");

			SetupValue(aimbot->hitboxes.head, key, "Head Hitbox");
			SetupValue(aimbot->hitboxes.chest, key, "Chest Hitbox");
			SetupValue(aimbot->hitboxes.hands, key, "Hands Hitbox");
			SetupValue(aimbot->hitboxes.legs, key, "Legs Hitbox");

			SetupValue(aimbot->rcs.enabled, key, "RCS Enabled");
			SetupValue(aimbot->rcs.start, key, "RCS Start");
			SetupValue(aimbot->rcs.type, key, "RCS Type");
			SetupValue(aimbot->rcs.x, key, "RCS X");
			SetupValue(aimbot->rcs.y, key, "RCS Y");

			SetupValue(aimbot->backtrack.ticks, key, "Backtrack Ticks");

			SetupValue(aimbot->autofire.enabled, key, "AutoFire Enabled");
			SetupValue(aimbot->autofire.hotkey, key, "AutoFire Hotkey");
		}
	}
	for (auto& [key, val] : k_weapon_names) {
		auto& option = g_Options.changers.skin.m_items[key];
		SetupValue(option.definition_vector_index, val, "d_vec_index");
		SetupValue(option.definition_index, val, "d_index");
		SetupValue(option.paint_kit_vector_index, val, "pk_vec_index");
		SetupValue(option.paint_kit_index, val, "pk_index");
		SetupValue(option.definition_override_index, val, "do_index");
		SetupValue(option.definition_override_vector_index, val, "do_vec_index");
		//SetupValue(option.seed, val, "seed");
		//SetupValue(option.enable_stat_track, val, "stattrack");
		//SetupValue(option.stat_trak, val, "stat_trak_val");
		//SetupValue(option.wear, val, "wear");
	}

}
void Options::SetupMisc()
{

	// ---------------------------bot stuff above -------------------//

	SetupValue(g_Options.misc_bhop, "Misc", "Misc Bunny hop");
	SetupValue(g_Options.misc_no_hands, "Misc", "Misc No hands");
	SetupValue(g_Options.misc_showranks, "Misc", "Misc Showranks");
	SetupValue(g_Options.m_watermark, "Misc", "Misc watermark");
	SetupValue(g_Options.thirdperson.misc_thirdperson, "Misc", "Misc Thirdperson");
	SetupValue(g_Options.thirdperson.misc_thirdperson_dist, "Misc", "Misc Thirdperson Dist");
	SetupValue(g_Options.thirdperson.hotkey, "Misc", "Misc Thirdperson Hotkey");
	SetupValue(g_Options.viewmodel_fov, "Misc", "Misc Viewmodel fov");
	SetupValue(g_Options.spectator_list, "Misc", "Spectator List");
	SetupValue(g_Options.Preview, "Misc", "Preview");
	SetupValue(g_Options.region_changer, "Misc", "Preview");
	SetupValue(g_Options.player_list, "Misc", "Player List");
	SetupValue(g_Options.mat_ambient_light_r, "Misc", "Mat ambient light r");
	SetupValue(g_Options.mat_ambient_light_g, "Misc", "Mat ambient light g");
	SetupValue(g_Options.mat_ambient_light_b, "Misc", "Mat ambient light b");
	SetupValue(g_Options.playerModelCT, "Misc", "playerModelCT");
	SetupValue(g_Options.playerModelT, "Misc", "playerModelT");
	SetupValue(g_Options.hitmarker, "Misc", "Hitmarker");
	SetupValue(g_Options.Presolver, "Misc", "Presolver");
	SetupValue(g_Options.leftknife, "Misc", "Left hand knife");
	SetupValue(g_Options.VisibleCrosshair, "Misc", "Left hand knife");
	SetupValue(g_Options.nightmode, "Misc", "nightmode");
	SetupValue(g_Options.misc_backtrack, "Misc", "backtrack enable");

	SetupValue(g_Options.custom_viewmodel, "Misc", "Custom viewmodel");
	SetupValue(g_Options.viewmodel_fov, "Misc", "Viewmodel FOV");
	SetupValue(g_Options.viewmodel_offset_x, "Misc", "Offset X");
	SetupValue(g_Options.viewmodel_offset_y, "Misc", "Offset Y");
	SetupValue(g_Options.viewmodel_offset_z, "Misc", "Offset Z");

	SetupValue(g_Options.skyboxchanger, "Misc", "Skybox changer");
	SetupValue(g_Options.skybox, "Misc", "Skybox value");

	SetupValue(g_Options.blockbot, "Misc", "Blockbot");
	SetupValue(g_Options.blockbotkey, "Misc", "blobkbotkey");

	SetupValue(g_Options.aspectratio, "Misc", "Misc aspect ratio");
	SetupValue(g_Options.aspectvalue, "Misc", "Misc aspect value");

	SetupValue(g_Options.spotifyapi, "Misc", "Misc spotify api");
}

void Options::SetupColor(Color& value, const std::string& name)
{
	SetupValue(value._CColor[0], "Colors", name + "_r");
	SetupValue(value._CColor[1], "Colors", name + "_g");
	SetupValue(value._CColor[2], "Colors", name + "_b");
	SetupValue(value._CColor[3], "Colors", name + "_a");
}

void Options::Initialize()
{
	CHAR my_documents[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents);
	folder = my_documents + std::string("\\customhooks\\");
	CreateDirectoryA(folder.c_str(), nullptr);
	SetupVisuals();
	SetupMisc();
	SetupWeapons();
	//SetupColor();
}

void Options::SaveCFG(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	for (auto value : ints)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());

	for (auto value : floats)
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
}

void Options::LoadCFG(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}

	for (auto value : floats)
	{
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = float(atof(value_l));
	}
	g_ClientState->ForceFullUpdate();
}

void Options::DeleteCFG(const std::string& szIniFile)
{
	std::string file = folder + szIniFile;

	CreateDirectoryA(folder.c_str(), nullptr);

	if (!std::filesystem::exists(file))
		return;

	remove(file.c_str());
}