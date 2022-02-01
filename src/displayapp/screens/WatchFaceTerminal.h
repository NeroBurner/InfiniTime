#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
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

      class WatchFaceTerminal : public Screen {
      public:
        WatchFaceTerminal(DisplayApp* app,
                          Controllers::DateTime& dateTimeController,
                          Controllers::Battery& batteryController,
                          Controllers::Ble& bleController,
                          Controllers::NotificationManager& notificatioManager,
                          Controllers::Settings& settingsController,
                          Controllers::HeartRateController& heartRateController,
                          Controllers::MotionController& motionController);
        ~WatchFaceTerminal() override;

        void Refresh() override;

        void OnObjectEvent(lv_obj_t* pObj, lv_event_t i);

      private:
        char displayedChar[8];

        uint16_t currentYear = 1970;
        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<int> batteryPercentRemaining {};
        DirtyValue<bool> bleState {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<uint8_t> heartbeat {};
        DirtyValue<bool> heartbeatRunning {};
        DirtyValue<bool> notificationState {};

        lv_obj_t* label_time;
        lv_obj_t* label_date;
        lv_obj_t* label_prompt_1;
        lv_obj_t* label_prompt_2;
        lv_obj_t* backgroundLabel;
        lv_obj_t* batteryIcon;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* batteryPercent;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* heartbeatBpm;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* connectState;
        lv_obj_t* bleValue;

        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
      };
    }
  }
}