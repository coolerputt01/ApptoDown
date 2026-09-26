#include <string>
#include <pqxx/pqxx>
#include <iostream>
#include <bcrypt/bcrypt.h>
#include "models/User.hpp"
#include "config.hpp"

crow::response signup(const crow::request& req,DB& db) {
    auto body = crow::json::load(req.body);

    if (!body) {
        return crow::response(400, "Invalid JSON");
    }

    if (!body.has("username") ||
        !body.has("email") ||
        !body.has("password")) {

        return crow::response(400, "Missing required fields");
    }

    const std::string username = body["username"].s();
    const std::string email = body["email"].s();
    const std::string password = body["password"].s();

    if (bcrypt_gensalt(12, Settings.salt) != 0) {
        throw std::runtime_error("Failed to generate password salt.");
    }
    if (bcrypt_hashpw(password.c_str(), Settings.salt, Settings.hash) != 0) {
        throw std::runtime_error("Failed to hash password.");
    }

    try {
        User registered_user(username, email, std::string(hash));

        pqxx::work txn(db.pqxx_conn);

        txn.exec(
            "INSERT INTO users (username, email, password) "
            "VALUES ($1, $2, $3)",

            pqxx::params{
                registered_user.get_username(),
                registered_user.get_email(),
                registered_user.get_password()
            }
        );

        txn.commit();

        crow::json::wvalue response;
        response["message"] = "User registered successfully";

        return crow::response(201, response);
    }
    catch (const std::exception& e) {
        crow::json::wvalue response;
        response["error"] = e.what();

        return crow::response(500, response);
    }
}