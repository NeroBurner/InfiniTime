cmake_minimum_required(VERSION 3.10...${CMAKE_VERSION})
project(lv_sim C CXX)
set(CMAKE_C_STANDARD 11)#C11
set(CMAKE_CXX_STANDARD 17)#C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE INCLUDES "lv_drivers/*.h" "../src/libs/lvgl/src/*.h"  "./*.h" )
file(GLOB_RECURSE SOURCES  "lv_drivers/*.c" "../src/libs/lvgl/src/*.c" )

add_executable(main main.cpp mouse_cursor_icon.c ${SOURCES} ${INCLUDES})
#message("add_executable(main main.c mouse_cursor_icon.c ${SOURCES} ${INCLUDES})")

target_sources(main PUBLIC
  sim/displayapp/DisplayApp.h
  sim/displayapp/DisplayApp.cpp
  sim/displayapp/LittleVgl.h
  sim/displayapp/LittleVgl.cpp
  sim/components/battery/BatteryController.h
  sim/components/battery/BatteryController.cpp
  sim/components/ble/MusicService.h
  sim/components/ble/MusicService.cpp
  sim/components/brightness/BrightnessController.h
  sim/components/brightness/BrightnessController.cpp
  sim/components/fs/FS.h
  sim/components/fs/FS.cpp
  sim/components/heartrate/HeartRateController.h
  sim/components/heartrate/HeartRateController.cpp
  sim/components/motion/MotionController.h
  sim/components/motion/MotionController.cpp
  sim/components/motor/MotorController.h
  sim/components/motor/MotorController.cpp
  sim/systemtask/SystemTask.h
  sim/systemtask/SystemTask.cpp
  # FreeRTOS
  sim/FreeRTOS.h
  sim/task.h
  sim/tasks.cpp
  # src/FreeRTOS
  sim/portmacro_cmsis.h
  # nrf
  sim/libraries/log/nrf_log.h
  # nrf/components/libraries/timer
  sim/libraries/timer/app_timer.h
  sim/libraries/timer/app_timer.cpp
  )
target_include_directories(main PRIVATE "sim")
target_include_directories(main PRIVATE "sim/displayapp")
target_include_directories(main PRIVATE "sim/displayapp/screens")
target_include_directories(main PRIVATE "sim/libraries/timer") # for app_timer.h

target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_sources(main PUBLIC
  # LVGL Fonts
  ../src/libs/lvgl/src/font/lv_font_montserrat_14.c
  ../src/libs/lvgl/src/font/lv_font_montserrat_18.c
  ../src/libs/lvgl/src/font/lv_font_montserrat_22.c
  ../src/libs/lvgl/src/font/lv_font_montserrat_28.c

  ../src/displayapp/fonts/lv_font_navi_80.c

  ../src/displayapp/fonts/jetbrains_mono_extrabold_compressed.c
  ../src/displayapp/fonts/jetbrains_mono_bold_20.c
  ../src/displayapp/fonts/jetbrains_mono_76.c
  ../src/displayapp/fonts/jetbrains_mono_42.c
  ../src/displayapp/fonts/lv_font_sys_48.c
  ../src/displayapp/fonts/open_sans_light.c
)

target_include_directories(main PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(main PRIVATE "../src/libs")
target_include_directories(main PRIVATE "lv_drivers")

find_package(SDL2 REQUIRED SDL2)
target_link_libraries(main PRIVATE SDL2::SDL2)


# freeRTOS headers
#target_include_directories(main PRIVATE "../src") # include for FreeRTOSConfig.h
#target_include_directories(main PRIVATE "../src/FreeRTOS") # include for FreeRTOSConfig.h
#target_sources(main PRIVATE
#  ../src/FreeRTOS/port_cmsis_systick.c
#  ../src/FreeRTOS/port_cmsis.c
#  ../src/FreeRTOS/port.c)
##target_include_directories(main PRIVATE "/home/nero/repos/pinetime/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/include")

# add dates library
target_include_directories(main SYSTEM PRIVATE "../src/libs/date/includes")
# add paddle
target_include_directories(main PRIVATE "../src")
target_include_directories(main PRIVATE "../src/displayapp/screens")
target_sources(main PUBLIC
  #../src/displayapp/LittleVgl.h
  #../src/displayapp/LittleVgl.cpp
  ../src/displayapp/screens/Screen.h
  ../src/displayapp/screens/Screen.cpp
  ../src/displayapp/lv_pinetime_theme.h
  ../src/displayapp/lv_pinetime_theme.c
  ../src/displayapp/screens/Paddle.h
  ../src/displayapp/screens/Paddle.cpp
  ../src/displayapp/screens/InfiniPaint.h
  ../src/displayapp/screens/InfiniPaint.cpp
  ../src/displayapp/screens/Twos.h
  ../src/displayapp/screens/Twos.cpp
  ../src/displayapp/screens/Metronome.h
  ../src/displayapp/screens/Metronome.cpp
  ../src/displayapp/screens/Meter.h
  ../src/displayapp/screens/Meter.cpp
  ../src/displayapp/screens/BatteryInfo.h
  ../src/displayapp/screens/BatteryInfo.cpp
  ../src/displayapp/screens/FlashLight.h
  ../src/displayapp/screens/FlashLight.cpp
  ../src/displayapp/screens/WatchFaceDigital.h
  ../src/displayapp/screens/WatchFaceDigital.cpp
  ../src/displayapp/screens/WatchFaceAnalog.h
  ../src/displayapp/screens/WatchFaceAnalog.cpp
  ../src/displayapp/icons/bg_clock.c # used by WatchFaceAnalog.cpp
  ../src/displayapp/screens/Music.h
  ../src/displayapp/screens/Music.cpp
  ../src/displayapp/screens/PineTimeStyle.h
  ../src/displayapp/screens/PineTimeStyle.cpp
  ../src/displayapp/Colors.h
  ../src/displayapp/Colors.cpp
  ../src/displayapp/screens/Clock.h
  ../src/displayapp/screens/Clock.cpp
  ../src/displayapp/screens/BatteryIcon.h
  ../src/displayapp/screens/BatteryIcon.cpp
  ../src/displayapp/screens/BleIcon.h
  ../src/displayapp/screens/BleIcon.cpp
  ../src/displayapp/screens/NotificationIcon.h
  ../src/displayapp/screens/NotificationIcon.cpp
  ../src/displayapp/screens/settings/QuickSettings.h
  ../src/displayapp/screens/settings/QuickSettings.cpp
  # components we can use unmodified
  ../src/components/ble/BleController.h
  ../src/components/ble/BleController.cpp
  ../src/components/datetime/DateTimeController.h
  ../src/components/datetime/DateTimeController.cpp
  ../src/components/settings/Settings.h
  ../src/components/settings/Settings.cpp
  ../src/components/ble/NotificationManager.h
  ../src/components/ble/NotificationManager.cpp
  ../src/components/timer/TimerController.h
  ../src/components/timer/TimerController.cpp
  ../src/components/alarm/AlarmController.h
  ../src/components/alarm/AlarmController.cpp
  )
