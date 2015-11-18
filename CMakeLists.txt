cmake_minimum_required (VERSION 3.0)

project (cmd_args_example)

set(CMAKE_C_STANDARD 14)
set(CMAKE_CXX_STANDARD 14)

include_directories (
	${CMAKE_CURRENT_LIST_DIR}/include
)

add_executable ( example ${CMAKE_CURRENT_LIST_DIR}/include/cmd_args.h ${CMAKE_CURRENT_LIST_DIR}/example/main.cpp )
