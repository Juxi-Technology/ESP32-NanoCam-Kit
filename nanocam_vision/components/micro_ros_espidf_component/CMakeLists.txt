option(BUILD_ETHERNET_INTERFACE "Build ethernet interface support" OFF)
option(BUILD_WLAN_INTERFACE "Build wlan interface support" ON)

set(COMPONENT_SRC "")

if(BUILD_ETHERNET_INTERFACE)
    list(APPEND COMPONENT_SRC "network_interfaces/uros_ethernet_netif.c")
endif()

if(BUILD_WLAN_INTERFACE)
    list(APPEND COMPONENT_SRC "network_interfaces/uros_wlan_netif.c")
endif()

idf_component_register(SRCS ${COMPONENT_SRC}
                       INCLUDE_DIRS "network_interfaces"
                       REQUIRES nvs_flash esp_wifi esp_eth lwip)

# Use pre-built libmicroros.a (cross-compiled on Ubuntu)
add_prebuilt_library(libmicroros-prebuilt ""
                     REQUIRES lwip)

set_target_properties(libmicroros-prebuilt PROPERTIES IMPORTED_LOCATION ${COMPONENT_DIR}/libmicroros.a)
target_include_directories(libmicroros-prebuilt INTERFACE ${COMPONENT_DIR}/include)
target_compile_definitions(libmicroros-prebuilt INTERFACE RCL_MICROROS)

file(GLOB include_folders LIST_DIRECTORIES true CONFIGURE_DEPENDS ${COMPONENT_DIR}/include/*)
foreach(include_folder ${include_folders})
    if(IS_DIRECTORY ${include_folder})
        get_filename_component(folder_name ${include_folder} NAME)
        target_include_directories(libmicroros-prebuilt INTERFACE ${COMPONENT_DIR}/include/${folder_name})
    endif()
endforeach()

target_compile_definitions(${COMPONENT_LIB} PUBLIC RCL_MICROROS)
add_dependencies(${COMPONENT_LIB} libmicroros-prebuilt)
target_link_libraries(${COMPONENT_LIB} INTERFACE libmicroros-prebuilt)
