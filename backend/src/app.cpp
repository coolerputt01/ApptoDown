
#include <crow.h>
#include "config.hpp"
#include "utility/responseSetter.hpp"
#include "controllers/users/signup.hpp"
#include <iostream>

int main() {
    std::string db_conn_string = "postgresql://admin_user:admin_password@127.0.0.1:5432/apptodown?sslmode=disable";

    static DB db(db_conn_string);
    std::cout<<db.pqxx_conn.dbname()<<std::endl;
    App software;
    

    CROW_ROUTE(software.app, "/") ([] {
        crow::json::wvalue response;
        setResponse(response,"ok","Welcome to App to Down API");
        return response;
    });

    CROW_ROUTE(software.app, "/signup").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        return signup(req,db);
    });

    software.run();
}