#include "nightmode.h"

#include "../../valve_sdk/csgostructs.hpp"
#include "../../Options.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../valve_sdk/misc/Convar.hpp"

using namespace std;
bool done = false;

namespace nightmode
{
	void doNightmode()
	{
		if (g_Options.nightmode)
		{
			if (!done)
			{
				static auto sv_skyname = g_CVar->FindVar("sv_skyname");
				static auto r_DrawSpecificStaticProp = g_CVar->FindVar("r_DrawSpecificStaticProp");
				r_DrawSpecificStaticProp->SetValue(1);
				sv_skyname->SetValue("sky_csgo_night02");

				for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
				{
					IMaterial *pMaterial = g_MatSystem->GetMaterial(i);

					if (!pMaterial)
						continue;

					const char* group = pMaterial->GetTextureGroupName();
					const char* name = pMaterial->GetName();

					if (strstr(group, "World textures"))
					{
						pMaterial->ColorModulate(0.10, 0.10, 0.10);
					}
					if (strstr(group, "StaticProp"))
					{
						pMaterial->ColorModulate(0.30, 0.30, 0.30);
					}
					if (strstr(name, "models/props/de_dust/palace_bigdome"))
					{
						pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
					}
					if (strstr(name, "models/props/de_dust/palace_pillars"))
					{
						pMaterial->ColorModulate(0.30, 0.30, 0.30);
					}

					if (strstr(group, "Particle textures"))
					{
						pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
					}
					done = true;
				}

			}
		}
		else
		{
			if (done)
			{
				for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
				{
					IMaterial *pMaterial = g_MatSystem->GetMaterial(i);

					if (!pMaterial)
						continue;

					const char* group = pMaterial->GetTextureGroupName();
					const char* name = pMaterial->GetName();

					if (strstr(group, "World textures"))
					{

						pMaterial->ColorModulate(1, 1, 1);
					}
					if (strstr(group, "StaticProp"))
					{

						pMaterial->ColorModulate(1, 1, 1);
					}
					if (strstr(name, "models/props/de_dust/palace_bigdome"))
					{
						pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
					}
					if (strstr(name, "models/props/de_dust/palace_pillars"))
					{

						pMaterial->ColorModulate(1, 1, 1);
					}
					if (strstr(group, "Particle textures"))
					{
						pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
					}
				}
				done = false;
			}
		}
	}
}


































































































