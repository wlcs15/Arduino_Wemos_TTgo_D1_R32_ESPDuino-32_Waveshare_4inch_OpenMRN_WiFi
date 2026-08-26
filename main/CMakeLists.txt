idf_component_register(
    SRCS
        "main.cpp"
        "wifi_cred.cpp"
        "ili9486_min.cpp"
        "debug_ids.cpp"
        "wifi_sta.cpp"
        "svc_reach.cpp"
        "lcc_uplink.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        nvs_flash
        esp_event
        esp_netif
        esp_wifi
        spi_flash
        mbedtls
        driver
        lwip
        OpenMRNIDF
)

# Initial bring-up: DEBUG ID screen + serial. Turn off later with -DDEBUG=0.
target_compile_definitions(${COMPONENT_LIB} PRIVATE DEBUG=1)

set(_gv_py "${CMAKE_SOURCE_DIR}/utils/git_version.py")
set(_gv_ver "v0.01+")
if(EXISTS "${_gv_py}")
    if(DEFINED PYTHON)
        set(_gv_python "${PYTHON}")
    else()
        find_program(_gv_python NAMES python3 python)
    endif()
    execute_process(
        COMMAND "${_gv_python}" "${_gv_py}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _gv_ver
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _gv_rc
        ERROR_QUIET
    )
    if(NOT _gv_rc EQUAL 0 OR _gv_ver STREQUAL "")
        set(_gv_ver "v0.01+")
    endif()
endif()
message(STATUS "SNIP softwareVersion ${_gv_ver}")
target_compile_definitions(${COMPONENT_LIB} PRIVATE RR_GIT_VERSION=${_gv_ver})

# Host-encrypted wrap only (utils/provision_wifi_build.sh). Never a PSK here.
set(_wifi_wrap_inc "${CMAKE_CURRENT_LIST_DIR}/wifi_psk_wrap.inc")
if(EXISTS "${_wifi_wrap_inc}")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE WIFI_WRAP_BLOB_PRESENT=1)
    set_source_files_properties(wifi_cred.cpp PROPERTIES OBJECT_DEPENDS "${_wifi_wrap_inc}")
    message(STATUS "wifi_cred: baking host-encrypted wrap blob (ciphertext only)")
else()
    message(STATUS "wifi_cred: no wrap blob; DEBUG collect / NVS-only build")
endif()
