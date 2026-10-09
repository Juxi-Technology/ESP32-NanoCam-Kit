set(include_dirs
    .
    ai
    ../components/modules/ai
)

file(GLOB_RECURSE SOURCES "*.cpp" "*.cc")

set(REQUIRES_COMMON
    driver esp_wifi esp_http_server esp_http_client nvs_flash spi_flash
    esp32-camera mbedtls app_update efuse esp_security pthread esp_psram
    spiffs esp-code-scanner
    modules
    micro_ros_espidf_component
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES ${REQUIRES_COMMON}
)
