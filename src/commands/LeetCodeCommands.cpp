#include "../../include/LeetCodeCommands.h"

dpp::task<void> handle_leetcode_commands(dpp::cluster& bot, const dpp::slashcommand_t& event, LeetoDB& db) {
	co_await event.co_thinking();
	
	dpp::snowflake guild_id = event.command.guild_id;
	dpp::snowflake user_id = event.command.get_issuing_user().id;
	dpp::guild_member member = dpp::find_guild_member(guild_id, user_id);
	dpp::user* username = member.get_user();
	std::string usernameString = username->username;
	std::string id = std::to_string(user_id);

	const dpp::command_interaction& cmd_data = event.command.get_command_interaction();

	if (cmd_data.options.empty()) {
		co_await event.co_edit_original_response(dpp::message("No subcommand provided."));
		co_return;
	}

	const dpp::command_data_option& subcommand = cmd_data.options[0];

	if (subcommand.name == "profile") {
		std::string leetcode_username = db.getLeetcodeUsername(std::to_string(user_id)).value_or("");

		if (leetcode_username.empty()) {
			co_await event.co_edit_original_response(dpp::message("Failed to get your leetcode username, remember to register with the bot first!"));
			co_return;
		}

		co_await event.co_edit_original_response(dpp::message("Fetching your LeetCode Profile..."));

		if (db.statsExist(usernameString)) {
			// then dont do fetch just get it.
			std::optional<UserProfile> optionalProfile = db.getLeetcodeProfile(usernameString);
			if (!optionalProfile.has_value()) {
				co_await event.co_edit_original_response(dpp::message("Error occurred in getting user profile. Consult someone who works on this to fix."));
				co_return;
			}

			dpp::embed embed;
			dpp::message msg;
			UserProfile profile = optionalProfile.value();
			int easySolved, mediumSolved, hardSolved, totalSolved, ranking, submissions;
			std::string realName, countryName, skillTags, avatarUrl;
			easySolved = profile.easy_solved;
			mediumSolved = profile.medium_solved;
			hardSolved = profile.hard_solved;
			submissions = profile.submissions;
			totalSolved = profile.total_solved;
			ranking = profile.ranking;
			realName = profile.real_name;
			countryName = profile.country_name;
			skillTags = profile.skill_tags;
			avatarUrl = profile.avatar_url;

			//std::string url = "leetcode.com/u/" + leetcode_username + "/";
			// also add the "added to database or whatever" to the fetch function
			// to let user know

			embed.set_title(leetcode_username + "'s Leetcode profile")
				.set_description(realName + "'s Profile!")
				.set_thumbnail(avatarUrl)
				.add_field("Country", countryName)
				.add_field("Easy solved", std::to_string(easySolved))
				.add_field("Medium solved", std::to_string(mediumSolved))
				.add_field("Hard solved", std::to_string(hardSolved))
				.add_field("Total solved", std::to_string(totalSolved))
				.add_field("Total accepted submissions", std::to_string(submissions))
				.add_field("Ranking", std::to_string(ranking), true)
				.add_field("Skills", skillTags)
				.set_color(dpp::colors::orange_gold)
				.set_timestamp(time(0));

			msg.add_embed(embed);

			co_await event.co_edit_original_response(msg);
			co_return;
		}
		else {
			fetchLeetCodeProfile(&bot, leetcode_username, usernameString, id, event, db);
			// this will add it to their database table
		}
	}
	else if (subcommand.name == "updateprofile") {
		std::string leetcode_username = db.getLeetcodeUsername(std::to_string(user_id)).value_or("");

		if (leetcode_username.empty()) {
			co_await event.co_edit_original_response(dpp::message("Failed to get your leetcode username, remember to register with the bot first!"));
			co_return;
		}

		fetchLeetCodeProfile(&bot, leetcode_username, usernameString, id, event, db);
	}
	else if (subcommand.name == "dailyquestion") {
		try {
			std::string leetcode_username = db.getLeetcodeUsername(std::to_string(user_id)).value_or("");
			if (leetcode_username.empty()) {
				co_await event.co_edit_original_response(dpp::message("Failed to get your leetcoe username, make sure to register with the bot."));
				co_return;
			}

			fetchLeetCodeDailyQuestion(&bot, leetcode_username, event, db);
		}
		catch (const std::exception& e) {
			std::cerr << "std exception: " << e.what() << "\n";
		}
	}
	else if (subcommand.name == "activity") {
		std::string leetcodeUsername = db.getLeetcodeUsername(std::to_string(user_id)).value_or("");
		if (leetcodeUsername.empty()) {
			co_await event.co_edit_original_response(dpp::message("Failed to get your leetcode username, make sure to register with the bot."));
			co_return;
		}

		fetchLeetCodeActivity(&bot, leetcodeUsername, event);
	}
	else {
		co_await event.co_edit_original_response(dpp::message("Subcommand not found."));
		co_return;
	}
}