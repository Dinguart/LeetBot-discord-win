#include "../include/Leeto.h"
#include <dpp/dpp.h>

#include "../include/ReadAuthFile.h"

#include "../include/SlashCommands.h"

#include "../include/LeetCodeButtonHandlers.h"
#include "../include/LeetCodePageHandlers.h"

#include "../include/LeetoDB.h"

int main()
{
	// connect database credentials
	auto DATABASE = readAuthFile("include/config.env");

	std::string DB_HOST, DB_USER, DB_PASS, DB_NAME;
	DB_HOST = DATABASE["DB_HOST"];
	DB_USER = DATABASE["DB_USER"];
	DB_PASS = DATABASE["DB_PASS"];
	DB_NAME = DATABASE["DB_NAME"];

	LeetoDB db(DB_HOST, DB_USER, DB_PASS, DB_NAME);

	if (!db.connect()) {
		std::cerr << "Failed to connect to the database!\n";
		return 1;
	}

	// connect discord bot token

	auto TOKEN = readAuthFile("include/token.env");

	std::string BOT_TOKEN = TOKEN["BOT_TOKEN"];

	dpp::cluster bot(BOT_TOKEN);

	bot.intents = dpp::i_default_intents | dpp::i_guild_members;

	bot.on_log(dpp::utility::cout_logger());

	bot.on_ready([&bot](const dpp::ready_t& event) {
		register_slash_commands(bot);
		});

	bot.on_slashcommand([&bot, &db](const dpp::slashcommand_t& event) -> dpp::task<void> {
		co_await handle_slash_commands(bot, event, db);
		});

	bot.on_button_click([&bot](const dpp::button_click_t& event) {
		const std::string& custom_id = event.custom_id;

		handle_solution_button_click(bot, event);
		if (custom_id.starts_with("solution_prev_page:") || custom_id.starts_with("solution_next_page:")) {
			//event.thinking();
			handle_solutions_page_button(event);
		}
		});

	bot.on_select_click([](const dpp::select_click_t& event) {
		handle_solution_dropdown(event);
		});

	bot.start(dpp::st_wait);
	return 0;
}
