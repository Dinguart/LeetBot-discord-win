#include "../include/PerUserPage.h"

std::unordered_map<dpp::snowflake, std::vector<Solution>> userSolutionPages;
std::unordered_map<dpp::snowflake, int> solutionCurrentPage;