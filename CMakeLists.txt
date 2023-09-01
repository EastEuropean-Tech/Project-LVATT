cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("Project LVATT")

if(WIN32)
add_compile_options (/MT)
endif(WIN32)

message(STATUS "CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")

option(BUILD_SHARED_LIBS "Link Statically" OFF)
option(HTTPLIB_USE_OPENSSL_IF_AVAILABLE "Link Statically" ON)

# Include sub-projects.
add_subdirectory (External)
add_subdirectory (LVATT)