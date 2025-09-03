#pragma once
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <mysql_driver.h>

#include <iostream>
#include <string>
#include <fstream>
#include <optional>

struct UserProfile {
	int easy_solved, medium_solved, hard_solved, total_solved, ranking, submissions;
	std::string real_name, avatar_url, country_name, skill_tags;
	// can add more in future just refactor this struct.
};

class LeetoDB {
public:
	LeetoDB(const std::string& host, const std::string& user, const std::string& password, const std::string& dbName);
	~LeetoDB();

	bool connect();
	void disconnect();
	bool isConnected() const;

	// get stuff
	std::optional<std::string> getLeetcodeUsername(const std::string& discord_id);
	std::optional<UserProfile> getLeetcodeProfile(const std::string& discord_username);
	std::optional<std::string> getLeetcodeAvatar(const std::string& discord_id);

	// register method
	bool registerUser(const std::string& discord_id, const std::string& discord_user, const std::string& leetcode_user);


	// existence methods (check if in db)
	bool statsExist(const std::string& discord_username);

	// update shit (usin discord id cuz user/leetcode user can be changed easily
	bool updateProfile(const std::string& discord_id, const std::string& discord_username, int easy_solved, int medium_solved, int hard_solved, int submissions, const std::string& leetcode_username, int total_solved, int ranking, const std::string& real_name, const std::string& country_name, const std::string& skill_tags, const std::string& avatar_url);
private:
	sql::Connection* con;
	std::string host;
	std::string user;
	std::string password;
	std::string dbName;
	bool connected;
};