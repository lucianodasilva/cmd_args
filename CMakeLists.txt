cmake_minimum_required (VERSION 3.0)
project (cmd_args)

set(CMAKE_C_STANDARD 14)
set(CMAKE_CXX_STANDARD 14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# test project
OPTION ( CMD_ARGS_BUILD_TESTS "Build unit tests" ON)

if (${CMD_ARGS_BUILD_TESTS})
    set ( CMD_ARGS_GTEST_PATH "${CMAKE_CURRENT_LIST_DIR}/googletest" CACHE PATH "google test project path")
    
    option (BUILD_GTEST "" ON)
    add_subdirectory (${CMD_ARGS_GTEST_PATH})
    
    list (APPEND gtest_projects  "gmock" "gmock_main" "gtest" "gtest_main")
    set_target_properties (${gtest_projects} PROPERTIES FOLDER "gtest")
    
    if (MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Od /Ob0 /D \"_DEBUG\"")
        set(CMAKE_CXX_FLAGS_RELEASE "/MT")
    endif ()
    
    file (GLOB test_sources
        ${CMAKE_CURRENT_LIST_DIR}/test/*.h
        ${CMAKE_CURRENT_LIST_DIR}/test/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp
    )
    
    add_executable (utest ${test_sources})
    target_link_libraries(utest gtest)
endif()

# example project
project (example)

include_directories (
	${CMAKE_CURRENT_LIST_DIR}/include
)

add_executable ( example ${CMAKE_CURRENT_LIST_DIR}/include/cmd_args.h ${CMAKE_CURRENT_LIST_DIR}/example/main.cpp )

project (cmd_args)