cmake_minimum_required(VERSION 3.9)

project(httpc)

add_library(${PROJECT_NAME}
        httpclient.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})
