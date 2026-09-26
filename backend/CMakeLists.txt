cmake_minimum_required(VERSION 3.20)

project(app_to_down_api 
        VERSION 1.0.0
        LANGUAGES CXX
    )


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Crow CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_library(BCRYPT_LIB bcrypt REQUIRED)

add_executable(app_to_down_api
                src/app.cpp
)

target_link_libraries(app_to_down_api
                PRIVATE 
                Crow::Crow
                libpqxx::pqxx
                ${BCRYPT_LIB}
    )


target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

target_compile_options(app_to_down_api
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )