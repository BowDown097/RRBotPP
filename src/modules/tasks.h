#pragma once
#include "dppcmd/modules/module.h"

class Tasks : public dppcmd::module<Tasks>
{
public:
	Tasks();
private:
	dpp::task<dppcmd::command_result> chop();
	dpp::task<dppcmd::command_result> dig();
	dpp::task<dppcmd::command_result> farm();
	dpp::task<dppcmd::command_result> fish();
	dpp::task<dppcmd::command_result> hunt();
	dpp::task<dppcmd::command_result> mine();

	dpp::task<dppcmd::command_result> genericTask(std::string_view toolType, std::string_view activity,
		std::string_view mineral, std::string_view cooldownKey, int64_t duration);
};
