cmake_minimum_required(VERSION 3.0)
project(lacffi C CXX)

option(WITH_STATIC_LIB "Compile demo with static/shared library, default use static."  OFF)

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
if(NOT DEFINED PADDLE_ROOT)
    message(FATAL_ERROR "please set PADDLE_ROOT with -DPADDLE_ROOT=/path/paddle/lib")
endif()

if (IS_ABSOLUTE ${PADDLE_ROOT})
    set(PADDLE_ABS_PATH ${PADDLE_ROOT})
else ()
    get_filename_component(PADDLE_ABS_PATH ${CMAKE_BINARY_DIR}/${PADDLE_ROOT}/ ABSOLUTE)
endif ()


include_directories(${PADDLE_ABS_PATH}/paddle/include)
set(PADDLE_LIB_THIRD_PARTY_PATH "${PADDLE_ABS_PATH}/third_party/install/")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}protobuf/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}glog/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}gflags/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}xxhash/include")

link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}protobuf/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}glog/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}gflags/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}xxhash/lib")
link_directories("${PADDLE_ABS_PATH}/paddle/lib")

# add mkldnn library if it exitsts
set (mkldnn_inc_path ${PADDLE_ABS_PATH}/third_party/install/mkldnn/include)
set (mkldnn_lib_path ${PADDLE_ABS_PATH}/third_party/install/mkldnn/lib)
if (EXISTS ${mkldnn_inc_path} AND EXISTS ${mkldnn_lib_path})
    message(STATUS "MKLDNN lib found.")
    include_directories(${PADDLE_ABS_PATH}/third_party/install/mkldnn/include)
    if (WIN32)
        set (MKLDNN_LIB ${mkldnn_lib_path}/mkldnn.lib)
    else (WIN32)
        set (MKLDNN_LIB ${mkldnn_lib_path}/libmkldnn.so.0)
    endif (WIN32)
else ()
    message(STATUS "MKLDNN lib not found.")
endif()

# add mklml library if it exists
set (mklml_inc_path ${PADDLE_ABS_PATH}/third_party/install/mklml/include)
set (mklml_lib_path ${PADDLE_ABS_PATH}/third_party/install/mklml/lib)
if (EXISTS ${mklml_inc_path} AND EXISTS ${mklml_lib_path})
    message(STATUS "MKLML lib found.")
    include_directories(${PADDLE_ABS_PATH}/third_party/install/mklml/include)
    if(WIN32)
        set(MATH_LIB ${mklml_lib_path}/mklml${CMAKE_STATIC_LIBRARY_SUFFIX}
                ${mklml_lib_path}/libiomp5md${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(MATH_LIB ${mklml_lib_path}/libmklml_intel${CMAKE_SHARED_LIBRARY_SUFFIX}
                ${mklml_lib_path}/libiomp5${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
else ()
    message(STATUS "MKLML lib not found.")
endif()

if(WITH_STATIC_LIB)
    set(DEPS ${PADDLE_ABS_PATH}/paddle/lib/libpaddle_inference${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    if(WIN32)
        set(DEPS ${PADDLE_ABS_PATH}/paddle/lib/libpaddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
        set(DEPS ${PADDLE_ABS_PATH}/paddle/lib/libpaddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
endif()

if (NOT WIN32)
    set(EXTERNAL_LIB "-ldl -lpthread")
    set(DEPS ${DEPS}
            ${MATH_LIB} ${MKLDNN_LIB}
            glog gflags protobuf xxhash
            ${EXTERNAL_LIB})
else()
    set(DEPS ${DEPS}
            ${MATH_LIB} ${MKLDNN_LIB}
            glog gflags_static libprotobuf  xxhash ${EXTERNAL_LIB})
    set(DEPS ${DEPS} libcmt shlwapi.lib)
endif(NOT WIN32)

message(STATUS "deps ${DEPS}")

include_directories(include)
aux_source_directory(src SOURCE)
message(STATUS "'${PROJECT_SOURCE_DIR}' output dir")

add_library(lacffi SHARED ${SOURCE})
target_link_libraries(lacffi ${DEPS})
set_target_properties(lacffi PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(lac_demo main.c)
set_target_properties(lac_demo PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(lac_demo lacffi ${DEPS})

add_executable(lac_cpp_demo main.cpp)
set_target_properties(lac_cpp_demo PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(lac_cpp_demo lacffi ${DEPS})


install(TARGETS lacffi DESTINATION ${PROJECT_SOURCE_DIR}/lib)

install(TARGETS lac_demo DESTINATION ${PROJECT_SOURCE_DIR}/bin)

install(TARGETS lac_cpp_demo DESTINATION ${PROJECT_SOURCE_DIR}/bin)
