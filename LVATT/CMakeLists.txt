cmake_minimum_required (VERSION 3.8)

project (LVATT)

add_executable (${PROJECT_NAME} "LVATT.cpp" "Headers/AudioTranscribing.hpp" "Headers/Common.hpp" "Headers/SignalProcessing.hpp" "Headers/WAV.hpp")
target_link_libraries(${PROJECT_NAME} -static DSPFilters)
target_link_libraries(${PROJECT_NAME} -static whisper)
target_link_libraries(${PROJECT_NAME} -static httplib)

set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# make executable static

#install(TARGETS ${PROJECT_NAME} DESTINATION bin)