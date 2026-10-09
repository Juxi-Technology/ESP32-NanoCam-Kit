idf_build_get_property(target IDF_TARGET)

if(${IDF_TARGET} STREQUAL "esp32")
    set(lib_path "${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32")
elseif(${IDF_TARGET} STREQUAL "esp32s2")
    set(lib_path "${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32s2")
elseif(${IDF_TARGET} STREQUAL "esp32s3")
    set(lib_path "${CMAKE_CURRENT_SOURCE_DIR}/lib/esp32s3")
endif()

idf_component_register(
    INCLUDE_DIRS include
)

target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-L ${lib_path}"
    "-lesp-code-scanner"
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libnewlib_iconv.a
)
