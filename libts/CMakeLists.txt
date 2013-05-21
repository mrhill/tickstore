cmake_minimum_required(VERSION 2.6)
project(ts)

include_directories (
	../../babel/include
	${MYSQL_INCLUDE_DIR}
	${MHASH_INCLUDE_PATH}
)

file(GLOB HEADERS "../../babel/include/*.h")
file(GLOB SRCS "*.cpp")

add_library (ts ${SRCS} ${HEADERS})

IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
