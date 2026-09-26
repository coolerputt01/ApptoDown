#pragma once
#include <string>

class User {
    public:

        User(std::string user_name,std::string email, std::string pass){
            this->username = user_name;
            this->email = email;
            this->passwordhash_ = pass;
            this->is_verified_ = false;
        }

        const std::string& get_username() const {
            return username;
        }

        const std::string& get_email() const {
            return email;
        }

        const std::string& get_password() const {
            return passwordhash_;
        }

        ~User() = default;
    private:
        std::string username;
        std::string email;
        std::string passwordhash_;
        bool is_verified_;
};