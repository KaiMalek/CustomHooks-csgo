#pragma once
#include "../../singleton.hpp"
#include "../../valve_sdk/csgostructs.hpp"

class player_log : public Singleton<player_log>
{
public:
	void log(const ClientFrameStage_t stage);
	player_log_t& get_log(const int index);
	void filter_records(bool cm = false);
private:
	player_log_t logs[65];
	
};