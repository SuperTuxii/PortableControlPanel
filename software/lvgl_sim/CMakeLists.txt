cmake_minimum_required(VERSION 3.10)

set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE STRING "" FORCE)
add_subdirectory(lvgl EXCLUDE_FROM_ALL)

qt_add_library(lvgl_sim STATIC)

qt_add_qml_module(lvgl_sim
        URI "LvglSimulator"
        VERSION 1.0
        QML_FILES
            src/LvglDisplay.qml
        SOURCES
            src/LvglRenderer.h
            src/LvglRenderer.cpp
            src/LvglImageProvider.h
            src/LvglImageProvider.cpp
)

target_include_directories(lvgl_sim
        PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(lvgl_sim
        PRIVATE
            lvgl
            Qt::Gui
            Qt::Quick
)
