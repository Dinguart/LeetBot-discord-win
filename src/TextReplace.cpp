#include "../include/TextReplace.h"

std::string htmlToMarkdown(const std::string& html) {
	std::string out = html;
    out = std::regex_replace(out, std::regex("<code>"), "`");
    out = std::regex_replace(out, std::regex("</code>"), "`");
    out = std::regex_replace(out, std::regex("<pre>"), "```");
    out = std::regex_replace(out, std::regex("</pre>"), "```");
    out = std::regex_replace(out, std::regex("<[^>]*>"), "");

    return out;
}

std::string removeNewLines(const std::string& text, const std::string& languageSlug)
{
    std::string out = text;


    //out.replace(std::remove_if(out.begin(), out.end(),
    //    [](char c) { return c == '\n' || c == '\r'; }),
    //    out.end());
    //
    //size_t pos;
    //while ((pos = out.find("\\n")) != std::string::npos) {
    //    out.erase(pos, 2);
    //}

    out = std::regex_replace(out, std::regex("\\\\n"), "\n");
    out = std::regex_replace(out, std::regex("\\\\r"), "\r");
    out = std::regex_replace(out, std::regex("\\\\t"), "\t");
    out = std::regex_replace(out, std::regex("```(?!\\w)"), "```" + languageSlug);
    //out = std::regex_replace(out, std::regex("[^>]*"), "");

    return out;
}

std::string htmlDecoder(const std::string& text) {
    std::string out;
    out.reserve(text.size());

    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '&') {
            size_t sec = text.find(';', i);
            if (sec != std::string::npos) {
                std::string code = text.substr(i, sec - i + 1);
                if (code == "&lt;") { out += '<'; i = sec; continue; }
                if (code == "&gt;") { out += '>'; i = sec; continue; }
                if (code == "&amp;") { out += '&'; i = sec;  continue; }
                if (code == "&quot;") { out += '"'; i = sec; continue; }
                if (code == "&#39;") { out += '\''; i = sec; continue; }
                if (code == "&nbsp;") { out += " "; i = sec; continue; }
            }
        }
        out += text[i];
    }
    return out;
}