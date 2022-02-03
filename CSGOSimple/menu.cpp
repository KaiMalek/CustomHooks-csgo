#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "styles.h"
#include "features/visuals/visuals.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "render.hpp"
#include "hooks.hpp"
#include "features/skins/item_definitions.h"
#include "features/skins/kit_parser.h"
#include "features/skins/skins.h"

// implement colors tab finish
// desync from qo0
// finish backtrack
// search up player list

void ReadDirectory(const std::string& name, std::vector<std::string>& v)
{
    auto pattern(name);
    pattern.append("\\*.ch");
    WIN32_FIND_DATAA data;
    HANDLE hFind;
    if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            v.emplace_back(data.cFileName);
        } while (FindNextFileA(hFind, &data) != 0);
        FindClose(hFind);
    }
}
struct hud_weapons_t {
    std::int32_t* get_weapon_count() {
        return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
    }
};

namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        float clr[4] = {
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };
        //clr[3]=255;
        if (ImGui::ColorEdit4(label, clr, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
            v->SetColor(clr[0], clr[1], clr[2], clr[3]);
            return true;
        }
        return false;
    }
    inline bool ColorEdit4a(const char* label, Color* v, bool show_alpha = true)
    {
        float clr[4] = {
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };
        //clr[3]=255;
        if (ImGui::ColorEdit4(label, clr, show_alpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_AlphaBar)) {
            v->SetColor(clr[0], clr[1], clr[2], clr[3]);
            return true;
        }
        return false;
    }

    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

template<class T>
static T* FindHudElement(const char* name)
{
    static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan2("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

    static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan2("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33"));
    return (T*)find_hud_element(pThis, name);
}

void Menu::Initialize()
{
    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Spectator list ===================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

void Menu::SpectatorList()
{
    if (!g_Options.spectator_list)
        return;

    ImGui::End();

    std::string spectators;

    if (g_EngineClient->IsInGame() && g_LocalPlayer)
    {
        for (int i = 1; i <= g_GlobalVars->maxClients; i++)
        {
            auto ent = C_BasePlayer::GetPlayerByIndex(i);

            if (!ent || ent->IsAlive() || ent->IsDormant())
                continue;

            auto target = (C_BasePlayer*)ent->m_hObserverTarget();

            if (!target || target != g_LocalPlayer)
                continue;

            if (ent == target)
                continue;

            auto info = ent->GetPlayerInfo();

            spectators += std::string(info.szName) + u8"\n";
        }
    }

    ImGui::SetNextWindowPos({ 1800, 400 });
    if (ImGui::Begin("Spectator-List", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | (_visible ? NULL : ImGuiWindowFlags_NoMove)))
    {

        ImGui::Text("Spectator List");

        ImGui::Text(spectators.c_str());

    }
}

// Watermark =======================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

void Menu::Watermark()
{
    if (!g_Options.m_watermark)
        return;


    ImGui::SetNextWindowPos({ 1, 1 });
    if (ImGui::Begin("watermark", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | (_visible ? NULL : ImGuiWindowFlags_NoMove)))
    {
        ImGui::Text("customhooks.tk [beta]");
    }
    ImGui::End();
}

// Player List ========================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

void Menu::PlayerList()
{
    if (!g_Options.player_list)
        return;

    ImGui::SetNextWindowSize(ImVec2{ 600.f, 400.f }, ImGuiCond_Once);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f));
    ImGui::Begin("Player List", &_visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetCursorPosX(7);
    ImGui::SetCursorPosY(2);
    ImGui::Text("                                     Player List");
    ImGui::BeginChild("PlayerList 2", { 585, 372 });
    {
        ImGui::PopStyleColor();
        ImGui::SetCursorPosX(5);
        ImGui::SetCursorPosY(5);
        ImGui::BeginChild("Child for playerlist", ImVec2(575, 250), true);
        {
        }
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::End();
}

// ESP Preview and Image =========================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

void Menu::Preview()
{
    if (!g_Options.Preview)
        return;
    ImGui::SetNextWindowSize(ImVec2(235, 400));
    ImGui::Begin("esp previewwwwww", &_visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPosX(7);
    ImGui::SetCursorPosY(2);
    ImGui::Text("            Preview");
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImColor c = ImColor(32, 114, 247);

        const auto cur_window = ImGui::GetCurrentWindow();
        const ImVec2 w_pos = cur_window->Pos;

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1); 
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 1); 


        if (g_Options.esp_player_boxes) 
        {
            cur_window->DrawList->AddRect(ImVec2(w_pos.x + 40, w_pos.y + 60), ImVec2(w_pos.x + 200, w_pos.y + 360), ImGui::GetColorU32(ImGuiCol_Text));
        }

        if (g_Options.esp_player_health)
        {
            cur_window->DrawList->AddRectFilled(ImVec2(w_pos.x + 34, w_pos.y + 60), ImVec2(w_pos.x + 36, w_pos.y + 360), ImGui::GetColorU32(ImVec4(83 / 255.f, 200 / 255.f, 84 / 255.f, 255 / 255.f)));
        }

        if (g_Options.esp_player_names)
        {
            ImGui::SetCursorPosX(101);
            ImGui::SetCursorPosY(45);
            ImGui::Text("Name");
        }

        if (g_Options.esp_player_weapons)
        {
            ImGui::SetCursorPosX(101);
            ImGui::SetCursorPosY(362);
            ImGui::Text("AK-47");
        }

        if (g_Options.esp_player_armour)
        {
            ImGui::SetCursorPosX(202);
            ImGui::SetCursorPosY(62);
            ImGui::Text("HK");
        }
    ImGui::End();
}


bool Button3(const char* label, const ImVec2& size_arg, bool state)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);
    if (pressed)
        ImGui::MarkItemEdited(id);

    ImGui::RenderFrame(bb.Min, bb.Max, state ? ImColor(35, 35, 35) : ImColor(48, 48, 48), true, style.FrameRounding);
    window->DrawList->AddRect(bb.Min, bb.Max, ImColor(0, 0, 0));
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    if (state)
    {
        window->DrawList->AddLine(bb.Max - ImVec2(0, 1), bb.Max - ImVec2(60, 1), ImColor(255, 255, 255), 1);
    }
    // checkmark color ImVec4(1.00f, 0.99f, 0.00f, 1.00f)
    return pressed;
}


// Menu start =============================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================

void Menu::Render()
{
    SetupStyles();
    SpectatorList();
    Watermark();

    if (!_visible)
        return;

    PlayerList();
    Preview();

    static int activeTab = 0;
    ImGui::SetNextWindowSize(ImVec2{ 500.f, 550.f }, ImGuiCond_Once);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
    ImGui::Begin("CustomHooks", &_visible, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetCursorPosX(7);
    ImGui::SetCursorPosY(2);
    ImGui::Text("                           CustomHooks [beta]");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.00f)); // Push grey child bg
    ImGui::BeginChild("ButtonsForChild", { 484, 20 }); // Add a parameter `true` if u want border
    {
        static int tab = 0;
        static int sub_rage = 0;
        static int sub_legit = 0;
        static int sub_visuals = 0;
        static int sub_misc = 0;
        static int sub_skins = 0;
        ImGui::PopStyleColor();
        if (Button3("Aimbot", { 60, 20 }, activeTab == 0))
        {
            activeTab = 0;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(62);
        if (Button3("Visuals", { 60, 20 }, activeTab == 1))
        {
            activeTab = 1;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(124);
        if (Button3("Misc", { 60, 20 }, activeTab == 2))
        {
            activeTab = 2;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(186);
        if (Button3("Changer", { 60, 20 }, activeTab == 3))
        {
            activeTab = 3;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(248);
        if (Button3("Config", { 60, 20 }, activeTab == 4))
        {
            activeTab = 4;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(310);
        if (Button3("Colors", { 60, 20 }, activeTab == 5))
        {
            activeTab = 5;
        }


    }
    ImGui::EndChild();
    ImGui::BeginGroup();
    if (activeTab == 0)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
        ImGui::SetCursorPosY(40);
        ImGui::BeginChild("Aimbot", { 484, 500 }); // Add a parameter `true` if u want border
        {
            ImGui::PopStyleColor();
            ImGui::SetCursorPosX(5);
            ImGui::SetCursorPosY(5);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            static int definition_index = WEAPON_INVALID;

            auto localPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
            if (g_EngineClient->IsInGame() && localPlayer && localPlayer->IsAlive() && localPlayer->m_hActiveWeapon() && localPlayer->m_hActiveWeapon()->IsGun())
                definition_index = localPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex();
            else
                definition_index = WEAPON_INVALID;
            if (definition_index == WEAPON_INVALID)definition_index = WEAPON_DEAGLE;
            ImGui::BeginChild("aimbot 1", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
                ImGui::Separator("Legitbot");
                ImGui::Spacing();
                ImGui::Checkbox("Resolver", &g_Options.Presolver);

            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::BeginChild("aimbot 2", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
                auto settings = &g_Options.weapons[definition_index].legit;
                ImGui::Separator("Recoil System");
                ImGui::Spacing();
                ImGui::Checkbox("Enabled##rcs", &settings->rcs.enabled);
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::SliderInt("X", &settings->rcs.x, 0, 100, "%i");
                ImGui::Spacing();		ImGui::Spacing();		ImGui::Spacing();
                ImGui::SliderInt("Y", &settings->rcs.y, 0, 100, "%i");
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    else if (activeTab == 1)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
        ImGui::SetCursorPosY(40);
        ImGui::BeginChild("Visuals", { 484, 500 }); // Add a parameter `true` if u want border
        {
            ImGui::PopStyleColor();
            ImGui::SetCursorPosX(5);
            ImGui::SetCursorPosY(5);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);

            ImGui::BeginChild("ESP", ImVec2(232, 230), true); // esp
            {
                ImGui::PopStyleVar();
                ImGui::Separator("ESP");
                ImGui::Spacing();
                ImGui::Checkbox("Boxes", &g_Options.esp_player_boxes);
                ImGui::Spacing();
                ImGui::Checkbox("Skeleton", &g_Options.esp_skeleton);
                ImGui::Spacing();
                ImGui::Checkbox("Names", &g_Options.esp_player_names);
                ImGui::Spacing();
                ImGui::Checkbox("Health", &g_Options.esp_player_health);
                ImGui::Spacing();
                ImGui::Checkbox("Armour", &g_Options.esp_player_armour);
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::BeginChild("Chams And Glow", ImVec2(232, 230), true); // chams x glow  // dont make same label text names, idk why tf this is like that.
            {
                ImGui::PopStyleVar();
                ImGui::Separator("Chams");
                ImGui::Spacing();
                ImGui::Checkbox("Enable", &g_Options.chams_player_enabled);
                ImGui::Spacing();
                ImGui::Checkbox("Ignore Z", &g_Options.chams_player_ignorez);
                ImGui::Spacing();
                ImGui::Checkbox("Wireframe", &g_Options.chams_player_wireframe);
                ImGui::Spacing();
                ImGui::Checkbox("Flat", &g_Options.chams_player_flat);
                ImGui::Spacing();
                ImGui::Checkbox("Glass", &g_Options.chams_player_glass);
                ImGui::Spacing();
                ImGui::Separator("Arm chams");
                ImGui::Spacing();
                ImGui::Checkbox("Enabled ", &g_Options.chams_arms_enabled);
                ImGui::Spacing();
                ImGui::Checkbox("Wireframe ", &g_Options.chams_arms_wireframe);
                ImGui::Spacing();
                ImGui::Checkbox("Flat ", &g_Options.chams_arms_flat);
                ImGui::Spacing();
                ImGui::Checkbox("Ignore-Z ", &g_Options.chams_arms_ignorez);
                ImGui::Spacing();
                ImGui::Checkbox("Glass ", &g_Options.chams_arms_glass);
                ImGui::Spacing();
                ImGui::Separator("Glow");
                ImGui::Spacing();
                ImGui::Checkbox("Enable ", &g_Options.glow_enabled);
                ImGui::Spacing();
                ImGui::Checkbox("Players", &g_Options.glow_players);
            }
            ImGui::EndChild();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::SetCursorPosX(5);
            ImGui::SetCursorPosY(240);
            ImGui::BeginChild("ViewModel", ImVec2(232, 255), true); // viewmodel and stuff
            {
                ImGui::PopStyleVar();
                ImGui::Separator("Viewmodel");
                ImGui::Spacing();
                ImGui::Checkbox("Custom viewmodel", &g_Options.custom_viewmodel);
                if (g_Options.custom_viewmodel) {
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::SliderInt("Y FOV Override", &g_Options.viewmodel_fov, 68, 120);
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::SliderInt("Offset X", &g_Options.viewmodel_offset_x, -20, 20);
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::SliderInt("Offset Y", &g_Options.viewmodel_offset_y, -20, 20);
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::SliderInt("Offset Z", &g_Options.viewmodel_offset_z, -20, 20);
                    ImGui::Spacing();
                    ImGui::Checkbox("Left hand knife", &g_Options.leftknife);
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();
            
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::BeginChild("World", ImVec2(232, 255), true); // world
            {
                ImGui::PopStyleVar();
                ImGui::Separator("World");
                ImGui::Spacing();
                ImGui::Checkbox("Nighmode", &g_Options.nightmode);
                ImGui::Spacing();
                ImGui::Checkbox("Remove Smoke", &g_Options.no_smoke);
                ImGui::Spacing();
                ImGui::Checkbox("Remove Flashbangs", &g_Options.no_flash);
                ImGui::Spacing();
                ImGui::Checkbox("Skybox changer", &g_Options.skyboxchanger);
                if (g_Options.skyboxchanger) {
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Combo("Skyboxes", &g_Options.skybox, "cs_tibet\0cs_baggage_skybox_\0embassy\0italy\0jungle\0office\0sky_cs15_daylight01_hdr\0vertigoblue_hdr\0sky_cs15_daylight02_hdr\0vertigo\0sky_day02_05_hdr\0nukeblank\0sky_venice\0sky_cs15_daylight03_hdr\0sky_cs15_daylight04_hdr\0sky_csgo_cloudy01\0sky_csgo_night02\0sky_csgo_night02b\0sky_csgo_night_flat\0sky_lunacy\0sky_dust\0vietnam\0amethyst\0sky_descent\0clear_night_sky\0otherworld\0cloudynight\0dreamyocean\0grimmnight\0sky051\0sky081\0sky091\0sky561\0");
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

    }
    else if (activeTab == 2)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
        ImGui::SetCursorPosY(40);
        ImGui::BeginChild("Misc", { 484, 500 }); // Add a parameter `true` if u want border
        {
            ImGui::PopStyleColor();
            ImGui::SetCursorPosX(5);
            ImGui::SetCursorPosY(5);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);

            ImGui::BeginChild("misc 1", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
                ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
                ImGui::Spacing();
                ImGui::Checkbox("Third Person", &g_Options.thirdperson.misc_thirdperson);
                //ImGui::SameLine();
                //ImGui::Hotkey("Thirdperson Hotkey", &g_Options.thirdperson.hotkey);
                if (g_Options.thirdperson.misc_thirdperson)
                {
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::SliderFloat("Distance", &g_Options.thirdperson.misc_thirdperson_dist, 0.f, 150.f);
                    ImGui::Spacing();
                    ImGui::Checkbox("Remove Crosshair", &g_Options.rendercrosshair);
                }
                ImGui::Spacing();
                ImGui::Checkbox("No Hands", &g_Options.misc_no_hands);
                ImGui::Spacing();
                ImGui::Checkbox("Rank reveal", &g_Options.misc_showranks);
                ImGui::Spacing();
                ImGui::Checkbox("Spectator list", &g_Options.spectator_list);
                ImGui::Spacing();
                ImGui::Checkbox("Watermark", &g_Options.m_watermark);
                ImGui::Spacing();
                ImGui::Checkbox("Player List", &g_Options.player_list);
                ImGui::Spacing();
                ImGui::Checkbox("Visuals Preview", &g_Options.Preview);
                ImGui::Spacing();
                ImGui::Checkbox("Remove Crosshair", &g_Options.rendercrosshair);
                ImGui::Spacing();
                ImGui::Checkbox("Hitmarker", &g_Options.hitmarker);
                ImGui::Spacing();
                ImGui::Checkbox("Dot Crosshair", &g_Options.dotcrosshair);
                ImGui::Spacing();
                ImGui::Checkbox("Visible Crosshair", &g_Options.VisibleCrosshair);
                ImGui::Spacing();
                if (ImGui::Button("Unload", ImVec2{ 217.f, 20.f })) {
                    g_Unload = true;
                }
            }

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::BeginChild("misc 2", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
                ImGui::Separator("Griefing");
                ImGui::Spacing();
                ImGui::Checkbox("Blockbot", &g_Options.blockbot);
                //ImGui::SameLine();
                //ImGui::Hotkey("", &g_Options.blockbotkey);
                //ImGui::Spacing();
                ImGui::Checkbox("Enable Region Changer", &g_Options.region_changer);
                ImGui::Spacing();

                const char* regions2[48] = { "Amsterdam [Netherlands]","Atlanta [U.S. South]","Mumbai [India]","Guangzhou [China]","Guangzhou #2 [China]","Guangzhou #3 [China]","Guangzhou #4 [China]","Dubai [United Arab Emirates]","Seattle [U.S. North]","Frankfurt [Switzerland]","Sao Paulo [Brazil]","Hong Kong [China]","Washington D.C [U.S. East]","Cape Town [South Africa]","Los Angeles [U.S. West]","London [United Kingdom]","Lima [Peru]","Berlin [Germany]","Chennai [India]","Madrid [Spain]","Manila [Phillipines]","Oklahoma City [Canada]","Chicago [U.S.]","Paris [France]","Guangzhou #5 [China]","Tianjin [China]","Hebei [China]","Wuhan [China]","Jiaxing [China]","Santiago [Chile]","Seoul [South Korea]","Singapore [Australia]","Shanghai [China]","Shanghai #2 [China]","Shanghai #3 [China]","Shanghai #4 [China]","Nakashibetsu [Japan]","Moscow [Russia]","Moscow #2 [Russia]","Sydney [Australia]","Beijing [China]","Beijing #2 [China]","Beijing #3 [China]","Beijing #4 [China]","Tokyo [Japan]","Tokyo #2 [Japan]","Vienna [Austria]","Warsaw [Poland]" };
                static int selectedItem = 0;

                ImGui::Separator("M-M Region Changer");
                ImGui::PushItemWidth(217.f);
                ImGui::ListBox("", &selectedItem, regions2, 48);
                if (ImGui::Button("Change Region"))
                {
                    Visuals::Get().ChangeRegion();
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }

    else if (activeTab == 3)
    {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
    ImGui::SetCursorPosY(40);
    ImGui::BeginChild("Skins", { 484, 500 }); // Add a parameter `true` if u want border
    {
        ImGui::PopStyleColor();
        ImGui::SetCursorPosX(5);
        ImGui::SetCursorPosY(5);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
        static std::string selected_weapon_name = "";
        static std::string selected_skin_name = "";
        static auto definition_vector_index = 0;
        auto& entries = g_Options.changers.skin.m_items;
        ImGui::BeginChild("skins 1", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
        {
            ImGui::PopStyleVar();
            ImGui::Separator("Changer");
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();

            {
                for (size_t w = 0; w < k_weapon_names.size(); w++)
                {
                    switch (w)
                    {
                    case 0:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "knife");
                        break;
                    case 2:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "glove");
                        break;
                    case 4:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "pistols");
                        break;
                    case 14:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "semi-rifle");
                        break;
                    case 21:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "rifle");
                        break;
                    case 28:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "sniper-rifle");
                        break;
                    case 32:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "machingun");
                        break;
                    case 34:
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "shotgun");
                        break;
                    }

                    if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w))
                    {
                        definition_vector_index = w;
                    }
                }
            }
            //ImGui::ListBoxFooter();

            ImGui::Separator("Agent Changer");
            ImGui::PushItemWidth(211.f);
            ImGui::Spacing();
            ImGui::Combo("##TPlayerModel", &g_Options.playerModelCT, "CT Default Agent\0Special Agent Ava\0Operator\0Markus Delrow\0Michael Syfers\0B Squadron Officer\0Seal Team 6 Soldier\0Buckshot\0Commander Ricksaw\0Third Commando\0'Two Times' McCoy\0Dragomir\0Rezan The Ready\0The Doctor Romanov\0Maximus\0Blackwolf\0The Elite Muhlik\0Ground Rebel\0Osiris\0Shahmat\0Enforcer\0Slingshot\0Soldier\0");

            ImGui::PushItemWidth(211.f);
            ImGui::Combo("##CTPlayerModel", &g_Options.playerModelT, "T Default Agent\0Special Agent Ava\0Operator\0Markus Delrow\0Michael Syfers\0B Squadron Officer\0Seal Team 6 Soldier\0Buckshot\0Commander Ricksaw\0Third Commando\0'Two Times' McCoy\0Dragomir\0Rezan The Ready\0The Doctor Romanov\0Maximus\0Blackwolf\0The Elite Muhlik\0Ground Rebel\0Osiris\0Shahmat\0Enforcer\0Slingshot\0Soldier\0");
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
        ImGui::BeginChild("skins 2", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
        {
            ImGui::PopStyleVar();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
            auto& satatt = g_Options.changers.skin.statrack_items[k_weapon_names[definition_vector_index].definition_index];
            selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
            selected_entry.definition_vector_index = definition_vector_index;
            if (selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T)
            {
                ImGui::PushItemWidth(160.f);

                ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
                    {
                        *out_text = k_knife_names.at(idx).name;
                        return true;
                    }, nullptr, k_knife_names.size(), 10);
                selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

            }
            else if (selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE)
            {
                ImGui::PushItemWidth(160.f);

                ImGui::Combo("", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
                    {
                        *out_text = k_glove_names.at(idx).name;
                        return true;
                    }, nullptr, k_glove_names.size(), 10);
                selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
            }
            else {
                static auto unused_value = 0;
                selected_entry.definition_override_vector_index = 0;
            }

            if (selected_entry.definition_index != GLOVE_T_SIDE &&
                selected_entry.definition_index != GLOVE_CT_SIDE &&
                selected_entry.definition_index != WEAPON_KNIFE &&
                selected_entry.definition_index != WEAPON_KNIFE_T)
            {
                selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
            }
            else
            {
                if (selected_entry.definition_index == GLOVE_T_SIDE ||
                    selected_entry.definition_index == GLOVE_CT_SIDE)
                {
                    selected_weapon_name = k_glove_names_preview.at(selected_entry.definition_override_vector_index).name;
                }
                if (selected_entry.definition_index == WEAPON_KNIFE ||
                    selected_entry.definition_index == WEAPON_KNIFE_T)
                {
                    selected_weapon_name = k_knife_names_preview.at(selected_entry.definition_override_vector_index).name;
                }
            }
            if (skins_parsed)
            {
                static char filter_name[32];
                std::string filter = filter_name;

                bool is_glove = selected_entry.definition_index == GLOVE_T_SIDE ||
                    selected_entry.definition_index == GLOVE_CT_SIDE;

                bool is_knife = selected_entry.definition_index == WEAPON_KNIFE ||
                    selected_entry.definition_index == WEAPON_KNIFE_T;

                int cur_weapidx = 0;
                if (!is_glove && !is_knife)
                {
                    cur_weapidx = k_weapon_names[definition_vector_index].definition_index;
                    //selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
                }
                else
                {
                    if (selected_entry.definition_index == GLOVE_T_SIDE ||
                        selected_entry.definition_index == GLOVE_CT_SIDE)
                    {
                        cur_weapidx = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
                    }
                    if (selected_entry.definition_index == WEAPON_KNIFE ||
                        selected_entry.definition_index == WEAPON_KNIFE_T)
                    {
                        cur_weapidx = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

                    }
                }

                /*	ImGui::InputText("name filter [?]", filter_name, sizeof(filter_name));
                    if (ImGui::ItemsToolTipBegin("##skinfilter"))
                    {
                        ImGui::Checkbox("show skins for selected weapon", &g_Options.changers.skin.show_cur);
                        ImGui::ItemsToolTipEnd();
                    }*/

                auto weaponName = weaponnames(cur_weapidx);
                /*ImGui::Spacing();

                ImGui::Spacing();
                ImGui::SameLine();*/
                //ImGui::ListBoxHeader("##sdsdadsdadas", ImVec2(155, 245));
                {
                    if (selected_entry.definition_index != GLOVE_T_SIDE && selected_entry.definition_index != GLOVE_CT_SIDE)
                    {
                        if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
                        {
                            selected_entry.paint_kit_vector_index = -1;
                            selected_entry.paint_kit_index = -1;
                            selected_skin_name = "";
                        }

                        int lastID = ImGui::GetItemID();
                        for (size_t w = 0; w < k_skins.size(); w++)
                        {
                            for (auto names : k_skins[w].weaponName)
                            {
                                std::string name = k_skins[w].name;

                                if (g_Options.changers.skin.show_cur)
                                {
                                    if (names != weaponName)
                                        continue;
                                }

                                if (name.find(filter) != name.npos)
                                {
                                    ImGui::PushID(lastID++);

                                    ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(is_knife && g_Options.changers.skin.show_cur ? 6 : k_skins[w].rarity));
                                    {
                                        if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
                                        {
                                            selected_entry.paint_kit_vector_index = w;
                                            selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
                                            selected_skin_name = k_skins[w].name_short;
                                        }
                                    }
                                    ImGui::PopStyleColor();

                                    ImGui::PopID();
                                }
                            }
                        }
                    }
                    else
                    {
                        int lastID = ImGui::GetItemID();

                        if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
                        {
                            selected_entry.paint_kit_vector_index = -1;
                            selected_entry.paint_kit_index = -1;
                            selected_skin_name = "";
                        }

                        for (size_t w = 0; w < k_gloves.size(); w++)
                        {
                            for (auto names : k_gloves[w].weaponName)
                            {
                                std::string name = k_gloves[w].name;
                                //name += " | ";
                                //name += names;

                                if (g_Options.changers.skin.show_cur)
                                {
                                    if (names != weaponName)
                                        continue;
                                }

                                if (name.find(filter) != name.npos)
                                {
                                    ImGui::PushID(lastID++);

                                    ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(6));
                                    {
                                        if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
                                        {
                                            selected_entry.paint_kit_vector_index = w;
                                            selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
                                            selected_skin_name = k_gloves[selected_entry.paint_kit_vector_index].name_short;
                                        }
                                    }
                                    ImGui::PopStyleColor();

                                    ImGui::PopID();
                                }
                            }
                        }
                    }
                }
                //	ImGui::ListBoxFooter();
            }
            else
            {
                ImGui::Text("skins parsing, wait...");
            }
            ImGui::Checkbox("skin preview", &g_Options.changers.skin.skin_preview);
            ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);
            ImGui::InputInt("seed", &selected_entry.seed);
            ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
            ImGui::SliderFloat("wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
            
            if (ImGui::Button("Apply", ImVec2(ImGui::GetContentRegionAvailWidth(), 18.f)))
            {
                Utils::force_full_update();

                /*if (wearIndex == 0) //Factory New
                    selected_entry.wear = 0.01;

                if (wearIndex == 1) //Minimal Wear
                    selected_entry.wear = 0.08;

                if (wearIndex == 2) //Field Tested
                    selectwed_entry.wear = 0.16;

                if (wearIndex == 3) //Well Worn
                    selected_entry.wear = 0.39;

                if (wearIndex == 4) //Battle Scarred
                    selected_entry.wear = 0.46; */
            }

        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
    }
    else if (activeTab == 4)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
        ImGui::SetCursorPosY(40);
        ImGui::BeginChild("Config", { 484, 500 }); // Add a parameter `true` if u want border
        {
            ImGui::PopStyleColor();
            ImGui::SetCursorPosX(5);
            ImGui::SetCursorPosY(5);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);

            ImGui::BeginChild("config 1", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();

                static int selected = 0;
                static char cfgName[64];

                std::vector<std::string> cfgList;
                ReadDirectory(g_Options.folder, cfgList);
                ImGui::PushItemWidth(211.f);
                if (!cfgList.empty())
                {
                    ImGui::Separator("Settings");
                    if (ImGui::BeginCombo("##SelectConfig", cfgList[selected].c_str(), ImGuiComboFlags_NoArrowButton))
                    {
                        for (size_t i = 0; i < cfgList.size(); i++)
                        {
                            if (ImGui::Selectable(cfgList[i].c_str(), i == selected))
                            {
                                selected = i;
                            }
                        }
                        ImGui::EndCombo();
                    }
                    if (ImGui::Button("Save", ImVec2{ 217.f, 20.f }))
                    {
                        g_Options.SaveCFG(cfgList[selected]);
                    }
                    //ImGui::SameLine();
                    if (ImGui::Button("Load", ImVec2{ 217.f, 20.f }))
                    {
                        g_Options.LoadCFG(cfgList[selected]);
                    }
                    //ImGui::SameLine();
                    if (ImGui::Button("Delete", ImVec2{ 217.f, 20.f }))
                    {
                        g_Options.DeleteCFG(cfgList[selected]);
                        selected = 0;
                    }
                    //	ImGui::Separator();
                }
                ImGui::Spacing();
                ImGui::Separator("Create Config");
                ImGui::PushItemWidth(211.f);
                ImGui::InputText("##configname", cfgName, 24);
                //ImGui::SameLine();
                if (ImGui::Button("Create", ImVec2{ 217.f, 20.f }))
                {
                    if (strlen(cfgName))
                    {
                        g_Options.SaveCFG(cfgName + std::string(".ch"));
                    }
                }
                ImGui::PopItemWidth();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::BeginChild("config 2", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

    }
    else if (activeTab == 5)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.23f, 0.23f, 0.23f, 1.00f)); // Push grey child bg
        ImGui::SetCursorPosY(40);
        ImGui::BeginChild("Colors", { 484, 500 }); // Add a parameter `true` if u want border
        {
            ImGui::PopStyleColor();
            ImGui::SetCursorPosX(5);
            ImGui::SetCursorPosY(5);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);

            ImGui::BeginChild("Colors 1", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
                ImGui::Spacing();
                ImGuiEx::ColorEdit3("ESP Box Color", &g_Options.color_esp_enemy_visible);
                ImGui::SameLine();
                ImGui::Text("ESP Box Color");

                ImGui::Spacing();
                ImGuiEx::ColorEdit3("Chams Visible Color", &g_Options.color_chams_player_enemy_visible);
                ImGui::SameLine();
                ImGui::Text("Chams Visible Color");

                    ImGui::Spacing();
                ImGuiEx::ColorEdit3("Chams Behind Walls Color", &g_Options.color_chams_player_enemy_occluded);
                ImGui::SameLine();
                ImGui::Text("Chams Occluded Color");

                    ImGui::Spacing();
                ImGuiEx::ColorEdit3("Chams Arms Visible Color", &g_Options.color_chams_arms_visible);
                ImGui::SameLine();
                ImGui::Text("Chams Arms Visible Color");

                    ImGui::Spacing();
                ImGuiEx::ColorEdit3("Chams Arms Behind Wall Color", &g_Options.color_chams_arms_occluded);
                ImGui::SameLine();
                ImGui::Text("Chams Arms Occluded Color");

                    ImGui::Spacing();
                ImGuiEx::ColorEdit3("Glow Color", &g_Options.color_glow_enemy);
                ImGui::SameLine();
                ImGui::Text("Glow Color");
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.f);
            ImGui::BeginChild("Colors 2", ImVec2(232, 490), true); // 151 is the x axis and 152 is the y axis so changed them to make them bigger or smaller
            {
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();

    }
}

void Menu::Toggle()
{
    _visible = !_visible;
}
