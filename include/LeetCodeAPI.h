#pragma once
#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
#include <optional>

#include "constants.h"

#include "LeetoDB.h"

#include "ReadAuthFile.h"
#include "PerUserPage.h"
#include "LeetCodePageLogic.h"

#include "TextReplace.h"

void fetchLeetCodeDescription(dpp::cluster* bot, const std::string& username, const dpp::slashcommand_t& event, std::function<void(std::optional<std::string>)> callback, int retry_count = 0, std::shared_ptr<std::promise<std::optional<std::string>>> retry_promise = nullptr);

void fetchLeetCodeProfile(dpp::cluster* bot, const std::string& username, const std::string& discord_username, const std::string& discord_id, const dpp::slashcommand_t& event, LeetoDB& db, int retry_count = 0);

void fetchLeetCodeDailyQuestion(dpp::cluster* bot, const std::string& username, const dpp::slashcommand_t& event, LeetoDB& db, int retry_count = 0);

void fetchLeetCodeActivity(dpp::cluster* bot, const std::string& username, const dpp::slashcommand_t& event, int retry_count = 0);

void fetchLeetCodeDailyQuestionSolution(dpp::cluster* bot, const std::string& titleSlug, const std::string& languageSlug, const dpp::button_click_t& event, int retry_count = 0);

void fetchLeetCodeDailyQuestionDescription(dpp::cluster* bot, const std::string& titleSlug, const dpp::button_click_t& event, int retry_count = 0);