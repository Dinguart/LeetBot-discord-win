#include "../../include/LeetCodeAPI.h"

void fetchLeetCodeDescription(dpp::cluster* bot, const std::string& username, const dpp::slashcommand_t& event, std::function<void(std::optional<std::string>)> callback, int retry_count, std::shared_ptr<std::promise<std::optional<std::string>>> retry_promise) {
    if (!retry_promise) {
        retry_promise = std::make_shared<std::promise<std::optional<std::string>>>();
    }

    nlohmann::json body = {
        {"query", R"(
            query getUserProfile($username: String!) {
                matchedUser(username: $username) {
                    profile {
                        aboutMe
                    }
                }
            }
        )"},
        {"variables", {
            {"username", username}
        }}
    };

    std::string query = body.dump();
    
    bot->request(
        "https://leetcode.com/graphql",
        dpp::m_post,
        [event, username, callback, retry_count, bot, retry_promise](const dpp::http_request_completion_t& cc) {

        if (cc.body.size() > NetworkLimits::MAX_RESPONSE_BYTES) {
            callback(std::nullopt);
            return; // end early cuz that too much response (currently 100kb)
        }

        if (cc.status == 200) {
            try {
                // parse json response
                if (!nlohmann::json::accept(cc.body)) {
                    callback(std::nullopt);
                    return;
                }

                nlohmann::json response = nlohmann::json::parse(cc.body);

                // check if the user exists
                if (response["data"].is_null() || response["data"]["matchedUser"].is_null()) {
                    event.edit_response("User `" + username + "` not found on LeetCode.");
                    callback(std::nullopt);
                }
                auto& matchedUser = response["data"]["matchedUser"];
                std::string leetcode_username = matchedUser["username"].is_null() ?
                    "Unknown" : matchedUser["username"].get<std::string>();

                // Check if profile and about me exists
                if (matchedUser["profile"].is_null() ||
                    matchedUser["profile"]["aboutMe"].is_null()) {
                    callback(std::nullopt);
                }
                else {
                    std::string userDescription = matchedUser["profile"]["aboutMe"].get<std::string>();
                    callback(userDescription);
                }

            }
            catch (const std::exception& e) {
                std::cerr << "Error with getting request (description): " << e.what() << "\n";
                callback(std::nullopt);
            }
        }
        else if ((cc.status == 429 || cc.status == 499) && retry_count < 3) {
            std::thread([=]() {
                std::cerr << "Retry attempt #" << retry_count << " for HTTP " << cc.status << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(1 + retry_count));
                fetchLeetCodeDescription(bot, username, event, callback, retry_count + 1, retry_promise);
                }).detach();
        }
        else {
            std::cerr << "Failure to get description request: HTTP " << std::to_string(cc.status) << "\n";
            callback(std::nullopt);
        }
    },
        query,
        "application/json",
        {
            {"Content-Type", "application/json"},
            {"Accept", "*/*"},
            {"Referer", "https://leetcode.com/" + username + "/"},
            {"Origin", "https://leetcode.com"},
            {"User-Agent", "Mozilla/5.0"}
        }
    );

}

void fetchLeetCodeProfile(dpp::cluster* bot, const std::string& username, const std::string& discord_username, const std::string& discord_id, const dpp::slashcommand_t& event, LeetoDB& db, int retry_count) {
    // GraphQL query for LeetCode profile stats
    nlohmann::json body = {
    {"query", R"(
        query getUserProfile($username: String!) {
            matchedUser(username: $username) {
                username
                submitStats: submitStatsGlobal {
                    acSubmissionNum {
                        difficulty
                        count
                        submissions
                    }
                }
                profileInfo: profile {
                    ranking
                    userAvatar
                    realName
                    countryName
                    skillTags
                }
            }
        }
    )"},
    {"variables", {
        {"username", username}
    }}
    };


    std::string query = body.dump();

    bot->request(
        "https://leetcode.com/graphql",
        dpp::m_post,
        [event, username, discord_username, discord_id, bot, &retry_count, &db](const dpp::http_request_completion_t& cc) {
            if (cc.status == 200) {
                try {
                    // parse json response
                    nlohmann::json response = nlohmann::json::parse(cc.body);

                    // check if the user exists
                    if (response["data"].is_null() || response["data"]["matchedUser"].is_null()) {
                        event.edit_response("User `" + username + "` not found on LeetCode.");
                        return;
                    }
                    auto& matchedUser = response["data"]["matchedUser"];
                    std::string leetcode_username = matchedUser["username"].is_null() ?
                        "Unknown" : matchedUser["username"].get<std::string>();

                    // Check if stats exist
                    if (matchedUser["submitStats"].is_null() ||
                        matchedUser["submitStats"]["acSubmissionNum"].is_null()) {
                        event.edit_response("No submission stats available for " + username);
                        return;
                    }          

                    if (matchedUser["profileInfo"].is_null()) {
                        event.edit_response("No profile info available for " + username);
                        return;
                    }
                    int total_submissions = 0;
                    int ranking = matchedUser["profileInfo"]["ranking"].is_null() ?
                        -1 : matchedUser["profileInfo"]["ranking"].get<int>();
                    std::string user_avatar = matchedUser["profileInfo"]["userAvatar"].is_null() ?
                        "Unknown" : matchedUser["profileInfo"]["userAvatar"].get<std::string>();
                    std::string real_name = matchedUser["profileInfo"]["realName"].is_null() ?
                        "Unknown" : matchedUser["profileInfo"]["realName"].get<std::string>();
                    std::string country_name = matchedUser["profileInfo"]["countryName"].is_null() ?
                        "Unknown" : matchedUser["profileInfo"]["countryName"].get<std::string>();
                    std::string skill_stats = "None";

                    if (!matchedUser["profileInfo"]["skillTags"].empty()) {
                        skill_stats.clear();

                        for (const auto& tag : matchedUser["profileInfo"]["skillTags"]) {
                            if (!skill_stats.empty()) skill_stats += ", ";
                            skill_stats += tag.get<std::string>();
                        }
                    }

                    dpp::embed embed;
                    embed.set_title(leetcode_username + "'s LeetCode Stats")
                        .set_color(dpp::colors::orange_red);

                    UserProfile profile{};

                    for (auto& stat : matchedUser["submitStats"]["acSubmissionNum"]) {
                        if (stat["difficulty"].is_null() || stat["count"].is_null() || stat["submissions"].is_null()) continue;

                        std::string difficulty = stat["difficulty"].get<std::string>();
                        int count = stat["count"].get<int>();
                        if (stat["difficulty"].get<std::string>() == "All") {
                            total_submissions = stat["submissions"].get<int>();
                        }

                        if (difficulty == "Easy") profile.easy_solved = count;
                        else if (difficulty == "Medium") profile.medium_solved = count;
                        else if (difficulty == "Hard") profile.hard_solved = count;

                        if (!difficulty.empty()) {
                            difficulty[0] = toupper(difficulty[0]);
                            for (size_t i = 1; i < difficulty.size(); ++i) {
                                difficulty[i] = tolower(difficulty[i]);
                            }
                        }

                        embed.add_field(
                            difficulty,
                            std::to_string(count) + " solved",
                            true
                        );
                    }
                    embed.add_field("Ranking", std::to_string(ranking))
                        .add_field("Name", real_name)
                        .add_field("Country", country_name)
                        .add_field("Skills", skill_stats)
                        .add_field("Total accepted submissions", std::to_string(total_submissions))
                        .set_thumbnail(user_avatar);


                    // update table.
                    profile.ranking = ranking;
                    profile.submissions = total_submissions;
                    profile.real_name = real_name;
                    profile.country_name = country_name;
                    profile.skill_tags = skill_stats;
                    profile.avatar_url = user_avatar;
                    profile.total_solved = profile.easy_solved + profile.medium_solved + profile.hard_solved;
                    bool update = db.updateProfile(discord_id, discord_username, profile.easy_solved, profile.medium_solved, profile.hard_solved, total_submissions, username, profile.total_solved, ranking, real_name, country_name, skill_stats, user_avatar);

                    if (!update) event.edit_response("Update failure.");

                    event.edit_response(dpp::message(event.command.channel_id, embed));

                }
                catch (const std::exception& e) {
                    event.edit_response("Error parsing LeetCode response: " + std::string(e.what()));
                }
            }
            else if (cc.status == 429 && retry_count < 3) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                fetchLeetCodeProfile(bot, username, discord_username, discord_id, event, db, retry_count + 1);
            }
            else {
                event.edit_response("Failed to fetch LeetCode profile (HTTP " + std::to_string(cc.status) + ")");
            }
        },
        query,
        "application/json",
        {
            {"Content-Type", "application/json"},
            {"Accept", "*/*"},
            {"Referer", "https://leetcode.com/" + username + "/"},
            {"Origin", "https://leetcode.com"},
            {"User-Agent", "Mozilla/5.0"}
        }
    );
}

void fetchLeetCodeDailyQuestion(dpp::cluster* bot, const std::string& username, const dpp::slashcommand_t& event, LeetoDB& db, int retry_count)
{
    nlohmann::json body = {
    {"query", R"(
        query getCombined($username: String!, $limit: Int!) {
            daily: activeDailyCodingChallengeQuestion {
                date
                link
                question {
                    acRate
                    difficulty
                    questionFrontendId
                    isPaidOnly
                    title
                    titleSlug
                    topicTags {
                        name
                        slug
                    }
                }
            }
            recent: recentAcSubmissionList(username: $username, limit: $limit) {
                id
                title
                timestamp
            }
        }
    )"},
    {"variables", {
        {"username", username},
        {"limit", 50}
    }}
    };

    auto auth = readAuthFile("include/cookies.env");
    std::string session_cookie = auth["LEETCODE_SESSION"];
    std::string csrf_token = auth["csrftoken"];
    std::string query = body.dump();

    bot->request(
        "https://leetcode.com/graphql",
        dpp::m_post,
        [event, bot, username, &retry_count, &db](const dpp::http_request_completion_t& cc) {
            if (cc.status == 200) {
                try {
                    nlohmann::json response = nlohmann::json::parse(cc.body);

                    if (response["data"].is_null() ||
                        response["data"]["daily"].is_null() ||
                        response["data"]["daily"]["question"].is_null()) {
                        std::cerr << "Malformed API response structure:\n" << response.dump(2) << "\n";
                        event.edit_original_response(dpp::message("Daily question data missing in response"));
                        return;
                    }

                    if (response["data"]["recent"].is_null() || response["data"]["recent"].empty()) {
                        event.edit_original_response(dpp::message("Could not get user's recent submissions."));
                        return;
                    }

                    auto& daily = response["data"]["daily"];
                    auto& recent = response["data"]["recent"];
                    // daily
                    std::string date, link;
                    date = daily["date"].get<std::string>();
                    link = daily["link"].get<std::string>();

                    auto& question = daily["question"];

                    std::string difficulty, frontendQuestionId, title, titleSlug;
                    bool paidOnly{ false };
                    double acRate = 0.0;

                    acRate = question["acRate"].is_null() ? 0.0
                        : question["acRate"].get<double>();
                    difficulty = question["difficulty"].is_null() ? "Unknown"
                        : question["difficulty"].get<std::string>();
                    frontendQuestionId = question["questionFrontendId"].is_null() ? "Unknown"
                        : question["questionFrontendId"].get<std::string>();
                    title = question["title"].is_null() ? "Unknown"
                        : question["title"].get<std::string>();
                    titleSlug = question["titleSlug"].is_null() ? "Unknown"
                        : question["titleSlug"].get<std::string>();
                    paidOnly = question["isPaidOnly"].get<bool>();

                    std::string topicTags{ "None" };
                    std::string topicSlugTags{ "None" };

                    if (!question["topicTags"].is_null() && !question["topicTags"].empty()) {
                        topicTags.clear();
                        topicSlugTags.clear();

                        for (const auto& tag : question["topicTags"]) {
                            if (!topicTags.empty()) topicTags += ", ";
                            topicTags += tag["name"].get<std::string>();
                            if (!topicSlugTags.empty()) topicSlugTags += "|";
                            topicSlugTags += tag["slug"].get<std::string>();
                        }
                    }

                    std::string isPaid = !paidOnly ? "Not paid only" : "Paid only";

                    // recent
                    std::string completedDaily = "Not recently completed ";
                    completedDaily += dpp::unicode_emoji::cross_mark;
                    for (const auto& item : recent) {
                        if (item["id"].is_null() || item["title"].is_null() || item["timestamp"].is_null()) continue;

                        std::string qTitle = item["title"];
                        if (qTitle == title) {
                            completedDaily.clear();
                            completedDaily = "Completed ";
                            completedDaily += dpp::unicode_emoji::white_check_mark;
                        }
                    }

                    dpp::embed embed;
                    dpp::message msg;
                    embed.set_title("Daily Question | id -> " + frontendQuestionId + ": " + title)
                        .set_color(dpp::colors::mango_orange)
                        .set_description(topicTags)
                        .add_field("Date", date)
                        .add_field("Difficulty", difficulty)
                        .add_field("Acceptance Rate", std::to_string(acRate) + "%")
                        .add_field("Paid only?", isPaid)
                        .set_thumbnail("https://upload.wikimedia.org/wikipedia/commons/1/19/LeetCode_logo_black.png")
                        .add_field("Progress", completedDaily)
                        .add_field("URL", "leetcode.com" + link);

                    msg.add_embed(embed);

                    dpp::component dropdown = dpp::component()
                        .set_type(dpp::cot_selectmenu)
                        .set_placeholder("Select solution language")
                        .set_id("solution_language_dropdown");

                    std::vector<std::string> languages = {
                        "C++", "Java", "Python", "Python3", "C", "C#",
                        "JavaScript", "TypeScript", "PHP", "Swift", "Kotlin",
                        "Dart", "Go", "Ruby", "Scala", "Rust", "Racket",
                        "Erlang", "Elixir"
                    };

                    for (const auto& language : languages) {
                        dropdown.add_select_option(dpp::select_option(language, language));
                    }

                    msg.add_component(
                        dpp::component()
                        .add_component(dpp::component()
                            .set_label("Solution")
                            .set_style(dpp::cos_primary)
                            .set_type(dpp::cot_button)
                            .set_id("solution_id:" + titleSlug)
                        )
                        .add_component(dpp::component()
                            .set_label("Description")
                            .set_style(dpp::cos_primary)
                            .set_type(dpp::cot_button)
                            .set_id("description_id:" + titleSlug)
                        )
                    );

                    msg.add_component(
                        dpp::component().add_component(dropdown)
                    );

                    event.edit_original_response(msg);
                }
                catch (std::exception& e) {
                    event.edit_original_response(dpp::message("Error parsing LeetCode response: " + std::string(e.what())));
                }
            }
            else if ((cc.status == 429 && retry_count < 3) || (cc.status == 499 && retry_count < 3)) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                fetchLeetCodeDailyQuestion(bot, username, event, db, retry_count + 1);
            }
            else {
                event.edit_original_response(dpp::message("Failed to fetch LeetCode daily question (HTTP " + std::to_string(cc.status) + ": " + cc.body));
            }
        },
        query,
        "application/json",
        {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"},
            {"Referer", "https://leetcode.com/problemset/all/"},
            {"Origin", "https://leetcode.com"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"},
            {"Cookie", "LEETCODE_SESSION=" + session_cookie + "; csrftoken=" + csrf_token},
            {"X-CSRFToken", csrf_token},
            {"X-Requested-With", "XMLHttpRequest"}
        }
    );
}

void fetchLeetCodeActivity(dpp::cluster* bot, const std::string& username, const dpp::slashcommand_t& event, int retry_count)
{
    nlohmann::json body = {
    {"query", R"(
        query getUserProfile($username: String!) {
            matchedUser(username: $username) {
                activity : userCalendar {
                    streak
                    totalActiveDays
                }
                profileInfo: profile {
                    userAvatar
                }
            }
        }
    )"},
    {"variables", {
        {"username", username}
    }}
    };

    std::string query = body.dump();

    bot->request(
        "https://leetcode.com/graphql",
        dpp::m_post,
        [&bot, username, event, &retry_count](const dpp::http_request_completion_t& cc) {
            if (cc.status == 200) {
                try {
                    nlohmann::json response = nlohmann::json::parse(cc.body);

                    if (response["data"].is_null() || response["data"]["matchedUser"].is_null()) {
                        event.edit_original_response(dpp::message("Failed to get user activity."));
                        return;
                    }

                    auto& matchedUser = response["data"]["matchedUser"];

                    if (matchedUser["activity"].is_null()) {
                        event.edit_original_response(dpp::message("Could not retrieve user activity"));
                        return;
                    }

                    auto& activity = matchedUser["activity"];

                    int streak = activity["streak"].get<int>();
                    int totalActiveDays = activity["totalActiveDays"].get<int>();

                    if (matchedUser["profileInfo"].is_null()) {
                        event.edit_original_response(dpp::message("Could not retrieve user profile."));
                        return;
                    }

                    auto& profile = matchedUser["profileInfo"];

                    std::string avatarUrl = profile["userAvatar"].is_null() ? "Unknown"
                        : profile["userAvatar"].get<std::string>();

                    dpp::embed embed;
                    embed.set_title(username + "'s activity")
                        .add_field("Daily active streak", std::to_string(streak))
                        .add_field("Total active days", std::to_string(totalActiveDays))
                        .set_thumbnail(avatarUrl)
                        .set_color(dpp::colors::orange_salmon)
                        .set_timestamp(time(0));

                    event.edit_original_response(dpp::message(event.command.channel_id, embed));
                }
                catch (std::exception& e) {
                    event.edit_original_response(dpp::message("failed fetching user activity: " + std::string(e.what())));
                }
            }
            else if ((cc.status == 429 && retry_count < 3) || (cc.status == 499 && retry_count < 3)) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                fetchLeetCodeActivity(bot, username, event, retry_count);
            }
            else {
                event.edit_original_response(dpp::message("Failed to fetch LeetCode activity (HTTP " + std::to_string(cc.status) + ": " + cc.body));
            }
        },
        query,
        "application/json",
        {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"},
            {"Referer", "https://leetcode.com/problemset/all/"},
            {"Origin", "https://leetcode.com"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"},
            {"X-Requested-With", "XMLHttpRequest"}
        }
    );
}

void fetchLeetCodeDailyQuestionSolution(dpp::cluster* bot, const std::string& titleSlug, const std::string& languageSlug, const dpp::button_click_t& event, int retry_count) {
    nlohmann::json body = {
    {"query", R"(
        query getSolutionPosts($titleSlug: String!, $skip: Int!, $languageTags: [String!]) {
            questionSolutions(
                filters: {
                    questionSlug: $titleSlug,
                    languageTags: $languageTags,
                    first: 5,
                    skip: $skip
                }
            ) {
                solutions {
                    title
                    post { 
                        id
                        status
                        creationDate
                        voteCount
                        content
                    }
                }
            }
        }
    )"},
    {"variables", {
        {"titleSlug", titleSlug},
        {"languageTags", languageSlug},
        {"skip", 1}
    }}
    };

    std::string query = body.dump();

    bot->request(
        "https://leetcode.com/graphql",
        dpp::m_post,
        [&bot, event, &retry_count, titleSlug, languageSlug](const dpp::http_request_completion_t& cc) {
            if (cc.status == 200) {
                dpp::snowflake userId = event.command.usr.id;
                try {
                    nlohmann::json response = nlohmann::json::parse(cc.body);

                    if (response["data"].is_null()) {
                        event.edit_original_response(dpp::message("Failed to get request."));
                        return;
                    }

                    if (response["data"]["questionSolutions"]["solutions"].is_null()) {
                        event.edit_original_response(dpp::message("Failed to get solutions."));
                        return;
                    }

                    auto solutions = response["data"]["questionSolutions"]["solutions"];

                    if (solutions.empty()) {
                        event.edit_original_response(dpp::message("This problem has no solutions for the selected language."));
                        return;
                    }

                    std::vector<Solution> solutionVector;

                    // its just gonna be the first 5 but whatever
                    for (const auto& sol : solutions) {
                        Solution solution;
                        solution.title = sol["title"].is_null() ? ""
                            : sol["title"].get<std::string>();
                        if (sol["post"].is_null()) continue;
                        auto& post = sol["post"];

                        solution.id = post["id"].get<int>();
                        solution.status = post["status"].is_null() ? ""
                            : post["status"].get<std::string>();
                        solution.creationDate = post["creationDate"].get<int>();
                        solution.voteCount = post["voteCount"].get<int>();
                        std::string content = post["content"].is_null() ? ""
                            : post["content"].get<std::string>();
                        solution.content = removeNewLines(content, languageSlug);
                        solutionVector.push_back(solution);
                    }

                    userSolutionPages[userId] = solutionVector;
                    solutionCurrentPage[userId] = 0;

                    int currentPage = 0;
                    std::string pageContent = build_solutions_page(solutionVector, currentPage);

                    dpp::embed embed;
                    embed.set_title("Solutions")
                        .set_color(dpp::colors::yellow_orange)
                        .set_description(pageContent)
                        .set_timestamp(time(0));

                    dpp::message msg;
                    msg.add_embed(embed);

                    msg.add_component(
                        dpp::component().add_component(
                            dpp::component()
                            .set_label(dpp::unicode_emoji::arrow_left)
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_primary)
                            .set_id("solution_prev_page:" + std::to_string(userId))
                        ).add_component(
                            dpp::component()
                            .set_label(dpp::unicode_emoji::arrow_right)
                            .set_type(dpp::cot_button)
                            .set_style(dpp::cos_primary)
                            .set_id("solution_next_page:" + std::to_string(userId))
                        )
                    );

                    event.edit_original_response(msg);
                    return;
                }
                catch (std::exception& e) {
                    std::string error = e.what();
                    event.edit_original_response(dpp::message("Error parsing response: " + error));
                }
            }
            else if ((cc.status == 429 && retry_count < 3) || (cc.status == 499 && retry_count < 3)) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                fetchLeetCodeDailyQuestionSolution(bot, titleSlug, languageSlug, event, retry_count);
            }
            else {
                event.edit_original_response(dpp::message("Failed to fetch LeetCode daily question solutions (HTTP " + std::to_string(cc.status) + ": " + cc.body));
            }
        },
        query,
        "application/json",
        {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"},
            {"Referer", "https://leetcode.com/problemset/all/"},
            {"Origin", "https://leetcode.com"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"},
            {"X-Requested-With", "XMLHttpRequest"}
        }
    );
}

void fetchLeetCodeDailyQuestionDescription(dpp::cluster* bot, const std::string& titleSlug, const dpp::button_click_t& event, int retry_count) {
    nlohmann::json body = {
    {"query", R"(
        query questionContent($titleSlug: String!) {
            problem : question(titleSlug: $titleSlug) {
                content
                mysqlSchemas
            }
        }
    )"},
    {"variables", {
        {"titleSlug", titleSlug}
    }}
    };

    std::string query = body.dump();


    bot->request(
        "https://leetcode.com/graphql",
        dpp::m_post,
        [event, bot, titleSlug, &retry_count](const dpp::http_request_completion_t& cc) {
            if (cc.status == 200) {
                try {
                    nlohmann::json response = nlohmann::json::parse(cc.body);

                    if (response["data"].is_null()) {
                        event.edit_original_response(dpp::message("Error parsing response."));
                        return;
                    }

                    if (response["data"]["problem"].is_null()) {
                        event.edit_original_response(dpp::message("Could not get LeetCode problem."));
                        return;
                    }

                    auto& problem = response["data"]["problem"];

                    std::string content = problem["content"].is_null() ? "No content available."
                        : problem["content"].get<std::string>();

                    auto& schemas = problem["mysqlSchemas"];

                    dpp::embed embed;
                    embed.set_title("Problem Description")
                        .set_description(htmlDecoder(htmlToMarkdown(content)))
                        .set_color(dpp::colors::bright_orange)
                        .set_timestamp(time(0));

                    if (!schemas.empty()) {
                        embed.add_field("mysql Schemas", "");
                        for (const auto& schema : schemas) {
                            embed.add_field(schema, "");
                        }
                    }

                    dpp::message msg;
                    msg.add_embed(embed);

                    event.edit_original_response(msg);
                    return;
                }
                catch (const std::exception& e) {
                    event.edit_original_response(dpp::message("Error parsing LeetCode response"));
                }
            }
            else if ((cc.status == 429 && retry_count < 3) || (cc.status == 499 && retry_count < 3)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                fetchLeetCodeDailyQuestionDescription(bot, titleSlug, event, retry_count);
            }
            else {
                event.edit_original_response(dpp::message("Failed to fetch LeetCode daily question (HTTP " + std::to_string(cc.status) + ": " + cc.body));
            }
        },
        query,
        "application/json",
        {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"},
            {"Referer", "https://leetcode.com/problemset/all/"},
            {"Origin", "https://leetcode.com"},
            {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36"},
            {"X-Requested-With", "XMLHttpRequest"}
        }
    );
}
