cmake_minimum_required(VERSION 2.8.11)
project(v_repExtSurfaceReconstruction)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED COMPONENTS system)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(VREP REQUIRED)

find_package(CGAL REQUIRED COMPONENTS Core)
include(${CGAL_USE_FILE})
include(CGAL_CreateSingleSourceCGALProgram)

#find_package(PCL 1.8 REQUIRED COMPONENTS common io surface)
#include_directories(${PCL_INCLUDE_DIRS})
#link_directories(${PCL_LIBRARY_DIRS})
#add_definitions(${PCL_DEFINITIONS})

include_directories(${VREP_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/generated)

add_custom_command(OUTPUT
    ${CMAKE_SOURCE_DIR}/generated/stubs.cpp ${CMAKE_SOURCE_DIR}/generated/stubs.h
    COMMAND python ${CMAKE_SOURCE_DIR}/external/v_repStubsGen/generate.py --xml-file ${CMAKE_SOURCE_DIR}/callbacks.xml --gen-all ${CMAKE_SOURCE_DIR}/generated
    DEPENDS ${CMAKE_SOURCE_DIR}/callbacks.xml)

file(GLOB SOURCES *.cpp)
add_library(v_repExtSurfaceReconstruction SHARED ${SOURCES} ${CMAKE_SOURCE_DIR}/generated/stubs.cpp ${VREP_COMMON}/v_repLib.cpp)
target_link_libraries(v_repExtSurfaceReconstruction ${CGAL_CORE_LIBRARY} ${VREP_LIBRARIES})
