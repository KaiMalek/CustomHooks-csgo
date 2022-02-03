#include <algorithm>
#include <fstream>
#include <mutex>
#include "visuals.hpp"

#include "../../options.hpp"
#include "../../helpers/math.hpp"
#include "../../helpers/utils.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../../hooks.hpp"
#include "../../runtime_saving.h"
#include "../../valve_sdk/misc/Convar.hpp"
#include "../../xor.h"
#include "../../patterns.h"
#include "../../valve_sdk/interfaces/IVModelInfoClient.hpp"

// ESP Context
// This is used so that we dont have to calculate player color and position
// on each individual function over and over
struct
{
	C_BasePlayer* pl;
	bool          is_enemy;
	bool          is_visible;
	Color         clr;
	Color		  filled;
	Vector        head_pos;
	Vector        feet_pos;
	RECT          bbox;
	float         Fade[64];
	int reservedspace_right;
	bool dangerzone;

} esp_ctx;

ConVar* r_3dsky = nullptr;
ConVar* r_drawspecificstaticprop = nullptr;

RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}


Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Render() {
}
//--------------------------------------------------------------------------------
int flPlayerAlpha[64];
int flAlphaFade = 5.f;

bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (!pl->IsAlive())
		return false;

	if (pl->IsDormant() && flPlayerAlpha[pl->EntIndex()] > 0)
	{
		flPlayerAlpha[pl->EntIndex()] -= flAlphaFade;
	}
	else if (flPlayerAlpha[pl->EntIndex()] < 255 && !(pl->IsDormant()))
	{
		flPlayerAlpha[pl->EntIndex()] += flAlphaFade;
	}
	if (flPlayerAlpha <= 0 && pl->IsDormant())
		return false;
	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy)
		return false;
	//ctx.clr = ctx.is_enemy ? (ctx.is_visible ? g_Options.color_esp_enemy_visible : g_Options.color_esp_enemy_occluded) : (ctx.is_visible ? g_Options.color_esp_ally_visible : g_Options.color_esp_ally_occluded);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	ctx.bbox = GetBBox(pl);

	std::swap(ctx.bbox.top, ctx.bbox.bottom);

	return !(!ctx.bbox.left || !ctx.bbox.top || !ctx.bbox.right || !ctx.bbox.bottom);
}

//--------------------------------------------------------------------------------
void Visuals::Player::RenderBox() 
{
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, Color(g_Options.color_esp_enemy_visible[0], g_Options.color_esp_enemy_visible[1], g_Options.color_esp_enemy_visible[2], 255), 1);
		Render::Get().RenderBoxByType(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color(0, 0, 0, 255), 1);
		Render::Get().RenderBoxByType(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color(0, 0, 0, 255), 1);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName()
{
	player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);

	Render::Get().RenderText(info.szName, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left - sz.x) / 2, (ctx.bbox.top - sz.y - 1), 12.f, Color(255, 255, 255, 255), false);

}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth()
{
	auto  hp = ctx.pl->m_iHealth();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 8;

	int height = (box_h * hp) / 100;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 3;
	int h = box_h;

	Render::Get().RenderBox(x, y - 1, x + w, y + h + 1, Color(0, 0, 0, 160));
	Render::Get().RenderBox(x + 1, y + h - height, x + w - 1, y + h, Color(0, 255, 0, 255));
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderArmour()
{
	auto  armour = ctx.pl->m_ArmorValue();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	//float off = (box_h / 6.f) + 5;
	float off = 4;

	int height = (((box_h * armour) / 100));

	int x = ctx.bbox.right + off;
	int y = ctx.bbox.top;
	int w = 3;
	int h = box_h;

	Render::Get().RenderBox(x, y - 1, x + w, y + h + 1, Color(0, 0, 0, 160));
	Render::Get().RenderBox(x + 1, y + h - height, x + w - 1, y + h, Color(0, 50, 255, 255));
}

//--------------------------------------------------------------------------------
void Visuals::Player::RenderWeaponName()
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;
	if (!weapon->GetCSWeaponData()) return;

	auto text = weapon->GetCSWeaponData()->szWeaponName + 7;
	auto sz = g_pDefaultFont->CalcTextSizeA(12.f, FLT_MAX, 0.0f, text);
	Render::Get().RenderText(text, ImVec2(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left - sz.x) / 2, ctx.bbox.bottom + 1), 12.f, Color(255, 255, 255, 255), false);
}
//--------------------------------------------------------------------------------

void Visuals::ChangeRegion()
{
	if (!g_Options.Enable_Region_Changer)
	{
		switch (g_Options.region_changer) {
		case 0:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster ams");
			break;
		case 1:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster atl");
			break;
		case 2:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster bom");
			break;
		case 3:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster can");
			break;
		case 4:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster canm");
			break;
		case 5:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster cant");
			break;
		case 6:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster canu");
			break;
		case 7:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster dxb");
			break;
		case 8:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster eat");
			break;
		case 9:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster fra");
			break;
		case 10:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster gru");
			break;
		case 11:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster hkg");
			break;
		case 12:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster iad");
			break;
		case 13:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster jnb");
			break;
		case 14:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster lax");
			break;
		case 15:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster lhr");
			break;
		case 16:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster lim");
			break;
		case 17:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster lux");
			break;
		case 18:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster maa");
			break;
		case 19:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster mad");
			break;
		case 20:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster man");
			break;
		case 21:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster okc");
			break;
		case 22:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster ord");
			break;
		case 23:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster par");
			break;
		case 24:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster pwg");
			break;
		case 25:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster pwj");
			break;
		case 26:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster pwu");
			break;
		case 27:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster pww");
			break;
		case 28:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster pwz");
			break;
		case 29:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster scl");
			break;
		case 30:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster sea");
			break;
		case 31:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster sgp");
			break;
		case 32:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster sha");
			break;
		case 33:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster sham");
			break;
		case 34:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster shat");
			break;
		case 35:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster shau");
			break;
		case 36:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster shb");
			break;
		case 37:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster sto");
			break;
		case 38:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster sto2");
			break;
		case 39:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster syd");
			break;
		case 40:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster tsn");
			break;
		case 41:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster tsnm");
			break;
		case 42:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster tsnt");
			break;
		case 43:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster tsnu");
			break;
		case 44:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster tyo");
			break;
		case 45:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster tyo1");
			break;
		case 46:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster vie");
			break;
		case 47:
			g_EngineClient->ExecuteClientCmd("sdr SDRClient_ForceRelayCluster waw");
			break;
		}
	}
	
}

void Visuals::run_viewmodel()
{
	g_CVar->FindVar("viewmodel_offset_x")->SetValue(g_Options.viewmodel_offset_x);
	g_CVar->FindVar("viewmodel_offset_y")->SetValue(g_Options.viewmodel_offset_y);
	g_CVar->FindVar("viewmodel_offset_z")->SetValue(g_Options.viewmodel_offset_z);
}

void Visuals::Player::RenderSnapline()
{

	int screen_w, screen_h;
	g_EngineClient->GetScreenSize(screen_w, screen_h);

	Render::Get().RenderLine(screen_w / 2.f, (float)screen_h,
		ctx.feet_pos.x, ctx.feet_pos.y, ctx.clr);
}
//--------------------------------------------------------------------------------
void Visuals::Player::leftknife()
{
	static auto left_knife = g_CVar->FindVar("cl_righthand");

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		left_knife->SetValue(0);
		return;
	}

	auto& weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return;

	left_knife->SetValue(!weapon->IsKnife());
}
// ------------------------------------------------------------------------------
void Visuals::RenderCrosshair()
{
	int w, h;

	g_EngineClient->GetScreenSize(w, h);

	int cx = w / 2;
	int cy = h / 2;
}
//--------------------------------------------------------------------------------

void Visuals::DotCrosshair()
{
	int w, h;

	g_EngineClient->GetScreenSize(w, h);
	
	int cx = w / 2;
	int cy = h / 2;
	Render::Get().RenderLine(cx, cy - 1, cx, cy + 1, g_Options.color_esp_crosshair);
	Render::Get().RenderLine(cx, cy - 1, cx, cy + 1, g_Options.color_esp_crosshair);
}

//--------------------------------------------------------------------------------

void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_weapons);


	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;


	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_weapons);
}
//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_defuse);

	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_defuse);
}
//--------------------------------------------------------------------------------
void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;


	Render::Get().RenderBox(bbox, g_Options.color_esp_c4);


	int bombTimer = std::ceil(ent->m_flC4Blow() - g_GlobalVars->curtime);
	std::string timer = std::to_string(bombTimer);

	auto name = (bombTimer < 0.f) ? "Bomb" : timer;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_c4);
}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;


	//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_item);
}
//--------------------------------------------------------------------------------
void Visuals::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	if (g_Options.thirdperson.misc_thirdperson && g_LocalPlayer->IsAlive())
	{
		if (!g_Input->m_fCameraInThirdPerson)
		{
			g_Input->m_fCameraInThirdPerson = true;
		}

		float dist = g_Options.thirdperson.misc_thirdperson_dist;

		QAngle *view = g_LocalPlayer->GetVAngles();
		trace_t tr;
		Ray_t ray;

		Vector desiredCamOffset = Vector(cos(DEG2RAD(view->yaw)) * dist,
			sin(DEG2RAD(view->yaw)) * dist,
			sin(DEG2RAD(-view->pitch)) * dist
		);

		//cast a ray from the Current camera Origin to the Desired 3rd person Camera origin
		ray.Init(g_LocalPlayer->GetEyePos(), (g_LocalPlayer->GetEyePos() - desiredCamOffset));
		CTraceFilter traceFilter;
		traceFilter.pSkip = g_LocalPlayer;
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &traceFilter, &tr);

		Vector diff = g_LocalPlayer->GetEyePos() - tr.endpos;

		float distance2D = sqrt(abs(diff.x * diff.x) + abs(diff.y * diff.y));// Pythagorean

		bool horOK = distance2D > (dist - 2.0f);
		bool vertOK = (abs(diff.z) - abs(desiredCamOffset.z) < 3.0f);

		float cameraDistance;

		if (horOK && vertOK)  // If we are clear of obstacles
		{
			cameraDistance = dist; // go ahead and set the distance to the setting
		}
		else
		{
			if (vertOK) // if the Vertical Axis is OK
			{
				cameraDistance = distance2D * 0.95f;
			}
			else// otherwise we need to move closer to not go into the floor/ceiling
			{
				cameraDistance = abs(diff.z) * 0.95f;
			}
		}
		g_Input->m_fCameraInThirdPerson = true;

		g_Input->m_vecCameraOffset.z = cameraDistance;
	}
	else
	{
		g_Input->m_fCameraInThirdPerson = false;
	}
}


void Visuals::AddToDrawList() {
	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;
		
		if (entity == g_LocalPlayer && !g_Input->m_fCameraInThirdPerson)
			continue;

		if (i <= g_GlobalVars->maxClients) {
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) {
				if (g_Options.esp_player_snaplines) player.RenderSnapline();
				if (g_Options.esp_player_boxes)     player.RenderBox();
				if (g_Options.esp_player_weapons)   player.RenderWeaponName();
				if (g_Options.esp_player_names)     player.RenderName();
				if (g_Options.esp_player_health)    player.RenderHealth();
				if (g_Options.esp_player_armour)    player.RenderArmour();
			}
		}
		else if (g_Options.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		else if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		else if (entity->IsPlantedC4() && g_Options.esp_planted_c4)
			RenderPlantedC4(entity);
		else if (entity->IsLoot() && g_Options.esp_items)
			RenderItemEsp(entity);
	}

	if (g_Options.dotcrosshair)
	{
		DotCrosshair();
	}


	if (g_Options.rendercrosshair)
	{
		RenderCrosshair();
	}
}

Chams::Chams() {
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube");
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat");
}

Chams::~Chams() {
}


void Chams::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba) {
	IMaterial* material = nullptr;

	if (flat) {
		material = materialFlat;
	}
	else {
		material = materialRegular;
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);


	if (glass) {
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}
	else {
		material->AlphaModulate(
			rgba.a() / 255.0f);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}


void Chams::OnDrawModelExecute(
	IMatRenderContext* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	//bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

	if (is_player && g_Options.chams_player_enabled) {
		// 
		// Draw player Chams.
		// 
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && g_LocalPlayer && ent->IsAlive()) {
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			if (!enemy && g_Options.chams_player_enemies_only)
				return;

			const auto clr_front = enemy ? g_Options.color_chams_player_enemy_visible : g_Options.color_chams_player_ally_visible;
			const auto clr_back = enemy ? g_Options.color_chams_player_enemy_occluded : g_Options.color_chams_player_ally_occluded;

			if (g_Options.chams_player_ignorez) {
				OverrideMaterial(
					true,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					clr_front);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					g_Options.chams_player_glass,
					clr_front);
			}
		}
	}
	else if (is_sleeve && g_Options.chams_arms_enabled) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Remove sleeves when drawing Chams.
		// 
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		g_MdlRender->ForcedMaterialOverride(material);
	}
	else if (is_arm) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.misc_no_hands) {
			// 
			// No hands.
			// 
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			if (g_Options.chams_arms_ignorez) {
				OverrideMaterial(
					true,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.color_chams_arms_occluded);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.color_chams_arms_visible);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					g_Options.chams_arms_glass,
					g_Options.color_chams_arms_visible);
			}
		}
	}
}

	Glow::Glow()
	{
	}

	Glow::~Glow()
	{
		// We cannot call shutdown here unfortunately.
		// Reason is not very straightforward but anyways:
		// - This destructor will be called when the dll unloads
		//   but it cannot distinguish between manual unload 
		//   (pressing the Unload button or calling FreeLibrary)
		//   or unload due to game exit.
		//   What that means is that this destructor will be called
		//   when the game exits.
		// - When the game is exiting, other dlls might already 
		//   have been unloaded before us, so it is not safe to 
		//   access intermodular variables or functions.
		//   
		//   Trying to call Shutdown here will crash CSGO when it is
		//   exiting (because we try to access g_GlowObjManager).
		//
	}

	void Glow::Shutdown()
	{
		// Remove glow from all entities
		for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) {
			auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
			auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

			if (glowObject.IsUnused())
				continue;

			if (!entity || entity->IsDormant())
				continue;

			glowObject.m_flAlpha = 0.0f;
		}
	}

	void Glow::Run()
	{
		for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) {
			auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
			auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

			if (glowObject.IsUnused())
				continue;

			if (!entity || entity->IsDormant())
				continue;

			auto class_id = entity->GetClientClass()->m_ClassID;
			auto color = Color{};

			switch (class_id) {
			case ClassId_CCSPlayer:
			{
				auto is_enemy = entity->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();

				if (entity->HasC4() && is_enemy && g_Options.glow_c4_carrier) {
					color = g_Options.color_glow_c4_carrier;
					break;
				}

				if (!g_Options.glow_players || !entity->IsAlive())
					continue;

				if (!is_enemy && g_Options.glow_enemies_only)
					continue;

				color = is_enemy ? g_Options.color_glow_enemy : g_Options.color_glow_ally;

				break;
			}
			case ClassId_CChicken:
				if (!g_Options.glow_chickens)
					continue;
				entity->m_bShouldGlow() = true;
				color = g_Options.color_glow_chickens;
				break;
			case ClassId_CBaseAnimating:
				if (!g_Options.glow_defuse_kits)
					continue;
				color = g_Options.color_glow_defuse;
				break;
			case ClassId_CPlantedC4:
				if (!g_Options.glow_planted_c4)
					continue;
				color = g_Options.color_glow_planted_c4;
				break;
			default:
			{
				if (entity->IsWeapon()) {
					if (!g_Options.glow_weapons)
						continue;
					color = g_Options.color_glow_weapons;
				}
			}
			}

			glowObject.m_flRed = color.r() / 255.0f;
			glowObject.m_flGreen = color.g() / 255.0f;
			glowObject.m_flBlue = color.b() / 255.0f;
			glowObject.m_flAlpha = color.a() / 255.0f;
			glowObject.m_bRenderWhenOccluded = true;
			glowObject.m_bRenderWhenUnoccluded = false;
		}
	}