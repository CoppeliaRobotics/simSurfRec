cmake_minimum_required(VERSION 2.8.11)
project(v_repExtSurfaceReconstruction)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED COMPONENTS system)

if(NOT DEFINED ENV{VREP_ROOT})
    message(FATAL_ERROR "Environment variable VREP_ROOT is not set")
endif()
file(TO_CMAKE_PATH "$ENV{VREP_ROOT}" VREP_ROOT)

if(WIN32)
    add_definitions(-DWIN_VREP)
    add_definitions(-DNOMINMAX)
    add_definitions(-Dstrcasecmp=_stricmp)
elseif(UNIX AND NOT APPLE)
    add_definitions(-DLIN_VREP)
elseif(UNIX AND APPLE)
    add_definitions(-DMAC_VREP)
endif()

find_package(CGAL REQUIRED COMPONENTS Core)
include(${CGAL_USE_FILE})
include(CGAL_CreateSingleSourceCGALProgram)

#find_package(PCL 1.8 REQUIRED COMPONENTS common io surface)
#include_directories(${PCL_INCLUDE_DIRS})
#link_directories(${PCL_LIBRARY_DIRS})
#add_definitions(${PCL_DEFINITIONS})

include_directories($ENV{VREP_ROOT}/programming/include)
include_directories(${CMAKE_SOURCE_DIR}/generated)

add_custom_command(OUTPUT
    ${CMAKE_SOURCE_DIR}/generated/stubs.cpp ${CMAKE_SOURCE_DIR}/generated/stubs.h
    COMMAND python ${CMAKE_SOURCE_DIR}/external/v_repStubsGen/generate.py --xml-file ${CMAKE_SOURCE_DIR}/callbacks.xml --gen-all ${CMAKE_SOURCE_DIR}/generated
    DEPENDS ${CMAKE_SOURCE_DIR}/callbacks.xml)

file(GLOB SOURCES *.cpp)
add_library(v_repExtSurfaceReconstruction SHARED ${SOURCES} ${CMAKE_SOURCE_DIR}/generated/stubs.cpp $ENV{VREP_ROOT}/programming/common/v_repLib.cpp)

target_link_libraries(v_repExtSurfaceReconstruction ${CGAL_CORE_LIBRARY})
