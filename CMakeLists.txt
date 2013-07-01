cmake_minimum_required(VERSION 2.8)
project(traffic-gen)
find_package(PkgConfig)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-long-long")


set(traffic-gen_SOURCES
  src/main.c
  src/options.c
  src/sender.c)
set(traffic-gen_HEADERS
  include/options.h
  include/sender.h)

include_directories(include/)

add_executable(traffic-gen
  ${traffic-gen_HEADERS}
  ${traffic-gen_SOURCES})
target_link_libraries(traffic-gen)
