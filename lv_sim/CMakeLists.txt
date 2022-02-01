cmake_minimum_required(VERSION 3.10...${CMAKE_VERSION})

# try to extract the PineTime project version
file(READ "../CMakeLists.txt" main_cmake)
string(REGEX MATCH "project\\(pinetime VERSION ([0-9]*\.[0-9]*\.[0-9]*)" _ ${main_cmake})
set(PROJECT_VERSION ${CMAKE_MATCH_1})
message(STATUS "InfiniTime PROJECT_VERSION extracted: ${PROJECT_VERSION}")

project(lv_sim VERSION ${PROJECT_VERSION} LANGUAGES C CXX)
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
string(COMPARE EQUAL "${CMAKE_CXX_STANDARD}" "" no_cmake_cxx_standard_set)
if(no_cmake_cxx_standard_set)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    message(STATUS "Using default C++ standard ${CMAKE_CXX_STANDARD}")
else()
    message(STATUS "Using user specified C++ standard ${CMAKE_CXX_STANDARD}")
endif()
set(CMAKE_C_STANDARD 11)#C11

file(GLOB_RECURSE INCLUDES "lv_drivers/*.h" "../src/libs/lvgl/src/*.h"  "./*.h" )
file(GLOB_RECURSE SOURCES  "lv_drivers/*.c" "../src/libs/lvgl/src/*.c" )

add_executable(pinesim main.cpp ${SOURCES} ${INCLUDES})

# add simulator files
target_sources(pinesim PUBLIC
  sim/displayapp/LittleVgl.h
  sim/displayapp/LittleVgl.cpp
  sim/displayapp/screens/Missing.h
  sim/displayapp/screens/Missing.cpp
  sim/components/battery/BatteryController.h
  sim/components/battery/BatteryController.cpp
  sim/components/ble/AlertNotificationService.h
  sim/components/ble/AlertNotificationService.cpp
  sim/components/ble/MusicService.h
  sim/components/ble/MusicService.cpp
  sim/components/ble/NavigationService.h
  sim/components/ble/NavigationService.cpp
  sim/components/ble/NimbleController.h
  sim/components/ble/NimbleController.cpp
  sim/components/brightness/BrightnessController.h
  sim/components/brightness/BrightnessController.cpp
  sim/components/firmwarevalidator/FirmwareValidator.h
  sim/components/firmwarevalidator/FirmwareValidator.cpp
  sim/components/fs/FS.h
  sim/components/fs/FS.cpp
  sim/components/heartrate/HeartRateController.h
  sim/components/heartrate/HeartRateController.cpp
  sim/components/motion/MotionController.h
  sim/components/motion/MotionController.cpp
  sim/components/motor/MotorController.h
  sim/components/motor/MotorController.cpp
  sim/drivers/Watchdog.h
  sim/drivers/Watchdog.cpp
  sim/drivers/Bma421.h
  sim/drivers/Bma421.cpp
  sim/drivers/Cst816s.h
  sim/drivers/Cst816s.cpp
  sim/drivers/Hrs3300.h
  sim/drivers/Hrs3300.cpp
  sim/drivers/SpiMaster.h
  sim/drivers/SpiMaster.cpp
  sim/drivers/TwiMaster.h
  sim/drivers/TwiMaster.cpp
  sim/drivers/SpiNorFlash.h
  sim/drivers/SpiNorFlash.cpp
  sim/heartratetask/HeartRateTask.h
  sim/heartratetask/HeartRateTask.cpp
  # FreeRTOS
  sim/FreeRTOS.h
  sim/FreeRTOS.cpp
  sim/task.h
  sim/task.cpp
  sim/timers.h
  sim/timers.cpp
  sim/queue.h
  sim/queue.cpp
  # src/FreeRTOS
  sim/portmacro_cmsis.h
  sim/portmacro_cmsis.cpp
  # nrf
  sim/libraries/log/nrf_log.h
  sim/libraries/delay/nrf_delay.h
  sim/libraries/delay/nrf_delay.cpp
  sim/nrfx/nrfx_log.h
  sim/nrfx/drivers/include/nrfx_twi.h
  sim/nrfx/hal/nrf_gpio.h
  sim/nrfx/hal/nrf_gpio.cpp
  sim/nrfx/hal/nrfx_gpiote.h # includes hal/nrf_gpio.h
  sim/nrfx/hal/nrf_rtc.h
  sim/nrfx/hal/nrf_rtc.cpp
  # nrf/components/libraries/timer
  sim/libraries/timer/app_timer.h
  sim/libraries/timer/app_timer.cpp
  sim/libraries/gpiote/app_gpiote.h # includes hal/nrf_gpio.h
  )
target_include_directories(pinesim PRIVATE "sim")
target_include_directories(pinesim PRIVATE "sim/libraries/log") # for nrf_log.h
target_include_directories(pinesim PRIVATE "sim/libraries/timer") # for app_timer.h
target_include_directories(pinesim PRIVATE "sim/nrfx") # for nrfx_log.h and others
target_include_directories(pinesim PRIVATE "sim/nrfx/hal") # for nrfx_log.h

target_compile_definitions(pinesim PRIVATE LV_CONF_INCLUDE_SIMPLE)
target_sources(pinesim PUBLIC
  # LVGL Fonts
  ../src/libs/lvgl/src/lv_font/lv_font_montserrat_14.c
  ../src/libs/lvgl/src/lv_font/lv_font_montserrat_18.c
  ../src/libs/lvgl/src/lv_font/lv_font_montserrat_22.c
  ../src/libs/lvgl/src/lv_font/lv_font_montserrat_28.c

  ../src/displayapp/fonts/lv_font_navi_80.c

  ../src/displayapp/fonts/jetbrains_mono_extrabold_compressed.c
  ../src/displayapp/fonts/jetbrains_mono_bold_20.c
  ../src/displayapp/fonts/jetbrains_mono_76.c
  ../src/displayapp/fonts/jetbrains_mono_42.c
  ../src/displayapp/fonts/lv_font_sys_48.c
  ../src/displayapp/fonts/open_sans_light.c
)

target_include_directories(pinesim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(pinesim PRIVATE "../src/libs")
target_include_directories(pinesim PRIVATE "lv_drivers")

# add dates library
target_include_directories(pinesim SYSTEM PRIVATE "../src/libs/date/includes")
# add Screens and other files directly from InfiniTime sources
target_include_directories(pinesim PRIVATE "../src")
target_sources(pinesim PUBLIC
  ../src/BootloaderVersion.h
  ../src/BootloaderVersion.cpp
  ../src/displayapp/Colors.h
  ../src/displayapp/Colors.cpp
  ../src/displayapp/DisplayApp.h
  ../src/displayapp/DisplayApp.cpp
  ../src/displayapp/lv_pinetime_theme.h
  ../src/displayapp/lv_pinetime_theme.c
  ../src/displayapp/screens/Alarm.h
  ../src/displayapp/screens/Alarm.cpp
  ../src/displayapp/screens/ApplicationList.h
  ../src/displayapp/screens/ApplicationList.cpp
  ../src/displayapp/screens/FirmwareValidation.h
  ../src/displayapp/screens/FirmwareValidation.cpp
  ../src/displayapp/screens/FirmwareUpdate.h
  ../src/displayapp/screens/FirmwareUpdate.cpp
  ../src/displayapp/screens/HeartRate.h
  ../src/displayapp/screens/HeartRate.cpp
  ../src/displayapp/screens/PassKey.h
  ../src/displayapp/screens/PassKey.cpp
  ../src/displayapp/screens/Tile.h
  ../src/displayapp/screens/Tile.cpp
  ../src/displayapp/screens/Screen.h
  ../src/displayapp/screens/Screen.cpp
  ../src/displayapp/screens/ScreenList.h
  ../src/displayapp/screens/Label.h
  ../src/displayapp/screens/Label.cpp
  ../src/displayapp/screens/List.h
  ../src/displayapp/screens/List.cpp
  ../src/displayapp/screens/Paddle.h
  ../src/displayapp/screens/Paddle.cpp
  ../src/displayapp/screens/InfiniPaint.h
  ../src/displayapp/screens/InfiniPaint.cpp
  ../src/displayapp/screens/Steps.h
  ../src/displayapp/screens/Steps.cpp
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
  ../src/displayapp/screens/WatchFaceTerminal.h
  ../src/displayapp/screens/WatchFaceTerminal.cpp
  ../src/displayapp/icons/bg_clock.c # used by WatchFaceAnalog.cpp
  ../src/displayapp/screens/Music.h
  ../src/displayapp/screens/Music.cpp
  ../src/displayapp/screens/PineTimeStyle.h
  ../src/displayapp/screens/PineTimeStyle.cpp
  ../src/displayapp/screens/Clock.h
  ../src/displayapp/screens/Clock.cpp
  ../src/displayapp/screens/BatteryIcon.h
  ../src/displayapp/screens/BatteryIcon.cpp
  ../src/displayapp/screens/BleIcon.h
  ../src/displayapp/screens/BleIcon.cpp
  ../src/displayapp/screens/Error.h
  ../src/displayapp/screens/Error.cpp
  ../src/displayapp/screens/Motion.h
  ../src/displayapp/screens/Motion.cpp
  ../src/displayapp/screens/Navigation.h
  ../src/displayapp/screens/Navigation.cpp
  ../src/displayapp/screens/Notifications.h
  ../src/displayapp/screens/Notifications.cpp
  ../src/displayapp/screens/NotificationIcon.h
  ../src/displayapp/screens/NotificationIcon.cpp
  ../src/displayapp/screens/StopWatch.h
  ../src/displayapp/screens/StopWatch.cpp
  ../src/displayapp/screens/Styles.h
  ../src/displayapp/screens/Styles.cpp
  ../src/displayapp/screens/SystemInfo.h
  ../src/displayapp/screens/SystemInfo.cpp
  ../src/displayapp/screens/Timer.h
  ../src/displayapp/screens/Timer.cpp
  ../src/displayapp/screens/settings/QuickSettings.cpp
  ../src/displayapp/screens/settings/QuickSettings.h
  ../src/displayapp/screens/settings/SettingChimes.cpp
  ../src/displayapp/screens/settings/SettingChimes.h
  ../src/displayapp/screens/settings/SettingDisplay.cpp
  ../src/displayapp/screens/settings/SettingDisplay.h
  ../src/displayapp/screens/settings/Settings.cpp
  ../src/displayapp/screens/settings/SettingSetDate.h
  ../src/displayapp/screens/settings/SettingSetDate.cpp
  ../src/displayapp/screens/settings/SettingSetTime.h
  ../src/displayapp/screens/settings/SettingSetTime.cpp
  ../src/displayapp/screens/settings/Settings.h
  ../src/displayapp/screens/settings/SettingShakeThreshold.cpp
  ../src/displayapp/screens/settings/SettingShakeThreshold.h
  ../src/displayapp/screens/settings/SettingSteps.cpp
  ../src/displayapp/screens/settings/SettingSteps.h
  ../src/displayapp/screens/settings/SettingTimeFormat.cpp
  ../src/displayapp/screens/settings/SettingTimeFormat.h
  ../src/displayapp/screens/settings/SettingWakeUp.cpp
  ../src/displayapp/screens/settings/SettingWakeUp.h
  ../src/displayapp/screens/settings/SettingWatchFace.cpp
  ../src/displayapp/screens/settings/SettingWatchFace.h
  ../src/buttonhandler/ButtonHandler.h
  ../src/buttonhandler/ButtonHandler.cpp
  ../src/components/alarm/AlarmController.h
  ../src/components/alarm/AlarmController.cpp
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
  ../src/drivers/PinMap.h
  ../src/drivers/Spi.h
  ../src/drivers/Spi.cpp
  ../src/drivers/St7789.h
  ../src/drivers/St7789.cpp
  ../src/touchhandler/TouchHandler.h
  ../src/touchhandler/TouchHandler.cpp
  ../src/systemtask/SystemTask.h
  ../src/systemtask/SystemTask.cpp
  )


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# Special case for SDL2 dependency, goal is to find a config that exports SDL2::SDL2 target
# libsdl2-dev has a `sdl2-config.cmake` that doesn't export this, but vcpkg does..
find_package(SDL2 CONFIG QUIET)
if(NOT TARGET SDL2::SDL2)
  find_package(SDL2 MODULE REQUIRED)
endif()
target_link_libraries(pinesim PRIVATE SDL2::SDL2)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE PROJECT_GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(VERSION_EDIT_WARNING "// Do not edit this file, it is automatically generated by CMAKE!")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../src/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/Version.h)

target_include_directories(pinesim PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
