include_directories (
	../../babel/include
	${MYSQL_INCLUDE_DIR}
)
file(GLOB HEADERS "../../babel/include/*.h")
file(GLOB SRCS "*.cpp")
add_library (ts ${SRCS} ${HEADERS})
