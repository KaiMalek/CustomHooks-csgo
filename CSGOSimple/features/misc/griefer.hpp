#pragma once
#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include "../../helpers/input.hpp"


class CBlockBot {
private:
	C_BasePlayer* Target = nullptr;
public:
	void draw();
	void cmove(CUserCmd*);
};

extern CBlockBot* g_BlockBot;