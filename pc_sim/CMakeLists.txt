cmake_minimum_required(VERSION 3.16)
project(roon_knob_pc C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(CURL REQUIRED)

add_executable(roon_knob_pc
  main.c
  ui.c
  components/net_client/curl_client.c
)

target_include_directories(roon_knob_pc PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/components/net_client
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(roon_knob_pc PRIVATE CURL::libcurl)

install(TARGETS roon_knob_pc DESTINATION bin)
