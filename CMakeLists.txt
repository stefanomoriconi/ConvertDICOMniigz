cmake_minimum_required(VERSION 3.3)
project(ConvertDICOM)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(ConvertDICOM ConvertDICOM.cpp ConvertDICOM.h)
target_link_libraries(ConvertDICOM ${ITK_LIBRARIES})
