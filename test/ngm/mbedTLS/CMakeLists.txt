project(mbedtls)

file(GLOB SOURCES *.c)

file(GLOB INCLUDES *.h include/mbedtls/*.h)

include_directories(
    include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -W -Wdeclaration-after-statement -O2 -D_FILE_OFFSET_BITS=64 -FPIC -fpic")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${INCLUDES})
