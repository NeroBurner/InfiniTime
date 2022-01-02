/*
 * This file is part of the Infinitime distribution (https://github.com/InfiniTimeOrg/Infinitime).
 * Copyright (c) 2021 Kieran Cawthray.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * PineTimeStyle watchface for Infinitime created by Kieran Cawthray
 * Based on WatchFaceDigital
 * Style/layout copied from TimeStyle for Pebble by Dan Tilden (github.com/tilden)
 */

#include "displayapp/screens/PineTimeStyle.h"
#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include <displayapp/Colors.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_event_t *event) {
    auto* screen = static_cast<PineTimeStyle*>(lv_event_get_user_data(event));
    lv_obj_t *obj = lv_event_get_target(event);
    screen->UpdateSelected(obj, event);
  }
}

PineTimeStyle::PineTimeStyle(DisplayApp* app,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Battery& batteryController,
                             Controllers::Ble& bleController,
                             Controllers::NotificationManager& notificatioManager,
                             Controllers::Settings& settingsController,
                             Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    motionController {motionController} {

  displayedChar[0] = 0;
  displayedChar[1] = 0;
  displayedChar[2] = 0;
  displayedChar[3] = 0;
  displayedChar[4] = 0;

  lv_obj_t *label; // temporary pointer to set label for buttons

  // Create a 200px wide background rectangle
  timebar = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(timebar, Convert(settingsController.GetPTSColorBG()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(timebar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, LV_ALIGN_TOP_LEFT, 0, 0);

  // Display the time
  timeDD1 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(timeDD1, &open_sans_light, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(timeDD1, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeDD1, "00");
  lv_obj_align_to(timeDD1, timebar, LV_ALIGN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(timeDD2, &open_sans_light, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(timeDD2, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeDD2, "00");
  lv_obj_align_to(timeDD2, timebar, LV_ALIGN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(timeAMPM, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_line_space(timeAMPM, -3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeAMPM, "");
  lv_obj_align_to(timeAMPM, timebar, LV_ALIGN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen
  sidebar = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(sidebar, Convert(settingsController.GetPTSColorBar()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(sidebar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, LV_ALIGN_TOP_RIGHT, 0, 0);

  // Display icons
  batteryIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(batteryIcon, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_MID, 0, 2);

  bleIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(bleIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(bleIcon, "");

  notificationIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(notificationIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(notificationIcon, "");

  // Calendar icon
  calendarOuter = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarOuter, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarOuter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarOuter, 34, 34);
  lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);

  calendarInner = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarInner, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarInner, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarInner, 27, 27);
  lv_obj_align_to(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);

  calendarBar1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarBar1, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarBar1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarBar1, 3, 12);
  lv_obj_align_to(calendarBar1, calendarOuter, LV_ALIGN_TOP_MID, -6, -3);

  calendarBar2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarBar2, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarBar2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarBar2, 3, 12);
  lv_obj_align_to(calendarBar2, calendarOuter, LV_ALIGN_TOP_MID, 6, -3);

  calendarCrossBar1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarCrossBar1, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarCrossBar1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarCrossBar1, 8, 3);
  lv_obj_align_to(calendarCrossBar1, calendarBar1, LV_ALIGN_BOTTOM_MID, 0, 0);

  calendarCrossBar2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarCrossBar2, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarCrossBar2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarCrossBar2, 8, 3);
  lv_obj_align_to(calendarCrossBar2, calendarBar2, LV_ALIGN_BOTTOM_MID, 0, 0);

  // Display date
  dateDayOfWeek = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(dateDayOfWeek, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateDayOfWeek, "THU");
  lv_obj_align_to(dateDayOfWeek, sidebar, LV_ALIGN_CENTER, 0, -34);

  dateDay = lv_label_create(calendarInner);
  lv_obj_set_style_text_color(dateDay, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateDay, "25");
  lv_obj_center(dateDay);

  dateMonth = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(dateMonth, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateMonth, "MAR");
  lv_obj_align_to(dateMonth, sidebar, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  if (settingsController.GetPTSColorBar() == Pinetime::Controllers::Settings::Colors::White) {
    needle_colors[0] = lv_color_black();
  } else {
    needle_colors[0] = lv_color_white();
  }
  stepCont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(stepCont, 40, 40);
  lv_obj_align(stepCont, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  
  stepMeter = lv_meter_create(stepCont);
  lv_obj_set_size(stepMeter, 37, 37);
  lv_obj_center(stepMeter);

  stepScale = lv_meter_add_scale(stepMeter);
  stepIndicator = lv_meter_add_needle_line(stepMeter, stepScale, 2, needle_colors[0], -6);
  lv_meter_set_scale_range(stepMeter, stepScale, 0, 100, 360, 180);
  lv_meter_set_scale_ticks(stepMeter, stepScale, 11, 4, 4, lv_color_black());
  lv_meter_set_indicator_value(stepMeter, stepIndicator, 0);

  lv_obj_set_style_pad_all(stepMeter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepMeter, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepMeter, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(stepMeter, lv_color_black(), LV_PART_TICKS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepCont, LV_OPA_TRANSP, 0);

  backgroundLabel = lv_label_create(lv_scr_act());
  lv_obj_add_flag(backgroundLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  btnNextTime = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align(btnNextTime, LV_ALIGN_RIGHT_MID, -15, -80);
  lv_obj_set_style_bg_opa(btnNextTime, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnNextTime);
  lv_label_set_text_static(label, ">");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnNextTime, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnNextTime, LV_OBJ_FLAG_HIDDEN);

  btnPrevTime = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align(btnPrevTime, LV_ALIGN_LEFT_MID, 15, -80);
  lv_obj_set_style_bg_opa(btnPrevTime, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnPrevTime);
  lv_label_set_text_static(label, "<");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnPrevTime, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnPrevTime, LV_OBJ_FLAG_HIDDEN);

  btnNextBar = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnNextBar, 60, 60);
  lv_obj_align(btnNextBar, LV_ALIGN_RIGHT_MID, -15, 0);
  lv_obj_set_style_bg_opa(btnNextBar, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnNextBar);
  lv_label_set_text_static(label, ">");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnNextBar, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnNextBar, LV_OBJ_FLAG_HIDDEN);

  btnPrevBar = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnPrevBar, 60, 60);
  lv_obj_align(btnPrevBar, LV_ALIGN_LEFT_MID, 15, 0);
  lv_obj_set_style_bg_opa(btnPrevBar, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnPrevBar);
  lv_label_set_text_static(label, "<");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnPrevBar, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnPrevBar, LV_OBJ_FLAG_HIDDEN);

  btnNextBG = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align(btnNextBG, LV_ALIGN_RIGHT_MID, -15, 80);
  lv_obj_set_style_bg_opa(btnNextBG, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnNextBG);
  lv_label_set_text_static(label, ">");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnNextBG, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnNextBG, LV_OBJ_FLAG_HIDDEN);

  btnPrevBG = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align(btnPrevBG, LV_ALIGN_LEFT_MID, 15, 80);
  lv_obj_set_style_bg_opa(btnPrevBG, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnPrevBG);
  lv_label_set_text_static(label, "<");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnPrevBG, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnPrevBG, LV_OBJ_FLAG_HIDDEN);

  btnReset = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_bg_opa(btnReset, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnReset);
  lv_label_set_text_static(label, "Rst");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnReset, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnReset, LV_OBJ_FLAG_HIDDEN);

  btnRandom = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnRandom, 60, 60);
  lv_obj_align(btnRandom, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_opa(btnRandom, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnRandom);
  lv_label_set_text_static(label, "Rnd");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnRandom, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnRandom, LV_OBJ_FLAG_HIDDEN);

  btnClose = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_bg_opa(btnClose, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  label = lv_label_create(btnClose);
  lv_label_set_text_static(label, "X");
  lv_obj_center(label);
  lv_obj_add_event_cb(btnClose, event_handler, LV_EVENT_CLICKED, this);
  lv_obj_add_flag(btnClose, LV_OBJ_FLAG_HIDDEN);

  btnSet = lv_btn_create(lv_scr_act());
  lv_obj_set_height(btnSet, 150);
  lv_obj_set_width(btnSet, 150);
  lv_obj_align(btnSet, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(btnSet, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(btnSet, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(btnSet, event_handler, LV_EVENT_CLICKED, this);
  lbl_btnSet = lv_label_create(btnSet);
  lv_obj_set_style_text_font(lbl_btnSet, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lbl_btnSet, Symbols::settings);
  lv_obj_add_flag(btnSet, LV_OBJ_FLAG_HIDDEN);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
  Refresh();
}

PineTimeStyle::~PineTimeStyle() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool PineTimeStyle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_has_flag(btnRandom, LV_OBJ_FLAG_HIDDEN)) {
    lv_obj_clear_flag(btnSet, LV_OBJ_FLAG_HIDDEN);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_has_flag(btnRandom, LV_OBJ_FLAG_HIDDEN) == false)) {
    return true;
  }
  return false;
}

void PineTimeStyle::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_add_flag(btnNextTime, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevTime, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnNextBar, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevBar, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnNextBG, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnPrevBG, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnReset, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnRandom, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
}

bool PineTimeStyle::OnButtonPushed() {
  if (!lv_obj_has_flag(btnClose, LV_OBJ_FLAG_HIDDEN)) {
    CloseMenu();
    return true;
  }
  return false;
}

void PineTimeStyle::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
}

void PineTimeStyle::AlignIcons() {
  if (notificationState.Get() && bleState.Get()) {
    lv_obj_align_to(bleIcon, sidebar, LV_ALIGN_TOP_MID, 8, 25);
    lv_obj_align_to(notificationIcon, sidebar, LV_ALIGN_TOP_MID, -8, 25);
  } else if (notificationState.Get() && !bleState.Get()) {
    lv_obj_align_to(notificationIcon, sidebar, LV_ALIGN_TOP_MID, 0, 25);
  } else {
    lv_obj_align_to(bleIcon, sidebar, LV_ALIGN_TOP_MID, 0, 25);
  }
}

void PineTimeStyle::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_label_set_text(batteryIcon, Symbols::plug);
    } else {
      SetBatteryIcon();
    }
  }
  if (!isCharging.Get()) {
    batteryPercentRemaining = batteryController.PercentRemaining();
    if (batteryPercentRemaining.IsUpdated()) {
      SetBatteryIcon();
    }
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleState.Get()));
    AlignIcons();
  }

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
    AlignIcons();
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime); auto time = date::make_time(newDateTime - dp); auto yearMonthDay = date::year_month_day(dp);

    auto year = static_cast<int>(yearMonthDay.year());
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    int hour = time.hours().count();
    auto minute = time.minutes().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", static_cast<int>(minute));

    char hoursChar[3];
    char ampmChar[5];
    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
      sprintf(hoursChar, "%02d", hour);
    } else {
      if (hour == 0 && hour != 12) {
        hour = 12;
        sprintf(ampmChar, "A\nM");
      } else if (hour == 12 && hour != 0) {
        hour = 12;
        sprintf(ampmChar, "P\nM");
      } else if (hour < 12 && hour != 0) {
        sprintf(ampmChar, "A\nM");
      } else if (hour > 12 && hour != 0) {
        hour = hour - 12;
        sprintf(ampmChar, "P\nM");
      }
      sprintf(hoursChar, "%02d", hour);
    }

    if (hoursChar[0] != displayedChar[0] or hoursChar[1] != displayedChar[1] or minutesChar[0] != displayedChar[2] or
        minutesChar[1] != displayedChar[3]) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        lv_label_set_text(timeAMPM, ampmChar);
      }

      lv_label_set_text_fmt(timeDD1, "%s", hoursChar);
      lv_label_set_text_fmt(timeDD2, "%s", minutesChar);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(dateDayOfWeek, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(dateDay, "%d", day);
      lv_label_set_text_fmt(dateMonth, "%s", dateTimeController.MonthShortToString());

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_meter_set_indicator_value(stepMeter, stepIndicator, (stepCount.Get() / (settingsController.GetStepsGoal() / 100)));
    if (stepCount.Get() > settingsController.GetStepsGoal()) {
      lv_obj_set_style_line_color(stepMeter, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_grad_color(stepMeter, lv_color_white(), LV_PART_TICKS | LV_STATE_DEFAULT);
    }
  }
  if (!lv_obj_has_flag(btnSet, LV_OBJ_FLAG_HIDDEN)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_add_flag(btnSet, LV_OBJ_FLAG_HIDDEN);
      savedTick = 0;
    }
  }
}

void PineTimeStyle::UpdateSelected(lv_obj_t* object, lv_event_t *event) {
  auto valueTime = settingsController.GetPTSColorTime();
  auto valueBar = settingsController.GetPTSColorBar();
  auto valueBG = settingsController.GetPTSColorBG();

  if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
    if (object == btnNextTime) {
      valueTime = GetNext(valueTime);
      if (valueTime == valueBG) {
        valueTime = GetNext(valueTime);
      }
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnPrevTime) {
      valueTime = GetPrevious(valueTime);
      if (valueTime == valueBG) {
        valueTime = GetPrevious(valueTime);
      }
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnNextBar) {
      valueBar = GetNext(valueBar);
      if (valueBar == Controllers::Settings::Colors::Black) {
        valueBar = GetNext(valueBar);
      }
      if (valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = lv_color_black();
      } else {
        needle_colors[0] = lv_color_white();
      }
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnPrevBar) {
      valueBar = GetPrevious(valueBar);
      if (valueBar == Controllers::Settings::Colors::Black) {
        valueBar = GetPrevious(valueBar);
      }
      if (valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = lv_color_black();
      } else {
        needle_colors[0] = lv_color_white();
      }
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnNextBG) {
      valueBG = GetNext(valueBG);
      if (valueBG == valueTime) {
        valueBG = GetNext(valueBG);
      }
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnPrevBG) {
      valueBG = GetPrevious(valueBG);
      if (valueBG == valueTime) {
        valueBG = GetPrevious(valueBG);
      }
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnReset) {
      needle_colors[0] = lv_color_white();
      settingsController.SetPTSColorTime(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_text_color(timeDD1, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBar(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_bg_color(sidebar, Convert(Controllers::Settings::Colors::Teal), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBG(Controllers::Settings::Colors::Black);
      lv_obj_set_style_bg_color(timebar, Convert(Controllers::Settings::Colors::Black), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnRandom) {
      valueTime = static_cast<Controllers::Settings::Colors>(rand() % 17);
      valueBar = static_cast<Controllers::Settings::Colors>(rand() % 17);
      valueBG = static_cast<Controllers::Settings::Colors>(rand() % 17);
      if (valueTime == valueBG) {
        valueBG = GetNext(valueBG);
      }
      if (valueBar == Controllers::Settings::Colors::Black) {
        valueBar = GetPrevious(valueBar);
      }
      if (valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = lv_color_black();
      } else {
        needle_colors[0] = lv_color_white();
      }
      settingsController.SetPTSColorTime(static_cast<Controllers::Settings::Colors>(valueTime));
      lv_obj_set_style_text_color(timeDD1, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeDD2, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(timeAMPM, Convert(valueTime), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBar(static_cast<Controllers::Settings::Colors>(valueBar));
      lv_obj_set_style_bg_color(sidebar, Convert(valueBar), LV_PART_MAIN | LV_STATE_DEFAULT);
      settingsController.SetPTSColorBG(static_cast<Controllers::Settings::Colors>(valueBG));
      lv_obj_set_style_bg_color(timebar, Convert(valueBG), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnSet) {
      lv_obj_add_flag(btnSet, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnNextTime, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnPrevTime, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnNextBar, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnPrevBar, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnNextBG, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnPrevBG, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnReset, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnRandom, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btnClose, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

Pinetime::Controllers::Settings::Colors PineTimeStyle::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 16) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors PineTimeStyle::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(16);
  }
  return prevColor;
}
