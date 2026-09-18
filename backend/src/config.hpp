#include <pqxx/pqxx>
#include <string>
#include <crow.h>

class DB {
    private:
        std::string conn_string_;
    public:
        pqxx::connection pqxx_conn;
        explicit DB(const std::string& conn) : conn_string_(conn), pqxx_conn(conn){}
};

class App {
    public:
        crow::SimpleApp app;
        void run(){
            app.port(8080).multithreaded().run();
        }
};