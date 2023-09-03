cmake_minimum_required (VERSION 3.8)

project (DSPFilters)

include(CheckSymbolExists)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
if(NOT HAVE_SNPRINTF)
add_definitions(-Dsnprintf=_snprintf)
endif()

file(GLOB SOURCE_LIB "./source/*.cpp")

add_library(${PROJECT_NAME} ${SOURCE_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC "./include/")

#set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)