file(GLOB CPPFILES "*.cpp")
file(GLOB CFILES "*.c")
file(GLOB HFILES "*.h")

idf_component_register(SRCS ${CPPFILES} ${CFILES} ${HFILES}
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_i2c esp_driver_ledc esp_driver_ppa
                       INCLUDE_DIRS ".")

add_subdirectory(${CMAKE_SOURCE_DIR}/../protocol protocol)
target_link_libraries(${COMPONENT_LIB} PRIVATE protocol_lib)