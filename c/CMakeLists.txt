cmake_minimum_required(VERSION 3.23)
project(c C)
#include_directories(${Sophus_INCLUDE_DIRS} "./functions/led.h" "functions/motor.h" "functions/loop.h" "main.h")#添加头文件目录
set(CMAKE_C_STANDARD 99)

#add_compile_definitions(VALUE_MACRO=0x10)
#file(GLOB_RECURSE FUNCTIONS_LIB_SRC
#        ./functions/*.c
#        )
#install(FILES ${FUNCTIONS_LIB_SRC} DESTINATION ./functions)
add_library(functions STATIC ${FUNCTIONS_LIB_SRC} main.c main.h
        touchpad_online_update/touchpad_update.c touchpad_online_update/touchpad_update.h
        pixart/Storage1.c pixart/Storage1.h pixart/media_combo.c pixart/media_combo.h pixart/keycode.h pixart/process_combo.c pixart/process_combo.h pixart/linkedlist.c pixart/linkedlist.h
        aes/aes_example.c aes/aes_example.h aes/aes.c aes/aes.h aes/crc16.h aes/crc16.c
        hmac_sha256/hmac_sha256.c hmac_sha256/hmac_sha256.h hmac_sha256/sha256.c hmac_sha256/sha256.h  hmac_sha256/hmac_c_example.c hmac_sha256/hmac_c_example.h
        ota/ota.h ota/ota.c
#        functions/led.c functions/led.h functions/motor.c functions/motor.h functions/loop.c functions/loop.h
        sklearn/svc/svc.c sklearn/svc/svc.h sklearn/bayes/bayes.h sklearn/bayes/bayes.c
        bit/bit.c bit/bit.h
        kalman/filter.c kalman/filter.h
        timer_task/timer_task.c timer_task/timer_task.h
        battery/battery_level.c
        battery/battery_level.h
        preset_gesture/preset_gesture.c
        preset_gesture/preset_gesture.h
        #        knn/knn.c
#        knn/knn.h
        )

add_executable(c main.c main.h touchpad_online_update/touchpad_update.c touchpad_online_update/touchpad_update.h  pixart/Storage1.c pixart/Storage1.h pixart/media_combo.c pixart/media_combo.h pixart/keycode.h pixart/process_combo.c pixart/process_combo.h  pixart/linkedlist.c pixart/linkedlist.h  aes/AES128.c aes/AES128.h aes/aes_example.c aes/aes_example.h aes/aes.c aes/aes.h hmac_sha256/hmac_sha256.c hmac_sha256/hmac_sha256.h hmac_sha256/sha256.c hmac_sha256/sha256.h sklearn/svc/svc.c sklearn/svc/svc.h sklearn/bayes/bayes.h sklearn/bayes/bayes.c bit/bit.c bit/bit.h kalman/filter.c kalman/filter.h hmac_sha256/hmac_c_example.c hmac_sha256/hmac_c_example.h aes/aes_example.c aes/aes_example.h pixart/process_combo.c pixart/process_combo.h pixart/media_combo.c pixart/media_combo.h
        timer_task/timer_task.c
        timer_task/timer_task.h
        battery/battery_level.c
        battery/battery_level.h
        preset_gesture/preset_gesture.c
        preset_gesture/preset_gesture.h
        #        knn/knn.c knn/knn.h
        )

target_link_libraries(c functions)