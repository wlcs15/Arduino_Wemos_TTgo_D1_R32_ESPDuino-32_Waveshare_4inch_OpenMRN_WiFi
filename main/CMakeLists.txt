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

# Host-encrypted wrap only (utils/provision_wifi_build.sh). Never a PSK here.
set(_wifi_wrap_inc "${CMAKE_CURRENT_LIST_DIR}/wifi_psk_wrap.inc")
if(EXISTS "${_wifi_wrap_inc}")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE WIFI_WRAP_BLOB_PRESENT=1)
    set_source_files_properties(wifi_cred.cpp PROPERTIES OBJECT_DEPENDS "${_wifi_wrap_inc}")
    message(STATUS "wifi_cred: baking host-encrypted wrap blob (ciphertext only)")
else()
    message(STATUS "wifi_cred: no wrap blob; DEBUG collect / NVS-only build")
endif()
