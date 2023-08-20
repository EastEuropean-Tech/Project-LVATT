cmake_minimum_required(VERSION 3.27)

project (DSPFilters CXX)

set(CMAKE_BUILD_TYPE Release)

set(MYFLAGS "/O2 /WX- /MT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYFLAGS}" CACHE STRING "CXX Compiler flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MYFLAGS}" CACHE STRING "CXX Compiler Release flags" FORCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MYFLAGS}" CACHE STRING "C Compiler flags" FORCE)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${MYFLAGS}" CACHE STRING "C Compiler Release flags" FORCE)
include(CheckSymbolExists)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
if(NOT HAVE_SNPRINTF)
add_definitions(-Dsnprintf=_snprintf)
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source SOURCE_LIB)

add_library(${PROJECT_NAME} STATIC ${SOURCE_LIB})

target_include_directories(${PROJECT_NAME} 
PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")