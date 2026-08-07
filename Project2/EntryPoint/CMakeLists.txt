message(STATUS "*** Create P2 Entry Point ***")

add_executable(P2EntryPoint

"include/P2EntryPoint.hpp"
	
"source/P2EntryPoint.cpp"
	
"main.cpp"

)

target_compile_features(P2EntryPoint PUBLIC cxx_std_23)

target_link_libraries(P2EntryPoint PUBLIC P2Lib)

set_target_properties(P2EntryPoint
    PROPERTIES
	LINKER_LANGUAGE CXX
	INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
	INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/archive
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
