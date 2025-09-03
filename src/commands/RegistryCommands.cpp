#include "../../include/RegistryCommands.h"

dpp::task<void> handle_registry_commands(dpp::cluster& bot, const dpp::slashcommand_t& event, LeetoDB& db) {
	co_await event.co_thinking();
	
	dpp::snowflake guild_id = event.command.guild_id;
	dpp::snowflake user_id = event.command.get_issuing_user().id;

	const dpp::command_interaction& cmd_data = event.command.get_command_interaction();

	if (cmd_data.options.empty()) {
		co_await event.co_edit_original_response(dpp::message("No subcommand provided."));
		co_return;
	}

	const dpp::command_data_option& subcommand = cmd_data.options[0];

	if (subcommand.name == "user") {
		co_await event.co_edit_original_response(dpp::message("Registering user...").set_flags(dpp::m_ephemeral));

		std::string leetcode_user;

		for (const auto& opt : subcommand.options) {
			if (opt.name == "leetcode_name" && std::holds_alternative<std::string>(opt.value)) {
				leetcode_user = std::get<std::string>(opt.value);
			}
		}

		try {
			dpp::guild_member member = dpp::find_guild_member(guild_id, user_id);
			std::string username = member.get_user()->username;

			if (username.empty()) {
				co_await event.co_edit_original_response(dpp::message("No username set, please reach discord support for help."));
				co_return;
			}

			std::string registeredUser = db.getLeetcodeUsername(std::to_string(user_id)).value_or("");
			if (leetcode_user == registeredUser) {
				co_await event.co_edit_original_response(dpp::message("This user has already been registered."));
				co_return;
			}

			auto promise = std::make_shared<std::promise<std::optional<std::string>>>();
			auto future = promise->get_future();

			std::string discord_id_string = std::to_string(user_id);

			std::string description;

			fetchLeetCodeDescription(&bot, leetcode_user, event, [promise](std::optional<std::string> desc) mutable {
				promise->set_value(desc);
				}, 0, promise);
			std::optional<std::string> desc = co_await AwaitableFuture<std::optional<std::string>>{std::move(future)};;

			if (!desc) {
				co_await event.co_edit_original_response(dpp::message("Failed to fetch LeetCode profile. Please try again later."));
				co_return;
			}

			bool has_verification = desc->find("LeetBot->[" + username + "]->Verify") != std::string::npos;

			if (!has_verification) {
				co_await event.co_edit_original_response(dpp::message("Verification not found in profile. Please add: LeetBot->[" + username + "]->Verify"));
				co_return;
			}

			bool success = db.registerUser(std::to_string(user_id), username, leetcode_user);

			if (success) co_await event.co_edit_original_response(dpp::message("Registered successfully!"));
			else co_await event.co_edit_original_response(dpp::message("Registration failed (user may already exist"));
			co_return;
		}
		catch (const dpp::cache_exception& e) {
			std::cerr << "Cache exception: " << e.what() << "\n";
		}
		co_await event.co_edit_original_response(dpp::message("Error occurred with registration."));
		co_return;
	}
	else {
		co_await event.co_edit_original_response(dpp::message("No subcommand with that name found."));
		co_return;
	}
}