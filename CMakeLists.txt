cmake_minimum_required(VERSION 2.8.11)
project(v_repExtSurfaceReconstruction)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Boost REQUIRED COMPONENTS system)

find_package(CGAL REQUIRED COMPONENTS Core)
include(${CGAL_USE_FILE})
include(CGAL_CreateSingleSourceCGALProgram)

#find_package(PCL 1.8 REQUIRED COMPONENTS common io surface)
#include_directories(${PCL_INCLUDE_DIRS})
#link_directories(${PCL_LIBRARY_DIRS})
#add_definitions(${PCL_DEFINITIONS})

include_directories($ENV{VREP_ROOT}/programming/include)

if((EXISTS ${CMAKE_SOURCE_DIR}/stubs.h) AND (EXISTS ${CMAKE_SOURCE_DIR}/stubs.cpp))
  MESSAGE(STATUS "using the prebuilt stubs (stubs.h and stubs.cpp)")
else()
  MESSAGE(STATUS "generating the stubs (stubs.h and stubs.cpp)")
  add_custom_command(OUTPUT
    ${CMAKE_SOURCE_DIR}/stubs.cpp ${CMAKE_SOURCE_DIR}/stubs.h
    COMMAND python ${CMAKE_SOURCE_DIR}/external/v_repStubsGen/main.py -H ${CMAKE_SOURCE_DIR}/stubs.h -C ${CMAKE_SOURCE_DIR}/stubs.cpp ${CMAKE_SOURCE_DIR}/callbacks.xml
    DEPENDS ${CMAKE_SOURCE_DIR}/callbacks.xml)
endif()

if(NOT EXISTS ${CMAKE_SOURCE_DIR}/reference.html)
  MESSAGE(STATUS "generating reference.html")
  add_custom_command(OUTPUT
    ${CMAKE_SOURCE_DIR}/reference.html
    COMMAND xsltproc -o ${CMAKE_SOURCE_DIR}/reference.html ${CMAKE_SOURCE_DIR}/callbacks.xml ${CMAKE_SOURCE_DIR}/external/v_repStubsGen/xsl/reference.xsl
    DEPENDS ${CMAKE_SOURCE_DIR}/callbacks.xml)
  add_custom_target(gendoc ALL DEPENDS ${CMAKE_SOURCE_DIR}/reference.html)
endif()

file(GLOB SOURCES *.cpp)
list(REMOVE_ITEM SOURCES stubs.cpp)
add_library(v_repExtSurfaceReconstruction SHARED ${SOURCES} stubs.cpp $ENV{VREP_ROOT}/programming/common/v_repLib.cpp)

target_link_libraries(v_repExtSurfaceReconstruction ${CGAL_CORE_LIBRARY})

# Install library
#install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

# Install library headers
#file(GLOB HEADERS include/*.h)
#install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

