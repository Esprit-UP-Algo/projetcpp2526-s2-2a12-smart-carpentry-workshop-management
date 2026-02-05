include(C:/Users/amine/Desktop/projetqt/interface/build/.qt/QtDeploySupport.cmake)
include("${CMAKE_CURRENT_LIST_DIR}/interface-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_ALL_MODULES_FOUND_VIA_FIND_PACKAGE "ZlibPrivate;EntryPointPrivate;Core;Gui;Widgets")

qt6_deploy_runtime_dependencies(
    EXECUTABLE C:/Users/amine/Desktop/projetqt/interface/build/interface.exe
    GENERATE_QT_CONF
)
