#pragma once
#include <dpp/dpp.h>
#include <unordered_map>
#include <vector>
#include <string>

struct Solution {
    std::string title, content, status;
    int voteCount, id, creationDate;
};

extern std::unordered_map<dpp::snowflake, std::vector<Solution>> userSolutionPages;
extern std::unordered_map<dpp::snowflake, int> solutionCurrentPage;