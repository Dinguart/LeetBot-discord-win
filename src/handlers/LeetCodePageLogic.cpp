#include "../../include/LeetCodePageLogic.h"

std::string build_solutions_page(std::vector<Solution> solutions, int currentPage)
{
	int per_page = 1;

	std::string content;
	int start = currentPage * per_page;
	int end = std::min(static_cast<int>(solutions.size()), start + per_page);
	// test what the page is outputting cuz it keeps sending a new embed.
	for (int i = start; i < end; ++i) {
		auto& [title, solContent, status, voteCount, id, creationDate] = solutions[i];
		content += std::to_string(i + 1) + ". Title: " + title + ", id: " + std::to_string(id) + ", status: " + status + ", created at: " + std::to_string(creationDate) + ", vote count: " + std::to_string(voteCount) + ", solution: " + solContent + "\n";
	}
	content += "\nPage " + std::to_string(currentPage + 1) + " / " + std::to_string((solutions.size() + per_page - 1) / per_page);
	return content;
}