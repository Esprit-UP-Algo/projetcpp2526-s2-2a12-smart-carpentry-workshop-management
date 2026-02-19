cmake_minimum_required(VERSION 3.16)

project(Atelier_Connexion VERSION 1.0 LANGUAGES CXX)

# ── C++ Standard ─────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt auto-tools ─────────────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)   # handles Q_OBJECT macros
set(CMAKE_AUTOUIC ON)   # processes .ui files  → ui_*.h
set(CMAKE_AUTORCC ON)   # processes .qrc files (if any)

# ── Find Qt (supports both Qt5 and Qt6) ──────────────────────────────────────
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Sql)

# ── Sources ───────────────────────────────────────────────────────────────────
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    connection.cpp
    connection.h
)

# ── Executable ────────────────────────────────────────────────────────────────
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Atelier_Connexion
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(Atelier_Connexion
        ${PROJECT_SOURCES}
    )
endif()

# ── Link Qt modules ───────────────────────────────────────────────────────────
target_link_libraries(Atelier_Connexion PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

# ── Windows: suppress the console window in Release builds ───────────────────
if(WIN32)
    set_target_properties(Atelier_Connexion PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ── Qt6 finalization (no-op on Qt5) ──────────────────────────────────────────
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Atelier_Connexion)
endif()
