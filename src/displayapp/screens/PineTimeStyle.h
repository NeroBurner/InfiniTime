#pragma once

#include <lvgl/src/core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
      class PineTimeStyle : public Screen {
      public:
        PineTimeStyle(DisplayApp* app,
                      Controllers::DateTime& dateTimeController,
                      Controllers::Battery& batteryController,
                      Controllers::Ble& bleController,
                      Controllers::NotificationManager& notificatioManager,
                      Controllers::Settings& settingsController,
                      Controllers::MotionController& motionController);
        ~PineTimeStyle() override;

        void Refresh() override;

      private:
        char displayedChar[5];

        uint16_t currentYear = 1970;
        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> isCharging {};
        DirtyValue<bool> bleState {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<bool> notificationState {};

        lv_obj_t* timebar;
        lv_obj_t* sidebar;
        lv_obj_t* timeDD1;
        lv_obj_t* timeDD2;
        lv_obj_t* timeAMPM;
        lv_obj_t* dateDayOfWeek;
        lv_obj_t* dateDay;
        lv_obj_t* dateMonth;
        lv_obj_t* backgroundLabel;
        lv_obj_t* batteryIcon;
        lv_obj_t* bleIcon;
        lv_obj_t* calendarOuter;
        lv_obj_t* calendarInner;
        lv_obj_t* calendarBar1;
        lv_obj_t* calendarBar2;
        lv_obj_t* calendarCrossBar1;
        lv_obj_t* calendarCrossBar2;
        lv_obj_t* notificationIcon;
        lv_obj_t* stepMeter;
        lv_meter_scale_t* stepScale;
        lv_meter_indicator_t* stepIndicator;
        lv_color_t needle_color;

        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        void SetBatteryIcon();

        lv_timer_t* taskRefresh;
      };
    }
  }
}
