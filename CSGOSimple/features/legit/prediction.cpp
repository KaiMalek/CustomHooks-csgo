#include "prediction.hpp"

PredictionSystem::PredictionSystem()
{
	m_pPredictionRandomSeed = *reinterpret_cast<int**>(Utils::PatternScan(GetModuleHandleA("client.dll"), "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 0x2);

	m_flOldCurtime = NULL;
	m_flOldFrametime = NULL;
}

void PredictionSystem::StartPrediction(CUserCmd* cmd)
{
	*m_pPredictionRandomSeed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;

	m_flOldCurtime = g_GlobalVars->curtime;
	m_flOldFrametime = g_GlobalVars->frametime;

	g_GlobalVars->curtime = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
	g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;

	g_GameMovement->StartTrackPredictionErrors(g_LocalPlayer);

	memset(&m_MoveData, 0, sizeof(m_MoveData));
	g_MoveHelper->SetHost(g_LocalPlayer);
	g_Prediction->SetupMove(g_LocalPlayer, cmd, g_MoveHelper, &m_MoveData);
	g_GameMovement->ProcessMovement(g_LocalPlayer, &m_MoveData);
	g_Prediction->FinishMove(g_LocalPlayer, cmd, &m_MoveData);

}

void PredictionSystem::EndPrediction()
{
	g_GameMovement->FinishTrackPredictionErrors(g_LocalPlayer);
	g_MoveHelper->SetHost(nullptr);

	*m_pPredictionRandomSeed = -1;

	g_GlobalVars->curtime = m_flOldCurtime;
	g_GlobalVars->frametime = m_flOldFrametime;
}
