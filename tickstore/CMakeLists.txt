include_directories (../../babel/include ../libts ${MHASH_INCLUDE_PATH})
link_directories (../../babel ../libts)
find_package (Threads)

file(GLOB HEADERS "*.h")
file(GLOB CPP_SOURCES "*.cpp")

add_executable (tickstore ${C_SOURCES} ${CPP_SOURCES} ${HEADERS})

target_link_libraries (tickstore ts babel ${CMAKE_THREAD_LIBS_INIT} -lrt ${MYSQL_LIBRARY} -lmhash)
