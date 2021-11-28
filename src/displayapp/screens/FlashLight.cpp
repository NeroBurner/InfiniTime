#include "displayapp/screens/FlashLight.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t* event) {
    FlashLight* screen = static_cast<FlashLight*>(lv_event_get_user_data(event));
    screen->OnClickEvent(lv_event_get_target(event), event);
  }
}

FlashLight::FlashLight(Pinetime::Applications::DisplayApp* app,
                       System::SystemTask& systemTask,
                       Controllers::BrightnessController& brightnessController)
  : Screen(app),
    systemTask {systemTask},
    brightnessController {brightnessController}

{
  brightnessController.Backup();

  brightnessLevel = brightnessController.Level();

  flashLight = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(flashLight, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(flashLight, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(flashLight, Symbols::highlight);
  lv_obj_align(flashLight, LV_ALIGN_CENTER, 0, 0);

  for (auto & i : indicators) {
    i = lv_obj_create(lv_scr_act());
    lv_obj_set_size(i, 15, 10);
    lv_obj_set_style_border_width(i, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  lv_obj_align(indicators[1], flashLight, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  lv_obj_align(indicators[0], indicators[1], LV_ALIGN_OUT_LEFT_MID, -8, 0);
  lv_obj_align(indicators[2], indicators[1], LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  SetIndicators();
  SetColors();

  backgroundAction = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundAction, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundAction, 240, 240);
  lv_obj_set_pos(backgroundAction, 0, 0);
  lv_label_set_text(backgroundAction, "");
  lv_obj_add_flag(backgroundAction, LV_OBJ_FLAG_CLICKABLE);
  backgroundAction->user_data = this;
  lv_obj_add_event_cb(backgroundAction, event_handler, LV_EVENT_ALL, backgroundAction->user_data);

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  brightnessController.Restore();
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void FlashLight::SetColors() {
  if (isOn) {
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(flashLight, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    for (auto & i : indicators) {
      lv_obj_set_style_bg_color(i, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(i, lv_color_white(), LV_PART_MAIN | LV_STATE_DISABLED);
      lv_obj_set_style_border_color(i, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  } else {
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(flashLight, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    for (auto & i : indicators) {
      lv_obj_set_style_bg_color(i, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(i, lv_color_black(), LV_PART_MAIN | LV_STATE_DISABLED);
      lv_obj_set_style_border_color(i, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
}

void FlashLight::SetIndicators() {
  using namespace Pinetime::Controllers;

  if (brightnessLevel == BrightnessController::Levels::High) {
    lv_obj_set_state(indicators[1], LV_STATE_DEFAULT);
    lv_obj_set_state(indicators[2], LV_STATE_DEFAULT);
  } else if (brightnessLevel == BrightnessController::Levels::Medium) {
    lv_obj_set_state(indicators[1], LV_STATE_DEFAULT);
    lv_obj_set_state(indicators[2], LV_STATE_DISABLED);
  } else {
    lv_obj_set_state(indicators[1], LV_STATE_DISABLED);
    lv_obj_set_state(indicators[2], LV_STATE_DISABLED);
  }
}

void FlashLight::OnClickEvent(lv_obj_t* obj, lv_event_t *event) {
  if (obj == backgroundAction && event == LV_EVENT_CLICKED) {
    isOn = !isOn;
    SetColors();
  }
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  using namespace Pinetime::Controllers;

  if (event == TouchEvents::SwipeLeft) {
    if (brightnessLevel == BrightnessController::Levels::High) {
      brightnessLevel = BrightnessController::Levels::Medium;
      brightnessController.Set(brightnessLevel);
      SetIndicators();
    } else if (brightnessLevel == BrightnessController::Levels::Medium) {
      brightnessLevel = BrightnessController::Levels::Low;
      brightnessController.Set(brightnessLevel);
      SetIndicators();
    }
    return true;
  }
  if (event == TouchEvents::SwipeRight) {
    if (brightnessLevel == BrightnessController::Levels::Low) {
      brightnessLevel = BrightnessController::Levels::Medium;
      brightnessController.Set(brightnessLevel);
      SetIndicators();
    } else if (brightnessLevel == BrightnessController::Levels::Medium) {
      brightnessLevel = BrightnessController::Levels::High;
      brightnessController.Set(brightnessLevel);
      SetIndicators();
    }
    return true;
  }

  return false;
}
