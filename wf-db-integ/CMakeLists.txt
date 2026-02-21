cmake_minimum_required(VERSION 3.16)

project(WoodFlow VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ui ui/modules/auth)

# ── Qt: find ONCE with ALL required modules ──────────────────────────────────
find_package(Qt6 COMPONENTS Core Widgets PrintSupport Sql QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets PrintSupport Sql)
endif()

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    toggleswitch.cpp
    toggleswitch.h

    # Models
    src/models/employee.cpp
    src/models/employee.h

    # Database
    src/database/connection.cpp
    src/database/connection.h
    src/database/employeedatabase.cpp
    src/database/employeedatabase.h

    # Core
    src/core/session.cpp
    src/core/session.h

    # Common
    src/common/validators.cpp
    src/common/validators.h

    # Auth Module
    src/modules/auth/loginpage.cpp
    src/modules/auth/loginpage.h
    src/modules/auth/registerpage.cpp
    src/modules/auth/registerpage.h

    # Employee Module
    src/modules/employees/employeemanagementpage.cpp
    src/modules/employees/employeemanagementpage.h
    src/modules/employees/employeedialog.cpp
    src/modules/employees/employeedialog.h

    # UI Files
    ui/modules/auth/loginpage.ui
    ui/modules/auth/registerpage.ui
)

if(Qt6_FOUND)
    qt_add_executable(WoodFlow ${PROJECT_SOURCES}
        src/modules/stock/stockpage.h src/modules/stock/stockpage.cpp)
else()
    add_executable(WoodFlow ${PROJECT_SOURCES})
endif()

# Include directories
target_include_directories(WoodFlow PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ── Link ALL Qt modules ───────────────────────────────────────────────────────
if(Qt6_FOUND)
    target_link_libraries(WoodFlow PRIVATE Qt6::Core Qt6::Widgets Qt6::PrintSupport Qt6::Sql)
else()
    target_link_libraries(WoodFlow PRIVATE Qt5::Core Qt5::Widgets Qt5::PrintSupport Qt5::Sql)
endif()

# Copy stylesheet and images to build directory
configure_file(${CMAKE_SOURCE_DIR}/style.qss ${CMAKE_BINARY_DIR}/style.qss COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/style-dark.qss ${CMAKE_BINARY_DIR}/style-dark.qss COPYONLY)

# Create assets directory in build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/assets/icons)

if(EXISTS ${CMAKE_SOURCE_DIR}/src/assets/icons/icon.jpeg)
    configure_file(${CMAKE_SOURCE_DIR}/src/assets/icons/icon.jpeg ${CMAKE_BINARY_DIR}/src/assets/icons/icon.jpeg COPYONLY)
endif()
if(EXISTS ${CMAKE_SOURCE_DIR}/src/assets/icons/pfp.jpeg)
    configure_file(${CMAKE_SOURCE_DIR}/src/assets/icons/pfp.jpeg ${CMAKE_BINARY_DIR}/src/assets/icons/pfp.jpeg COPYONLY)
endif()
if(EXISTS ${CMAKE_SOURCE_DIR}/src/assets/icons/logo1.png)
    configure_file(${CMAKE_SOURCE_DIR}/src/assets/icons/logo1.png ${CMAKE_BINARY_DIR}/src/assets/icons/logo1.png COPYONLY)
endif()

set_target_properties(WoodFlow PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
