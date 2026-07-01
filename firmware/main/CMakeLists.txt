file(GLOB CFILES "*.c")
file(GLOB HFILES "*.h")

if(IDF_TARGET)
idf_component_register(SRCS ${CFILES} ${HFILES}
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_i2c esp_driver_ledc esp_driver_ppa
                       INCLUDE_DIRS ".")
else()
add_library(lv_control_grid_lib
        "lv_control_grid.c"
        "lv_control_grid.h"
        "util.c"
        "util.h"
)

target_include_directories(lv_control_grid_lib PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}" "${LVGL_DIR}"
)

target_link_libraries(lv_control_grid_lib PUBLIC lvgl)
endif()