file(GLOB CFILES "*.c")
file(GLOB HFILES "*.h")

idf_component_register(SRCS ${CFILES} ${HFILES}
                       PRIV_REQUIRES spi_flash
                       REQUIRES esp_driver_i2c esp_driver_ledc esp_driver_ppa
                       INCLUDE_DIRS ".")