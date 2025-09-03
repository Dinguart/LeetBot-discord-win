#pragma once

#include <regex>
#include <string>
#include <algorithm>

std::string htmlToMarkdown(const std::string& html);

std::string removeNewLines(const std::string& text, const std::string& languageSlug);

std::string htmlDecoder(const std::string& text);