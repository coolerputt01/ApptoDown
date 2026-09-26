#include <string>
#include <pqxx/pqxx>
#include <crow.h>
#include "../models.User.hpp"

void signin(const std::string& username,const std::string& password);