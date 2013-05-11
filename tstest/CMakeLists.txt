include_directories (../../babel/include)
include_directories (../libts)
link_directories (../../babel)
link_directories (../libts)
find_package (Threads)

add_executable (tstest tstest.cpp)

target_link_libraries (tstest ts babel ${CMAKE_THREAD_LIBS_INIT} -lrt)
