cmake_minimum_required(VERSION 3.16)

project(cache_application)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

set(SOURCES
    main.cpp
    src/application.cpp
)


add_executable(${PROJECT_NAME} ${SOURCES})

add_library(legacy STATIC src/legacy.c)

target_link_libraries(${PROJECT_NAME} PRIVATE legacy)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
