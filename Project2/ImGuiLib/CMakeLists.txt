message(STATUS "*** Create ImGui Lib ***")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "include/*.h"
    "source/*.cpp"
)

add_library(ImGuiLib STATIC ${SOURCES})

target_compile_features(ImGuiLib PUBLIC cxx_std_23)
target_include_directories(ImGuiLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(ImGuiLib PUBLIC
	stb::stb
)

set_target_properties(ImGuiLib PROPERTIES
	LINKER_LANGUAGE CXX
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
	INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)