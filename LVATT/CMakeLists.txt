cmake_minimum_required (VERSION 3.8)

project (LVATT)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "LVATT.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE -static DSPFilters)
