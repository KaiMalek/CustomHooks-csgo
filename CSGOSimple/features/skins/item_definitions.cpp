#include "item_definitions.h"

const std::map<size_t, weapon_info> k_weapon_info =
{
	{WEAPON_KNIFE,{"models/weapons/v_knife_default_ct.mdl", "knife"}},
	{WEAPON_KNIFE_T,{"models/weapons/v_knife_default_t.mdl", "knife_t"}},
	{WEAPON_KNIFE_BAYONET, {"models/weapons/v_knife_bayonet.mdl", "bayonet"}},
	{WEAPON_KNIFE_CSS,{"models/weapons/v_knife_css.mdl", "knife_css"}},

	{WEAPON_KNIFE_SKELETON,{"models/weapons/v_knife_skeleton.mdl", "knife_skeleton"}},
	{WEAPON_KNIFE_OUTDOOR,{"models/weapons/v_knife_outdoor.mdl", "knife_outdoor"}},
	{WEAPON_KNIFE_CANIS,{"models/weapons/v_knife_canis.mdl", "knife_canis"}},
	{WEAPON_KNIFE_CORD,{"models/weapons/v_knife_cord.mdl", "knife_cord"}},


	{WEAPON_KNIFE_FLIP, {"models/weapons/v_knife_flip.mdl", "knife_flip"}},
	{WEAPON_KNIFE_GUT, {"models/weapons/v_knife_gut.mdl", "knife_gut"}},
	{WEAPON_KNIFE_KARAMBIT, {"models/weapons/v_knife_karam.mdl", "knife_karambit"}},
	{WEAPON_KNIFE_M9_BAYONET, {"models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet"}},
	{WEAPON_KNIFE_TACTICAL, {"models/weapons/v_knife_tactical.mdl", "knife_tactical"}},
	{WEAPON_KNIFE_FALCHION, {"models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion"}},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, {"models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie"}},
	{WEAPON_KNIFE_BUTTERFLY, {"models/weapons/v_knife_butterfly.mdl", "knife_butterfly"}},
	{WEAPON_KNIFE_PUSH, {"models/weapons/v_knife_push.mdl", "knife_push"}},
	{WEAPON_KNIFE_URSUS,{"models/weapons/v_knife_ursus.mdl", "knife_ursus"}},
	{WEAPON_KNIFE_GYPSY_JACKKNIFE,{"models/weapons/v_knife_gypsy_jackknife.mdl", "knife_gypsy_jackknife"}},
	{WEAPON_KNIFE_STILETTO,{"models/weapons/v_knife_stiletto.mdl", "knife_stiletto"}},
	{WEAPON_KNIFE_WIDOWMAKER,{"models/weapons/v_knife_widowmaker.mdl", "knife_widowmaker"}},
	{GLOVE_STUDDED_BLOODHOUND,{"models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"}},
	{GLOVE_T_SIDE,{"models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl"}},
	{GLOVE_CT_SIDE,{"models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl"}},
	{GLOVE_SPORTY,{"models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"}},
	{GLOVE_SLICK,{"models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"}},
	{GLOVE_LEATHER_WRAP,{"models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"}},
	{GLOVE_MOTORCYCLE,{"models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"}},
	{GLOVE_SPECIALIST,{"models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"}},
	{GLOVE_HYDRA,{"models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl"}}
};

const std::vector<weapon_name> k_knife_names =
{
	{0, "Default"},
	{WEAPON_KNIFE_BAYONET, "Bayonet"},
	{WEAPON_KNIFE_CSS, "Classic"},
	{WEAPON_KNIFE_CORD, "Cord"},
	{WEAPON_KNIFE_CANIS, "Canis"},
	{WEAPON_KNIFE_OUTDOOR, "Outdoor"},
	{WEAPON_KNIFE_SKELETON, "Skeleton"},
	{WEAPON_KNIFE_FLIP, "Flip"},
	{WEAPON_KNIFE_GUT, "Gut"},
	{WEAPON_KNIFE_KARAMBIT, "Karambit"},
	{WEAPON_KNIFE_M9_BAYONET, "M9 Bayonet"},
	{WEAPON_KNIFE_TACTICAL, "Huntsman"},
	{WEAPON_KNIFE_FALCHION, "Falchion"},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, "Bowie"},
	{WEAPON_KNIFE_BUTTERFLY, "Butterfly"},
	{WEAPON_KNIFE_PUSH, "Shadow Daggers"},
	{WEAPON_KNIFE_URSUS, "Ursus"},
	{WEAPON_KNIFE_GYPSY_JACKKNIFE, "Navaja"},
	{WEAPON_KNIFE_STILETTO, "Stiletto"},
	{WEAPON_KNIFE_WIDOWMAKER, "Talon"}
};

const std::vector<weapon_name> k_glove_names =
{
	{0, "Default"},
	{GLOVE_T_SIDE, "Default T"},
	{GLOVE_CT_SIDE, "Default CT"},
	{GLOVE_STUDDED_BLOODHOUND, "Bloodhound"},
	{GLOVE_SPORTY, "Sporty"},
	{GLOVE_SLICK, "Slick"},
	{GLOVE_LEATHER_WRAP, "Handwrap"},
	{GLOVE_MOTORCYCLE, "Motorcycle"},
	{GLOVE_SPECIALIST, "Specialist"},
	{GLOVE_HYDRA, "Hydra"}
};

const std::vector<weapon_name> k_weapon_names =
{
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

const std::vector<weapon_name> k_knife_names_preview =
{
	{0, "default"},
	{WEAPON_KNIFE_BAYONET, "weapon_bayonet"},
	{WEAPON_KNIFE_CSS, "weapon_knife_css"},
	{WEAPON_KNIFE_CORD, "weapon_knife_cord"},
	{WEAPON_KNIFE_CANIS, "weapon_knife_canis"},
	{WEAPON_KNIFE_OUTDOOR, "weapon_knife_outdoor"},
	{WEAPON_KNIFE_SKELETON, "weapon_knife_skeleton"},
	{WEAPON_KNIFE_FLIP, "weapon_knife_flip"},
	{WEAPON_KNIFE_GUT, "weapon_knife_gut"},
	{WEAPON_KNIFE_KARAMBIT, "weapon_knife_karambit"},
	{WEAPON_KNIFE_M9_BAYONET, "weapon_knife_m9_bayonet"},
	{WEAPON_KNIFE_TACTICAL, "weapon_knife_tactical"},
	{WEAPON_KNIFE_FALCHION, "weapon_knife_falchion"},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, "weapon_knife_survival_bowie"},
	{WEAPON_KNIFE_BUTTERFLY, "weapon_knife_butterfly"},
	{WEAPON_KNIFE_PUSH, "weapon_knife_push"},
	{WEAPON_KNIFE_URSUS, "weapon_knife_ursus"},
	{WEAPON_KNIFE_GYPSY_JACKKNIFE, "weapon_knife_gypsy_jackknife"},
	{WEAPON_KNIFE_STILETTO, "weapon_knife_stiletto"},
	{WEAPON_KNIFE_WIDOWMAKER, "weapon_knife_widowmaker"}
};

const std::vector<weapon_name> k_glove_names_preview =
{
	{0, "default"},
	{GLOVE_T_SIDE, "default (t side)"},
	{GLOVE_CT_SIDE, "default (ct side)"},
	{GLOVE_STUDDED_BLOODHOUND, "studded_bloodhound_gloves"},
	{GLOVE_SPORTY, "sporty_gloves"},
	{GLOVE_SLICK, "slick_gloves"},
	{GLOVE_LEATHER_WRAP, "leather_handwraps"},
	{GLOVE_MOTORCYCLE, "motorcycle_gloves"},
	{GLOVE_SPECIALIST, "specialist_gloves"},
	{GLOVE_HYDRA, "studded_hydra_gloves"}
};

const std::vector<weapon_name> k_weapon_names_preview =
{
	{WEAPON_KNIFE,    "knife ct"},
	{WEAPON_KNIFE_T,  "knife t"},
	{GLOVE_CT_SIDE,   "glove ct"},
	{GLOVE_T_SIDE,    "glove t"},

	{61,              "weapon_usp_silencer"},
	{32,              "weapon_hkp2000"},
	{4,               "weapon_glock"},
	{2,               "weapon_elite"},
	{36,              "weapon_p250"},
	{3,               "weapon_fiveseven"},
	{30,              "weapon_tec9"},
	{63,              "weapon_cz75a"},
	{64,              "weapon_revolver"},
	{1,               "weapon_deagle"},

	{34,              "weapon_mp9"},
	{17,              "weapon_mac10"},
	{23,              "weapon_mp5sd"},
	{33,              "weapon_mp7"},
	{24,              "weapon_ump45"},
	{19,              "weapon_p90"},
	{26,              "weapon_bizon"},

	{7,               "weapon_ak47"},
	{60,              "weapon_m4a1_silencer"},
	{16,              "weapon_m4a1"},
	{8,               "weapon_aug"},
	{39,              "weapon_sg556"},
	{10,              "weapon_famas"},
	{13,              "weapon_galilar"},

	{40,              "weapon_ssg08"},
	{38,              "weapon_scar20"},
	{9,               "weapon_awp"},
	{11,              "weapon_g3sg1"},

	{14,              "weapon_m249"},
	{28,              "weapon_negev"},

	{27,              "weapon_mag7"},
	{35,              "weapon_nova"},
	{29,              "weapon_sawedoff"},
	{25,              "weapon_xm1014"},
};
