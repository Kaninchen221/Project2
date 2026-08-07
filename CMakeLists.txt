cmake_minimum_required(VERSION 3.15)
project(Project2 LANGUAGES CXX)

# Find packages
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Add subdirectories
add_subdirectory(Project2/P2Lib)
add_subdirectory(Project2/P2LibTests)
add_subdirectory(Project2/EntryPoint)