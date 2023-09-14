cmake_minimum_required(VERSION 3.19)

project(sqlite_jsoncons VERSION 0.1)
set(TARGET_NAME "sqlite_jsoncons")
set(SOURCES "sqlite_jsoncons.cpp")

# we might be able to use this to do some of the heavy lifting of the
# package build upload
# The github action looks a bit easier: 
#     https://github.com/simonw/blip-caption/blob/main/.github/workflows/publish.yml
find_package(Python COMPONENTS Interpreter REQUIRED)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_path(SQLITE_INCLUDE_DIR sqlite3ext.h)
include_directories(${SQLITE_INCLUDE_DIR})

find_package(jsoncons CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(${TARGET_NAME}   MODULE  ${SOURCES})
target_link_libraries(${TARGET_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${TARGET_NAME} PRIVATE jsoncons)

set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)

# we just want the library and the README in the archive
install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION .)  
install(FILES README.md DESTINATION .)


# this hardwires the zip archive name. This would be a problem if
# we are using a multi-config generator (e.g. Ninja; Visual Studio) with multiple configs
# in the same build directory. However, when this is run on Github, it should be with different jobs
# and we can take care of renaming the archive (perhaps with release tags; architecture; built-type) with 
# variables derived from the CI job.
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_GENERATOR "ZIP" CACHE STRING "Generators to support. semi-colon delimited list")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

include(CPack)