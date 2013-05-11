cmake_minimum_required (VERSION 2.6)
project (tickstore)

add_subdirectory(../babel "${CMAKE_CURRENT_BINARY_DIR}/babel")
add_subdirectory(libts)
add_subdirectory(tstest)
add_subdirectory(tickschleuder)
