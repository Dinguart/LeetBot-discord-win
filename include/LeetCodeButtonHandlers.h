#pragma once
#include <dpp/dpp.h>
#include "LeetCodeAPI.h"

void handle_solution_button_click(dpp::cluster& bot, const dpp::button_click_t& event);

void handle_solution_dropdown(const dpp::select_click_t& event);