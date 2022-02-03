#pragma once

#include "../../singleton.hpp"

#include "../../render.hpp"
#include "../../helpers/math.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../../singleton.hpp"




class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;

		bool Begin(C_BasePlayer * pl);
		void RenderBox();
		void RenderName();
		void RenderWeaponName();
		void RenderHealth();
		void RenderArmour();
		void RenderSnapline();
		void leftknife();
	};
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void ThirdPerson();
	void AddToDrawList();
	void RenderCrosshair();
	void ChangeRegion();
	void run_viewmodel();
	void DotCrosshair();
public:
	void Render();

	void acpect(float numbero)
	{
		ConVar* yessoftware = g_CVar->FindVar("r_aspectratio");
		*(int*)((DWORD)&yessoftware->m_fnChangeCallbacks + 0xC) = 0;
		yessoftware->SetValue(numbero);
	}
};

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class Chams
	: public Singleton<Chams>
{
	friend class Singleton<Chams>;

	Chams();
	~Chams();

public:
	void OnDrawModelExecute(
		IMatRenderContext* ctx,
		const DrawModelState_t& state,
		const ModelRenderInfo_t& pInfo,
		matrix3x4_t* pCustomBoneToWorld);

private:
	void OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba);

	IMaterial* materialRegular = nullptr;
	IMaterial* materialFlat = nullptr;
};

class Glow
	: public Singleton<Glow>
{
	friend class Singleton<Glow>;

	Glow();
	~Glow();

public:
	void Run();
	void Shutdown();
};

//void set(const char* tag);
//void set(const char* tag, const char* label);;
