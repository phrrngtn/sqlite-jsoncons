cmake_minimum_required(VERSION 3.19)

project(sqlite_jsoncons)
set(TARGET_NAME "sqlite_jsoncons")
set(SOURCES "sqlite_jsoncons.cpp")

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_path(SQLITE_INCLUDE_DIR sqlite3ext.h)
include_directories(${SQLITE_INCLUDE_DIR})

find_package(jsoncons CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(${TARGET_NAME}   SHARED ${SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_NAME} PRIVATE jsoncons)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)