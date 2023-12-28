cmake_minimum_required(VERSION 3.23)
project(c C)
#include_directories(${Sophus_INCLUDE_DIRS} "./functions/led.h" "functions/motor.h" "functions/loop.h" "main.h")#添加头文件目录
set(CMAKE_C_STANDARD 11)

#add_compile_definitions(VALUE_MACRO=0x10)
#file(GLOB_RECURSE FUNCTIONS_LIB_SRC
#        ./functions/*.c
#        )
#install(FILES ${FUNCTIONS_LIB_SRC} DESTINATION ./functions)
add_library(functions STATIC ${FUNCTIONS_LIB_SRC} main.h functions/led.c functions/led.h functions/motor.c functions/motor.h functions/loop.c functions/loop.h)

add_executable(c main.c main.h)
#add_executable(led functions/led.c functions/led.h )

target_link_libraries(c functions)