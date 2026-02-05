cmake_minimum_required(VERSION 3.16)

project(luck_dev VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 COMPONENTS Core Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)
endif()

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if(Qt6_FOUND)
    qt_add_executable(luck_dev ${PROJECT_SOURCES})
else()
    add_executable(luck_dev ${PROJECT_SOURCES})
endif()

if(Qt6_FOUND)
    target_link_libraries(luck_dev PRIVATE Qt6::Core Qt6::Widgets)
else()
    target_link_libraries(luck_dev PRIVATE Qt5::Core Qt5::Widgets)
endif()

# Copy stylesheet and images to build directory
configure_file(${CMAKE_SOURCE_DIR}/style.qss ${CMAKE_BINARY_DIR}/style.qss COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/style-dark.qss ${CMAKE_BINARY_DIR}/style-dark.qss COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/icon.jpeg ${CMAKE_BINARY_DIR}/icon.jpeg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/pfp.jpeg ${CMAKE_BINARY_DIR}/pfp.jpeg COPYONLY)

set_target_properties(luck_dev PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
