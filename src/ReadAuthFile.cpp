#include "../include/ReadAuthFile.h"

std::unordered_map<std::string, std::string> readAuthFile(const std::string& filename) {
	std::unordered_map<std::string, std::string> config;

	std::ifstream file(filename);

	std::string line;

	while (std::getline(file, line)) {
		auto equalPos = line.find('=');
		if (equalPos != std::string::npos) {
			std::string key = line.substr(0, equalPos);
			std::string value = line.substr(equalPos + 1);
			config[key] = value;
		}
	}

	return config;
}