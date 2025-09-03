#pragma once
#include <dpp/dpp.h>
#include "LeetoDB.h"

#include "InfoCommands.h"
#include "LeetCodeCommands.h"
#include "RegistryCommands.h"

void register_slash_commands(dpp::cluster& bot);
dpp::task<void> handle_slash_commands(dpp::cluster& bot, const dpp::slashcommand_t& event, LeetoDB& db);