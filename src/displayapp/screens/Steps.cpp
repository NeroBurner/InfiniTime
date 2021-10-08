#include "displayapp/screens/Steps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

static void lap_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* steps = static_cast<Steps*>(obj->user_data);
  steps->lapBtnEventHandler(event);
}

Steps::Steps(Pinetime::Applications::DisplayApp* app,
             Controllers::MotionController& motionController,
             Controllers::Settings& settingsController)
  : Screen(app), motionController {motionController}, settingsController {settingsController} {

  stepsArc = lv_arc_create(lv_scr_act());

  lv_obj_set_style_bg_opa(stepsArc, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(stepsArc, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(stepsArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(stepsArc, lv_color_hex(0x0000FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_arc_set_end_angle(stepsArc, 200);
  lv_obj_set_size(stepsArc, 240, 240);
  lv_arc_set_range(stepsArc, 0, 500);
  lv_obj_align(stepsArc,  LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(stepsArc, LV_OBJ_FLAG_CLICKABLE);

  stepsCount = motionController.NbSteps();
  currentTripSteps = stepsCount - motionController.GetTripSteps();

  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));

  lSteps = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(lSteps, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(lSteps, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align(lSteps, LV_ALIGN_CENTER, 0, -40);

  lv_obj_t* lstepsL = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(lstepsL, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lstepsL, "Steps");
  lv_obj_align_to(lstepsL, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

  lv_obj_t* lstepsGoal = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(lstepsGoal, lv_palette_main(LV_PALETTE_CYAN), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(lstepsGoal, "Goal\n%lu", settingsController.GetStepsGoal());
  lv_obj_set_style_text_align(lstepsGoal, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(lstepsGoal, lSteps, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  resetBtn = lv_btn_create(lv_scr_act());
  resetBtn->user_data = this;
  lv_obj_set_event_cb(resetBtn, lap_event_handler);
  lv_obj_set_height(resetBtn, 50);
  lv_obj_set_width(resetBtn, 115);
  lv_obj_align(resetBtn, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  resetButtonLabel = lv_label_create(resetBtn);
  lv_label_set_text(resetButtonLabel, "Reset");

  currentTripSteps = motionController.GetTripSteps();

  tripLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(tripLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps);
  lv_obj_align_to(tripLabel, lstepsGoal, LV_ALIGN_IN_LEFT_MID, 0, 20);

  taskRefresh = lv_timer_create(RefreshTaskCallback, 100, this);
}

Steps::~Steps() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Steps::Refresh() {
  stepsCount = motionController.NbSteps();
  currentTripSteps = motionController.GetTripSteps();

  lv_label_set_text_fmt(lSteps, "%li", stepsCount);
  lv_obj_align(lSteps, LV_ALIGN_CENTER, 0, -40);

  if (currentTripSteps < 100000) {
    lv_label_set_text_fmt(tripLabel, "Trip: %5li", currentTripSteps);
  } else {
    lv_label_set_text_fmt(tripLabel, "Trip: 99999+");
  }
  lv_arc_set_value(stepsArc, int16_t(500 * stepsCount / settingsController.GetStepsGoal()));
}

void Steps::lapBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  stepsCount = motionController.NbSteps();
  motionController.ResetTrip();
  Refresh();
}
