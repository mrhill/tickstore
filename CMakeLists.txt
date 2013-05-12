cmake_minimum_required (VERSION 2.6)
project (tickstore)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
find_package(MySQL REQUIRED)

add_subdirectory(../babel "${CMAKE_CURRENT_BINARY_DIR}/babel")
add_subdirectory(libts)
add_subdirectory(tstest)
add_subdirectory(testmysql)
add_subdirectory(tickschleuder)

