cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
project(sengine_jump_king)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(SENGINE_DEBUG)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SRC_CPP_FILES src/*.cpp)
add_executable(${PROJECT_NAME} main.cpp ${SRC_CPP_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE include)

add_subdirectory(thirdparty)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty)
target_link_libraries(${PROJECT_NAME} PRIVATE sengine)