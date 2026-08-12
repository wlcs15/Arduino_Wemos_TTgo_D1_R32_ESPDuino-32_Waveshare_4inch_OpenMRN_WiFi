idf_component_register(
    SRCS
        "main.cpp"
        "wifi_cred.cpp"
        "ili9486_min.cpp"
        "debug_ids.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_event
        esp_netif
        esp_wifi
        spi_flash
        mbedtls
        driver
        OpenMRNIDF
)

# Initial bring-up: DEBUG ID screen + serial. Turn off later with -DDEBUG=0.
target_compile_definitions(${COMPONENT_LIB} PRIVATE DEBUG=1)

# Compile-time secrets from the environment (utils/build_idf5.sh).
# Never log or commit these values. CMakeCache.txt is gitignored.
if(DEFINED ENV{WIFI_SSID} AND NOT "$ENV{WIFI_SSID}" STREQUAL "")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        "WIFI_SSID_FROM_ENV=\"$ENV{WIFI_SSID}\""
    )
endif()
if(DEFINED ENV{WIFI_PASSWORD} AND NOT "$ENV{WIFI_PASSWORD}" STREQUAL "")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        "WIFI_PASSWORD_FROM_ENV=\"$ENV{WIFI_PASSWORD}\""
    )
endif()
