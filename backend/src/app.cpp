
#include <crow.h>
#include <pqxx/pqxx>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

class Database {
public:
    explicit Database(const std::string& connection_string)
        : connection_string_(connection_string) {}

    pqxx::result get_users() {
        pqxx::connection connection(connection_string_);
        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec(
            "SELECT id, name, email "
            "FROM users "
            "ORDER BY id"
        );

        transaction.commit();

        return result;
    }

    pqxx::result get_user_by_id(int id) {
        pqxx::connection connection(connection_string_);
        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec_params(
            "SELECT id, name, email "
            "FROM users "
            "WHERE id = $1",
            id
        );

        transaction.commit();

        return result;
    }

private:
    std::string connection_string_;
};

int main() {
    const char* database_url = std::getenv("DATABASE_URL");

    if (database_url == nullptr) {
        std::cerr << "DATABASE_URL is not set\n";
        return 1;
    }

    Database database(database_url);

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([] {
        crow::json::wvalue response;
        response["status"] = "ok";
        response["message"] = "Crow API is running";
        return response;
    });

    CROW_ROUTE(app, "/users")
    ([&database] {
        crow::json::wvalue response;
        crow::json::wvalue::list users;

        try {
            pqxx::result result = database.get_users();

            for (const auto& row : result) {
                crow::json::wvalue user;

                user["id"] = row["id"].as<int>();
                user["name"] = row["name"].c_str();
                user["email"] = row["email"].c_str();

                users.push_back(std::move(user));
            }

            response["users"] = std::move(users);
            response["count"] = static_cast<int>(result.size());

            return crow::response(200, response);
        }
        catch (const std::exception& error) {
            std::cerr << "Database error: " << error.what() << '\n';

            crow::json::wvalue error_response;
            error_response["error"] = "Database error";

            return crow::response(500, error_response);
        }
    });

    CROW_ROUTE(app, "/users/<int>")
    ([&database](int id) {
        crow::json::wvalue response;

        try {
            pqxx::result result = database.get_user_by_id(id);

            if (result.empty()) {
                response["error"] = "User not found";
                return crow::response(404, response);
            }

            const auto& row = result[0];

            response["id"] = row["id"].as<int>();
            response["name"] = row["name"].c_str();
            response["email"] = row["email"].c_str();

            return crow::response(200, response);
        }
        catch (const std::exception& error) {
            std::cerr << "Database error: " << error.what() << '\n';

            response["error"] = "Database error";
            return crow::response(500, response);
        }
    });

    app.port(8080).multithreaded().run();
}