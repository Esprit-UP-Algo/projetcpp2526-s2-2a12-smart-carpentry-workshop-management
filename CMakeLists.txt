cmake_minimum_required(VERSION 3.16)

project(WoodFlow VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui ui/modules/auth)

# ── Qt: find ONCE with ALL required modules ───────────────────────────────────
find_package(Qt6 COMPONENTS
    Core
    Widgets
    PrintSupport
    Sql
    Network
    SvgWidgets
    Location
    Positioning
    Quick
    QuickWidgets
    QuickControls2
    QUIET
)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS
        Core Widgets PrintSupport Sql Network SvgWidgets
        Location Positioning Quick QuickWidgets QuickControls2)
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
    src/models/projet.h
    src/models/projet.cpp

    # Database
    src/database/connection.cpp
    src/database/connection.h
    src/database/employeedatabase.cpp
    src/database/employeedatabase.h
    src/database/stockdatabase.h
    src/database/stockdatabase.cpp
    src/database/projectdatabase.h
    src/database/projectdatabase.cpp

    # Core
    src/core/session.cpp
    src/core/session.h

    # Common
    src/common/validators.cpp
    src/common/validators.h

    # Auth Module
    src/modules/auth/loginpage.cpp
    src/modules/auth/loginpage.h
    src/modules/auth/forgotpasswordpage.cpp
    src/modules/auth/forgotpasswordpage.h

    # Employee Module
    src/modules/employees/employeemanagementpage.cpp
    src/modules/employees/employeemanagementpage.h
    src/modules/employees/employeedialog.cpp
    src/modules/employees/employeedialog.h

    # Stock Module
    src/models/stockmaterial.h
    src/models/stockmaterial.cpp
    src/modules/stock/stockpage.h
    src/modules/stock/stockpage.cpp

    # Projects Module
    src/modules/projects/projectmanagementpage.h
    src/modules/projects/projectmanagementpage.cpp
    src/modules/projects/projectstatspage.h
    src/modules/projects/projectstatspage.cpp

    # Finance Module
    src/modules/finance/financemodel.h
    src/modules/finance/financemodel.cpp
    src/modules/finance/financeview.h
    src/modules/finance/financeview.cpp
    src/modules/finance/transactiondialog.h
    src/modules/finance/transactiondialog.cpp

    # UI Files
    ui/modules/auth/loginpage.ui
)

if(Qt6_FOUND)
    qt_add_executable(WoodFlow ${PROJECT_SOURCES}
        src/modules/stock/stockpage.h src/modules/stock/stockpage.cpp
        src/database/stockdatabase.h src/database/stockdatabase.cpp
        src/models/stockmaterial.h src/models/stockmaterial.cpp
        src/modules/projects/projectmanagementpage.h src/modules/projects/projectmanagementpage.cpp
        src/models/projet.h src/models/projet.cpp
        src/database/projectdatabase.h src/database/projectdatabase.cpp
        src/modules/projects/projectstatspage.h src/modules/projects/projectstatspage.cpp
        src/common/projectvalidators.h src/common/projectvalidators.cpp
        src/common/stockvalidators.h src/common/stockvalidators.cpp
        src/modules/stock/stockalertbell.h src/modules/stock/stockalertbell.cpp
        src/modules/stock/stockalertpanel.h src/modules/stock/stockalertpanel.cpp
        src/modules/stock/stocklocales.h src/modules/stock/stocklocales.cpp

        # Map view (Qt Location / QML-based)
        src/modules/stock/stockmapview.h
        src/modules/stock/stockmapview.cpp
    )

    # ── QML module: embeds MapView.qml into the binary as a Qt resource ───────
    qt_add_qml_module(WoodFlow
        URI WoodFlowApp
        VERSION 1.0
        QML_FILES
            src/modules/stock/qml/MapView.qml
        RESOURCE_PREFIX /
    )
else()
    add_executable(WoodFlow ${PROJECT_SOURCES})
endif()

# Include directories
target_include_directories(WoodFlow PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ── Link ALL Qt modules ────────────────────────────────────────────────────────
if(Qt6_FOUND)
    target_link_libraries(WoodFlow PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::PrintSupport
        Qt6::Sql
        Qt6::Network
        Qt6::SvgWidgets
        Qt6::Location
        Qt6::Positioning
        Qt6::Quick
        Qt6::QuickWidgets
        Qt6::QuickControls2
    )
else()
    target_link_libraries(WoodFlow PRIVATE
        Qt5::Core Qt5::Widgets Qt5::PrintSupport Qt5::Sql Qt5::Network Qt5::SvgWidgets
        Qt5::Location Qt5::Positioning Qt5::Quick Qt5::QuickWidgets Qt5::QuickControls2
)
endif()

# Copy stylesheets
configure_file(${CMAKE_SOURCE_DIR}/style.qss     ${CMAKE_BINARY_DIR}/style.qss     COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/style-dark.qss ${CMAKE_BINARY_DIR}/style-dark.qss COPYONLY)

# Create assets directories in build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/assets/icons)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/assets)

if(EXISTS ${CMAKE_SOURCE_DIR}/src/assets/icons/icon.jpeg)
    configure_file(${CMAKE_SOURCE_DIR}/src/assets/icons/icon.jpeg
                   ${CMAKE_BINARY_DIR}/src/assets/icons/icon.jpeg COPYONLY)
endif()
if(EXISTS ${CMAKE_SOURCE_DIR}/src/assets/icons/pfp.jpeg)
    configure_file(${CMAKE_SOURCE_DIR}/src/assets/icons/pfp.jpeg
                   ${CMAKE_BINARY_DIR}/src/assets/icons/pfp.jpeg COPYONLY)
endif()
if(EXISTS ${CMAKE_SOURCE_DIR}/src/assets/icons/logo1.png)
    configure_file(${CMAKE_SOURCE_DIR}/src/assets/icons/logo1.png
                   ${CMAKE_BINARY_DIR}/src/assets/icons/logo1.png COPYONLY)
endif()

# NOTE: tunisia.svg is no longer needed — the map is now rendered by Qt Location (OSM tiles)

set_target_properties(WoodFlow PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)