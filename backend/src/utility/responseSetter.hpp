#pragma once

#include <crow.h>
#include <string>

void setResponse(crow::json::wvalue& res,const std::string& status, const std::string& message){
    res["status"] = status;
    res["message"] = message;
}