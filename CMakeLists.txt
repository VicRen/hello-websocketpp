cmake_minimum_required(VERSION 3.16)
project(hello-websocketpp LANGUAGES CXX)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cmake param should set WEBRTC_HEADER_PATH/WEBRTC_LIB_PATH/BOOST_HEADER_PATH
if (NOT BOOST_HEADER_PATH)
    message(FATAL_ERROR "BOOST_HEADER_PATH not provided")
else ()
    message(STATUS "BOOST_HEADER_PATH = " ${BOOST_HEADER_PATH})
endif ()

if (NOT BOOST_LIB_PATH)
    message(FATAL_ERROR "BOOST_LIB_PATH not provided")
else ()
    message(STATUS "BOOST_LIB_PATH = " ${BOOST_LIB_PATH})
endif ()

if (NOT LIBWEBSOCKETPP_INCLUDE_PATH)
    message(FATAL_ERROR "LIBWEBSOCKETPP_INCLUDE_PATH not provided")
else ()
    message(STATUS "WEBSOCKETPP_INCLUDE_PATH = " ${LIBWEBSOCKETPP_INCLUDE_PATH})
endif ()

if (NOT JSON_INCLUDE_PATH)
    message(FATAL_ERROR "JSON_INCLUDE_PATH not provided")
else ()
    message(STATUS "JSON_INCLUDE_PATH = " ${JSON_INCLUDE_PATH})
endif ()

set(LIBBOOST_INCLUDE_PATH ${BOOST_HEADER_PATH} CACHE STRING "libboost include path" FORCE)
set(LIBBOOST_BINARY_PATH ${BOOST_LIB_PATH} CACHE STRING "libboost binary path" FORCE)

add_executable(
        ${PROJECT_NAME}
        main.cpp
        websocket_transport.cpp
)

target_include_directories(
        ${PROJECT_NAME} PRIVATE
        ${LIBWEBSOCKETPP_INCLUDE_PATH}
        ${JSON_INCLUDE_PATH}
        ${LIBBOOST_INCLUDE_PATH}
)

link_directories(
        ${Boost_LIBRARY_DIRS}
)

add_subdirectory(debug_server)
add_subdirectory(debug_client)