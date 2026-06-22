cmake_minimum_required(VERSION 3.16)

project(ControlPanelSoftware VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(QT_QML_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml")
set(LVGL_DIR "${CMAKE_SOURCE_DIR}/lvgl_sim_qt/lvgl")

find_package(Qt6 REQUIRED COMPONENTS Core Quick Widgets)

add_subdirectory(lvgl_sim_qt)
add_subdirectory(lvgl_sim)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(ControlPanelSoftwareApp
        src/main.cpp
)

file(GLOB qml_files
        RELATIVE ${PROJECT_SOURCE_DIR}
        "src/qml/*.qml"
)
file(GLOB block_files
        RELATIVE ${PROJECT_SOURCE_DIR}
        "src/qml/blocks/*.qml"
)
file(GLOB source_files
        RELATIVE ${PROJECT_SOURCE_DIR}
        "src/cpp/*.h"
        "src/cpp/*.cpp"
)

set_source_files_properties(src/qml/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)
qt_add_qml_module(ControlPanelSoftwareApp
    URI ControlPanelSoftware
    QML_FILES
        ${qml_files} ${block_files}
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
    PRIVATE Qt6::Core Qt6::Quick Qt6::Widgets lvgl_simplugin
)

include(GNUInstallDirs)
install(TARGETS ControlPanelSoftwareApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
