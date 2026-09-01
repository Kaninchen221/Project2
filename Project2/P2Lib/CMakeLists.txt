message(STATUS "*** Create Project 2 Lib ***")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "include/*.hpp"
    "source/*.cpp"
)

add_library(P2Lib STATIC ${SOURCES})

target_compile_features(P2Lib PUBLIC cxx_std_23)
target_include_directories(P2Lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(P2Lib PUBLIC 
	spdlog::spdlog
	gtest::gtest
	nlohmann_json::nlohmann_json
	sfml::sfml
	Taskflow::Taskflow
	stb::stb
	ImGuiSFML
)

set_target_properties(P2Lib PROPERTIES
	LINKER_LANGUAGE CXX
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
	INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)