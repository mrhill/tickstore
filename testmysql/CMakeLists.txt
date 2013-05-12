include_directories (
	../../babel/include
	../libts
)
link_directories (
	../../babel
	../libts
)

find_package (Threads)

add_executable (testmysql testmysql.cpp)

target_link_libraries (testmysql
	ts
	babel
	${CMAKE_THREAD_LIBS_INIT}
	-lrt
	${MYSQL_LIBRARY}
)

