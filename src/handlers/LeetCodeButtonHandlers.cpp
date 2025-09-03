#include "../LeetCodeButtonHandlers.h"

std::unordered_map<dpp::snowflake, std::string> userLanguageMap;

void handle_solution_button_click(dpp::cluster& bot, const dpp::button_click_t& event) {
	const std::string& custom_id = event.custom_id;

	if (custom_id.starts_with("solution_id:")) {
		std::string titleSlug = custom_id.substr(std::string("solution_id:").size());
		titleSlug.erase(0, titleSlug.find_first_not_of(' '));

		dpp::snowflake userId = event.command.usr.id;
		auto it = userLanguageMap.find(userId);

		if (it == userLanguageMap.end()) {
			event.reply(dpp::message("Please choose a programming language from the language selection menu.").set_flags(dpp::m_ephemeral));
			return;
		}

		std::string languageSlug = it->second;
		event.thinking();
		fetchLeetCodeDailyQuestionSolution(&bot, titleSlug, languageSlug, event);
	}
	else if (custom_id.starts_with("description_id:")) {
		std::string titleSlug = custom_id.substr(std::string("description_id:").size());
		titleSlug.erase(0, titleSlug.find_first_not_of(' '));
		event.thinking(); // so discord waits for fetch
		fetchLeetCodeDailyQuestionDescription(&bot, titleSlug, event);
	}
}

void handle_solution_dropdown(const dpp::select_click_t& event) {
	std::string selected = event.values[0]; // gets what the user selected
	dpp::snowflake userId = event.command.usr.id;

	static const std::unordered_map<std::string, std::string> dropdownMap = {
		{"C++", "cpp"},
		{ "Java", "java" },
		{"Python", "python"},
		{"Python3", "python3"},
		{"C", "c"},
		{"C#", "csharp"},
		{"JavaScript", "javascript"},
		{"TypeScript", "typescript"},
		{"PHP", "php"},
		{"Swift", "swift"},
		{"Kotlin", "kotlin"},
		{"Dart", "dart"},
		{"Go", "golang"},
		{"Ruby", "ruby"},
		{"Scala", "scala"},
		{"Rust", "rust"},
		{"Racket", "racket"},
		{"Erlang", "erlang"},
		{"Elixir", "elixir"}
	};

	auto it = dropdownMap.find(selected);

	if (it != dropdownMap.end()) {
		userLanguageMap[userId] = it->second;
		event.reply(dpp::message("Language set to: " + selected).set_flags(dpp::m_ephemeral));
	}
	else {
		event.reply(dpp::message("Invalid language selection.").set_flags(dpp::m_ephemeral));
	}
}