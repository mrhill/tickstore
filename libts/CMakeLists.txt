include_directories (../../babel/include)
file(GLOB HEADERS "../../babel/include/*.h")
file(GLOB SRCS "*.cpp")
add_library (ts ${SRCS} ${HEADERS})
