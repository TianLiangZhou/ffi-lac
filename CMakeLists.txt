cmake_minimum_required(VERSION 3.19)
project(lacffi C CXX)

# set(CMAKE_C_STANDARD 11)

message(STATUS "C compiler: ${CMAKE_C_COMPILER}, version: "
        "${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")


set(INSTALL_LIB_DIR "${PROJECT_SOURCE_DIR}/lib")



message(STATUS "CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}")


set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# 确保链接库不在系统默认安装的目录上时更改到项目lib上
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES ${CMAKE_INSTALL_RPATH} isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${INSTALL_LIB_DIR}")
endif("${isSystemDir}" STREQUAL "-1")

message(STATUS "cmake_install_rpath ${CMAKE_INSTALL_RPATH}")

set(CMAKE_CXX_FLAGS "-O3 -g -pipe -W -Wall -Wno-unused-parameter -fPIC -fpermissive -std=gnu++11")
message(STATUS "CXX ${CMAKE_CXX_FLAGS}")
if(NOT DEFINED PADDLE_C_ROOT)
    message(FATAL_ERROR "please set PADDLE_C_ROOT with -DPADDLE_C_ROOT=/path/paddle/lib")
endif()

if (IS_ABSOLUTE ${PADDLE_C_ROOT})
    set(PADDLE_C_ABS_PATH ${PADDLE_C_ROOT})
else ()
    get_filename_component(PADDLE_C_ABS_PATH ${CMAKE_BINARY_DIR}/${PADDLE_C_ROOT}/ ABSOLUTE)
endif ()


set(DEPS ${PADDLE_C_ROOT}/paddle/lib/libpaddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})

include_directories(${PADDLE_C_ABS_PATH}/paddle/include)
include_directories(include)
aux_source_directory(src SOURCE)
link_directories("${PADDLE_C_ABS_PATH}/paddle/lib")



message(STATUS "'${PROJECT_SOURCE_DIR}' output dir")

add_library(lacffi SHARED ${SOURCE})
target_link_libraries(lacffi ${DEPS})
set_target_properties(lacffi PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(lac_demo main.c)
set_target_properties(lac_demo PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(lac_demo lacffi ${DEPS})



install(TARGETS lacffi DESTINATION ${PROJECT_SOURCE_DIR}/lib)

install(TARGETS lac_demo DESTINATION ${PROJECT_SOURCE_DIR}/bin)
