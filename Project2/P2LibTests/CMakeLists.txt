message(STATUS "*** Create P2Lib Tests ***")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "include/*.hpp"
    "source/*.cpp"
)

add_executable(P2LibTests
	${SOURCES}
	main.cpp
)

target_compile_features(P2LibTests PUBLIC cxx_std_23)

target_compile_definitions(P2LibTests PUBLIC GTEST_LINKED_AS_SHARED_LIBRARY)

target_link_libraries(P2LibTests PUBLIC 
	P2Lib
    gtest::gtest)

set_target_properties(P2LibTests
    PROPERTIES
	LINKER_LANGUAGE CXX
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
	INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/runtime
)

add_test(NAME P2LibTests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin COMMAND P2LibTests)