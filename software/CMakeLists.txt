cmake_minimum_required(VERSION 3.16)

project(ControlPanelSoftware VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")
set(LVGL_DIR "${CMAKE_SOURCE_DIR}/lvgl_sim_qt/lvgl")

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets)

add_subdirectory(lvgl_sim)
add_subdirectory(${CMAKE_SOURCE_DIR}/../firmware/main lv_control_grid_lib)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(ControlPanelSoftwareApp
        src/main.cpp
)

file(GLOB qml_files
        RELATIVE ${PROJECT_SOURCE_DIR}
        "src/qml/*.qml"
)
file(GLOB menu_values
        RELATIVE ${PROJECT_SOURCE_DIR}
        "src/qml/menuValues/*.qml"
)
file(GLOB source_files
        RELATIVE ${PROJECT_SOURCE_DIR}
        "src/cpp/*.h"
        "src/cpp/*.cpp"
)

qt_add_resources(ControlPanelSoftwareApp "fonts"
    PREFIX "/"
    FILES
        "fonts/MaterialSymbolsRounded.ttf"
        "fonts/Montserrat.ttf"
)

set_source_files_properties(src/qml/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)
qt_add_qml_module(ControlPanelSoftwareApp
    URI ControlPanelSoftware
    QML_FILES
        ${qml_files} ${menu_values}
    SOURCES
        ${source_files}
)

target_include_directories(ControlPanelSoftwareApp PRIVATE src/cpp)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(ControlPanelSoftwareApp PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ControlPanelSoftwareApp
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(ControlPanelSoftwareApp
    PRIVATE Qt6::Core Qt6::Quick Qt6::Widgets lvgl_simplugin lv_control_grid_lib
)

include(GNUInstallDirs)
install(TARGETS ControlPanelSoftwareApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
