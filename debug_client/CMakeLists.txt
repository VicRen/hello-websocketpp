cmake_minimum_required(VERSION 3.16)
project(debug-client LANGUAGES CXX)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
        ${PROJECT_NAME}
        main.cpp
        ../websocket_transport.cpp
)

target_include_directories(
        ${PROJECT_NAME} PRIVATE
        ${LIBWEBSOCKETPP_INCLUDE_PATH}
        ${JSON_INCLUDE_PATH}
        ${LIBBOOST_INCLUDE_PATH}
        ../
)

link_directories(
        ${Boost_LIBRARY_DIRS}
)