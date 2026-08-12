# ESP-IDF project boilerplate — keep this order.
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenMRNIDF 5.1.0 does not list the registry mdns component. Apply a
# one-line REQUIRES overlay if it is not already present.
set(_om_cmake "${CMAKE_CURRENT_LIST_DIR}/components/OpenMRNIDF/CMakeLists.txt")
set(_om_patch "${CMAKE_CURRENT_LIST_DIR}/patches/OpenMRNIDF-idf51-mdns.patch")
if(EXISTS "${_om_cmake}" AND EXISTS "${_om_patch}")
    file(READ "${_om_cmake}" _om_txt)
    if(NOT _om_txt MATCHES "\n    mdns\\)")
        execute_process(
            COMMAND git apply "${_om_patch}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/components/OpenMRNIDF"
            RESULT_VARIABLE _om_rc
        )
        if(NOT _om_rc EQUAL 0)
            message(FATAL_ERROR "Failed to apply patches/OpenMRNIDF-idf51-mdns.patch")
        endif()
    endif()
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(d1r32_ili9486_openmrn_wifi)
