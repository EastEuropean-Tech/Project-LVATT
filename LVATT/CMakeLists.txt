cmake_minimum_required (VERSION 3.8)

project (LVATT)

add_executable (${PROJECT_NAME} "LVATT.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE -static DSPFilters)

# make executable static
if(WIN32)
target_compile_options(${PROJECT_NAME} PUBLIC /MT)
target_link_options(${PROJECT_NAME} PUBLIC /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
endif(WIN32)