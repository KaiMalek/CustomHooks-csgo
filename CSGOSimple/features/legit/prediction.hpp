#pragma once

#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"

class PredictionSystem {
public:
	PredictionSystem();

	void StartPrediction(CUserCmd* cmd);
	void EndPrediction();

private:
	float m_flOldCurtime;
	float m_flOldFrametime;
	CMoveData m_MoveData;

	int* m_pPredictionRandomSeed;
};