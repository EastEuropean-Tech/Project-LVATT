cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("Project LVATT")

if(WIN32)
message(STATUS "Using /MT flag on windows")
add_compile_options (/MT)
elseif(UNIX)
message(STATUS "Using -static flag on linux")
add_compile_options ("-static")
endif()


message(STATUS "CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")

set(OPENSSL_USE_STATIC_LIBS TRUE CACHE BOOL "Statically Linking Open SSL")
option(BUILD_SHARED_LIBS "Link Statically" OFF)
option(HTTPLIB_REQUIRE_OPENSSL "Require SSL" ON)

# Include sub-projects.
add_subdirectory (External)
add_subdirectory (LVATT)