cmake_minimum_required(VERSION 3.15)
project(Project2 LANGUAGES CXX)

# Global compile options
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX /external:W0 /external:anglebrackets /MP)
endif()

# Find packages
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SFML CONFIG REQUIRED)
find_package(taskflow CONFIG REQUIRED)
find_package(stb CONFIG REQUIRED)

# Add subdirectories
add_subdirectory(Project2/ImGuiLib)
add_subdirectory(Project2/P2Lib)
add_subdirectory(Project2/P2LibTests)
add_subdirectory(Project2/EntryPoint)