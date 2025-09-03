#include "../../include/LeetoDB.h"

LeetoDB::LeetoDB(const std::string& host, const std::string& user, const std::string& password, const std::string& dbName)
	: host(host), user(user), password(password), dbName(dbName), con(nullptr), connected(false) {
}

LeetoDB::~LeetoDB() {
	disconnect();
}

bool LeetoDB::connect() {
	try {
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
		con = driver->connect(host, user, password);
		con->setSchema(dbName);
		connected = true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQL error: " << e.what() << "\n";
		connected = false;
	}
	return connected;
}

void LeetoDB::disconnect() {
	if (con) {
		delete con;
		con = nullptr;
		connected = false;
	}
}

bool LeetoDB::isConnected() const {
	return connected;
}

bool LeetoDB::registerUser(const std::string& discord_id, const std::string& discord_user, const std::string& leetcode_user) {
	if (!connected) {
		std::cerr << "Not connected to the database.\n";
		return false;
	}

	try {
		//check if registered
		std::unique_ptr<sql::PreparedStatement> checkstmt(
			con->prepareStatement("SELECT COUNT(*) FROM users WHERE discord_id = ?")
		);
		checkstmt->setString(1, discord_id);
		std::unique_ptr<sql::ResultSet> res(checkstmt->executeQuery());
		if (res->next() && res->getInt(1) > 0) {
			// then user already exists
			return false;
		}

		// register
		std::unique_ptr<sql::PreparedStatement> insertstmt(
			con->prepareStatement("INSERT INTO users (discord_id, discord_username, leetcode_username) VALUES (?, ?, ?)")
		);
		insertstmt->setString(1, discord_id);
		insertstmt->setString(2, discord_user);
		insertstmt->setString(3, leetcode_user);
		insertstmt->execute();

		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Register user error: " << e.what() << "\n";
	}
	return false;
}

std::optional<std::string> LeetoDB::getLeetcodeUsername(const std::string& discord_id) {
	if (!connected) {
		std::cerr << "Not connected to database.\n";
		return std::nullopt;
	}

	std::string leetcode_username;

	try {
		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("SELECT leetcode_username FROM users WHERE discord_id = ?")
		);
		stmt->setString(1, discord_id);
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

		if (res->next()) {
			leetcode_username = res->getString("leetcode_username");
		}
	}
	catch (sql::SQLException& e) {
		std::cerr << "Error getting leetcode username: " << e.what() << "\n";
		return std::nullopt;
	}

	return leetcode_username;
}

bool LeetoDB::statsExist(const std::string& discord_username) {
	if (!connected) {
		std::cerr << "Not connected to database.\n";
		return false;
	}
	
	
	try {
		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("SELECT * FROM users WHERE discord_username = ?")
		);
		stmt->setString(1, discord_username);
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
		if (res->next()) {
			if (res->isNull("easy_solved") || res->isNull("medium_solved") || res->isNull("hard_solved") || res->isNull("total_solved")) {
				return false;
			}
		}

		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Stats check exception: " << e.what() << "\n";
		return false;
	}
}

std::optional<UserProfile> LeetoDB::getLeetcodeProfile(const std::string& discord_username) {
	if (!connected) {
		std::cerr << "Not connected to database.\n";
		return std::nullopt;
	}

	try {
		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("SELECT * FROM users WHERE discord_username = ?")
		);
		stmt->setString(1, discord_username);
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

		UserProfile profile;

		if (!res->next()) {
			return std::nullopt;
		}

		profile.easy_solved = res->getInt("easy_solved");
		profile.medium_solved = res->getInt("medium_solved");
		profile.hard_solved = res->getInt("hard_solved");
		profile.submissions = res->getInt("total_submissions");
		profile.total_solved = res->getInt("total_solved");
		profile.ranking = res->getInt("ranking");
		profile.real_name = res->getString("real_name");
		profile.country_name = res->getString("country_name");
		profile.skill_tags = res->getString("skill_tags");
		profile.avatar_url = res->getString("avatar_url");

		return profile;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Profile retrieval error: " << e.what() << "\n";
		return std::nullopt;
	}
}

std::optional<std::string> LeetoDB::getLeetcodeAvatar(const std::string& discord_id)
{
	if (!connected) {
		std::cerr << "Not connected to database!\n";
		return std::nullopt;
	}

	try {
		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("SELECT avatar_url FROM users WHERE discord_id = ?")
		);
		stmt->setString(1, discord_id);
		std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

		if (!res->next()) {
			return std::nullopt;
		}

		std::string avatarUrl = res->getString("avatar_url");

		return avatarUrl;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Avatar retrieval error: " << e.what() << "\n";
		return std::nullopt;
	}
}

bool LeetoDB::updateProfile(const std::string& discord_id, const std::string& discord_username, int easy_solved, int medium_solved, int hard_solved, int submissions, const std::string& leetcode_username, int total_solved, int ranking, const std::string& real_name, const std::string& country_name, const std::string& skill_tags, const std::string& avatar_url) {
	if (!connected) {
		std::cerr << "Not connected to the database\n";
		return false;
	}

	try {
		std::unique_ptr<sql::PreparedStatement> checkstmt(
			con->prepareStatement("SELECT * FROM users WHERE discord_id = ?")
		);
		checkstmt->setString(1, discord_id);
		std::unique_ptr<sql::ResultSet> res(checkstmt->executeQuery());
		if (!res->next()) {
			std::cerr << "User does not exist in the database\n";
			return false;
		}


		std::unique_ptr<sql::PreparedStatement> stmt(
			con->prepareStatement("UPDATE users SET discord_username = ?, easy_solved = ?, medium_solved = ?, hard_solved = ?, leetcode_username = ?, total_solved = ?, ranking = ?, real_name = ?, country_name = ?, skill_tags = ?, avatar_url = ?, total_submissions = ? WHERE discord_id = ?")
		);
		stmt->setString(1, discord_username);
		stmt->setInt(2, easy_solved);
		stmt->setInt(3, medium_solved);
		stmt->setInt(4, hard_solved);
		stmt->setString(5, leetcode_username);
		stmt->setInt(6, total_solved);
		stmt->setInt(7, ranking);
		stmt->setString(8, real_name);
		stmt->setString(9, country_name);
		stmt->setString(10, skill_tags);
		stmt->setString(11, avatar_url);
		stmt->setInt(12, submissions);
		stmt->setString(13, discord_id);
		stmt->execute();

		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "Update profile error: " << e.what() << "\n";
		return false;
	}
}