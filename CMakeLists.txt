cmake_minimum_required(VERSION 3.22)
project(paho_client)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mqtt.hpp
)
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")
link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/debug")

IF (WIN32)
    link_directories("${CMAKE_CURRENT_LIST_DIR}/lib")
    add_executable(paho_client ${SOURCES})
    target_link_libraries(paho_client paho-mqtt3as-static paho-mqttpp3-static libcrypto64MD.lib libssl64MD.lib ws2_32.lib Advapi32.lib User32.lib)
ELSE()
    add_executable(paho_client ${SOURCES})
    target_link_libraries(paho_client paho-mqtt3as paho-mqttpp3 crypto ssl)
ENDIF()