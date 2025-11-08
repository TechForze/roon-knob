cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(roon_knob)

set(COMPONENT_SRCS
    "main/main.c"
    "main/ui.c"
    "main/encoder_input.c"
    "main/net_client_idf.c"
    "main/config_store.c"
    "main/mdns_client.c"
)

set(COMPONENT_ADD_INCLUDEDIRS "main")

set_property(TARGET ${COMPONENT_LIB} PROPERTY C_STANDARD 11)
