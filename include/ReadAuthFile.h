#pragma once
#include <fstream>
#include <sstream>
#include <unordered_map>

std::unordered_map<std::string, std::string> readAuthFile(const std::string& filename);