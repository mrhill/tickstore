include_directories (../../babel/include ../libts ${MHASH_INCLUDE_PATH} ${MYSQL_INCLUDE_DIR})
link_directories (../../babel ../libts)
find_package (Threads)

file(GLOB HEADERS "*.h")
file(GLOB CPP_SOURCES "*.cpp")

add_executable (tickschleuder ${C_SOURCES} ${CPP_SOURCES} ${HEADERS})

target_link_libraries (tickschleuder ts babel ${CMAKE_THREAD_LIBS_INIT} -lrt ${MYSQL_LIBRARY} -lmhash)
