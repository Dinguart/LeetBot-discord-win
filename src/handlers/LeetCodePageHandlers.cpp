#include "../../include/LeetCodePageHandlers.h"

void handle_solutions_page_button(const dpp::button_click_t& event) {
	std::string id = event.custom_id;
	if (!id.starts_with("solution_prev_page:") && !id.starts_with("solution_next_page:")) return;

	dpp::snowflake user_id = std::stoull(id.substr(id.find(':') + 1));

	if (event.command.usr.id != user_id) {
		event.reply(dpp::message("These buttons are not for you.").set_flags(dpp::m_ephemeral));
		return;
	}

	auto& solutions = userSolutionPages[user_id];
	int& currentPage = solutionCurrentPage[user_id];

	if (id.starts_with("solution_prev_page:") && currentPage > 0) currentPage--;
	else if (id.starts_with("solution_next_page:") && (currentPage + 1) * 1 < solutions.size()) currentPage++;

	std::string newContent = build_solutions_page(solutions, currentPage);

	dpp::embed embed;
	embed.set_title("Solutions")
		.set_description(newContent)
		.set_color(dpp::colors::bright_gold)
		.set_timestamp(time(0));

	event.reply(dpp::ir_update_message,
		dpp::message().add_embed(embed).add_component(
		dpp::component().add_component(
			dpp::component().set_label(dpp::unicode_emoji::arrow_left).set_type(dpp::cot_button).set_style(dpp::cos_primary).set_id("solution_prev_page:" + std::to_string(user_id))
		).add_component(
			dpp::component().set_label(dpp::unicode_emoji::arrow_right).set_type(dpp::cot_button).set_style(dpp::cos_primary).set_id("solution_next_page:" + std::to_string(user_id))
		)
	));
}