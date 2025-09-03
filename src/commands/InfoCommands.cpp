#include "../../include/InfoCommands.h"

dpp::task<void> handle_info_commands(const dpp::slashcommand_t& event) {

	co_await event.co_thinking();

	const dpp::command_interaction& cmd_data = event.command.get_command_interaction();

	if (cmd_data.options.empty()) {
		co_await event.co_edit_original_response(dpp::message("No subcommand provided."));
		co_return;
	}

	const dpp::command_data_option& subcommand = cmd_data.options[0];

	if (subcommand.name == "general") {
		dpp::embed embed = dpp::embed()
			.set_title("Leetcode Tracking Bot")
			.set_description("General bot for tracking leetcode stats, has many different features to help in your Leetcode journey!")
			.set_color(dpp::colors::orange_gold);

		dpp::message msg;
		msg.add_embed(embed);

	
		co_await event.co_edit_original_response(msg);
		co_return;
	}
	else {
		co_await event.co_edit_original_response(dpp::message("Unknown subcommand."));
		co_return;
	}
}
