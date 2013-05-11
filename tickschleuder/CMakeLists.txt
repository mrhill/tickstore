include_directories (../../babel/include)
include_directories (../libts)
link_directories (../../babel)
link_directories (../libts)
find_package (Threads)

file(GLOB HEADERS "*.h")
file(GLOB CPP_SOURCES "*.cpp")
add_executable (tickschleuder ${C_SOURCES} ${CPP_SOURCES} ${HEADERS})

target_link_libraries (tickschleuder ts babel ${CMAKE_THREAD_LIBS_INIT} -lrt)
