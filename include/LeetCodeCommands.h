#pragma once
#include <dpp/dpp.h>
#include "LeetoDB.h"
#include "LeetCodeAPI.h"

dpp::task<void> handle_leetcode_commands(dpp::cluster& bot, const dpp::slashcommand_t& event, LeetoDB& db);