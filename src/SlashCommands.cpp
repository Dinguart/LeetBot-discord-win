#include "../include/SlashCommands.h"

void register_slash_commands(dpp::cluster& bot) {
	if (dpp::run_once<struct register_bot_commands>()) {
		std::vector<dpp::slashcommand> commands;

		// ping pong
		commands.emplace_back(
			dpp::slashcommand("ping", "Replies with Pong!", bot.me.id)
		);

		// info command group
		dpp::slashcommand info_cmd("info", "info commands.", bot.me.id);

		// info general
		info_cmd.add_option(
			dpp::command_option(dpp::co_sub_command, "general", "General information about the bot.")
		);

		// register command group
		dpp::slashcommand register_cmd("register", "register commands.", bot.me.id);

		// register user (in database)
		register_cmd.add_option(
			dpp::command_option(dpp::co_sub_command, "user", "register user with the bot.")
			.add_option(dpp::command_option(dpp::co_string, "leetcode_name", "your leetcode username", true))
		);


		// leetcode command group
		dpp::slashcommand leetcode_cmd("leetcode", "leetcode interactive commands.", bot.me.id);

		// leetcode profile
		leetcode_cmd.add_option(
			dpp::command_option(dpp::co_sub_command, "profile", "get leetcode profile.")
		);
		// update (for profile changes and cache updates)
		leetcode_cmd.add_option(
			dpp::command_option(dpp::co_sub_command, "updateprofile", "updates your profile if anything is new.")
		);
		// daily question (from site)
		leetcode_cmd.add_option(
			dpp::command_option(dpp::co_sub_command, "dailyquestion", "Gets the daily leetcode question")
		);

		// daily streak
		leetcode_cmd.add_option(
			dpp::command_option(dpp::co_sub_command, "activity", "Gets your daily active streak from the site and total activity")
		);

		// 

		// add them to the main commands
		commands.push_back(info_cmd);
		commands.push_back(register_cmd);
		commands.push_back(leetcode_cmd);

		bot.global_bulk_command_create(commands);
	}
}

dpp::task<void> handle_slash_commands(dpp::cluster& bot, const dpp::slashcommand_t& event, LeetoDB& db)
{
	const std::string& cmd = event.command.get_command_name();

	if (cmd == "ping") {
		co_await event.co_reply("Pong!");
	}
	else if (cmd == "info") {
		co_await handle_info_commands(event);
	}
	else if (cmd == "register") {
		co_await handle_registry_commands(bot, event, db);
	}
	else if (cmd == "leetcode") {
		co_await handle_leetcode_commands(bot, event, db);
	}
}
