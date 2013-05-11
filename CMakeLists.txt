# project

project(osxrd)

set(ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(SRC_DIR "${ROOT_DIR}/src")
set(BUILD_DIR "${PROJECT_BINARY_DIR}")
set(TEMP_DIR "${BUILD_DIR}/tmp")

# options

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH
    "${ROOT_DIR}/cmake"
    "${ROOT_DIR}/cmake/packages"
    "${ROOT_DIR}/cmake/util"
    ${CMAKE_MODULE_PATH}
)

set(COMMONSRCS
    ${SRC_DIR}/datastream.cpp
    ${SRC_DIR}/miniz.c
    ${SRC_DIR}/timer.cpp
)

set(CLIENTSRCS
    ${COMMONSRCS}
    ${SRC_DIR}/client.cpp
)

set(SERVERSRCS
    ${COMMONSRCS}
    ${SRC_DIR}/server.cpp
)

if(WIN32)
    set(CLIENTSRCS
        ${CLIENTSRCS}
    )
endif()

# dependencies

set(CMAKE_LIBRARY_PATH "${ROOT_DIR}/lib" ${CMAKE_LIBRARY_PATH})
if(WIN32)
    set(CMAKE_LIBRARY_PATH "${ROOT_DIR}/lib/win32"
        ${CMAKE_LIBRARY_PATH})
endif()

message(${CMAKE_LIBRARY_PATH})

set(CMAKE_INCLUDE_PATH ${SRC_DIR} "${ROOT_DIR}/include"
    ${CMAKE_INCLUDE_PATH})
if(WIN32)
    set(CMAKE_INCLUDE_PATH "${ROOT_DIR}/include/win32"
        ${CMAKE_INCLUDE_PATH})
endif()

find_package(ENet REQUIRED)
find_package(OpenGL)
find_package(GLFW)

if(APPLE)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(APP_SERVICES_LIBRARY ApplicationServices)
    find_library(CARBON_LIBRARY Carbon)
endif()

set(COMMON_INCLUDES
    ${ENET_INCLUDE_DIR}
)

set(CLIENT_INCLUDES
    ${COMMON_INCLUDES}
    ${GLFW_INCLUDE_DIR}
)

set(SERVER_INCLUDES
    ${COMMON_INCLUDES}
)

if(WIN32)
    set(COMMON_LIBS
        ${COMMON_LIBS}
        winmm
        ws2_32
    )
endif()

set(COMMON_LIBS
    ${COMMON_LIBS}
    ${ENET_LIBRARY}
)

set(CLIENT_LIBS
    ${COMMON_LIBS}
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY}
)

set(SERVER_LIBS
    ${COMMON_LIBS}
)

if (APPLE)
    set(SERVER_LIBS
        ${CORE_FOUNDATION_FRAMEWORK}
        ${APP_SERVICES_LIBRARY}
        ${CARBON_LIBRARY}
        ${SERVER_LIBS})
endif()

# build

include_directories(${CMAKE_INCLUDE_PATH})

if (WIN32)
    set(BUILD_CLIENT ON)
endif()

if (APPLE)
    set(BUILD_SERVER ON)
endif()

if (BUILD_CLIENT)
    add_executable(client ${CLIENTSRCS})
    set_target_properties(client PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
    )
    include_directories(${CLIENT_INCLUDES})
    target_link_libraries(client ${CLIENT_LIBS})
endif()

if (BUILD_SERVER)
    add_executable(server ${SERVERSRCS})
    set_target_properties(server PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
    )
    include_directories(${SERVER_INCLUDES})
    target_link_libraries(server ${SERVER_LIBS})
endif()