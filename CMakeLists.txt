# ESP-IDF project boilerplate — keep this order.
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Local overlay for atanisoft/OpenMRNIDF on ESP-IDF 5.1 (WiFi-only).
# Do not push these CMake edits to the upstream submodule.
set(_om_cmake "${CMAKE_CURRENT_LIST_DIR}/components/OpenMRNIDF/CMakeLists.txt")
if(EXISTS "${_om_cmake}")
    file(READ "${_om_cmake}" _om_txt)
    if(NOT _om_txt MATCHES "Esp32HardwareTwai.cpp")
        execute_process(
            COMMAND git apply "${CMAKE_CURRENT_LIST_DIR}/patches/OpenMRNIDF-idf51-wifi-only.patch"
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/components/OpenMRNIDF"
            RESULT_VARIABLE _om_rc
        )
        if(NOT _om_rc EQUAL 0)
            message(FATAL_ERROR "Failed to apply patches/OpenMRNIDF-idf51-wifi-only.patch")
        endif()
    endif()
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(d1r32_ili9486_openmrn_wifi)
