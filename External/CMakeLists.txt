cmake_minimum_required(VERSION 3.8)

project (External_Solution)

if((${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC))
  include(CheckSymbolExists)
  check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
  if(NOT HAVE_SNPRINTF)
  add_definitions(-Dsnprintf=_snprintf)
  endif()
endif()

add_subdirectory(DSPFilters)
add_subdirectory(whisper)

get_target_property(WHISPER_SOURCES whisper SOURCES)
list(TRANSFORM WHISPER_SOURCES PREPEND "whisper/")
add_library(whisperStatic STATIC ${WHISPER_SOURCES})
target_include_directories(whisperStatic PUBLIC "whisper/" )