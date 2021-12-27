#include "PassKey.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

PassKey::PassKey(Pinetime::Applications::DisplayApp* app, uint32_t key) : Screen(app) {
  passkeyLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(passkeyLabel, lv_color_hex(0xFFFF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(passkeyLabel, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(passkeyLabel, "%06u", key);
  lv_obj_align(passkeyLabel, LV_ALIGN_CENTER, 0, -20);

  backgroundLabel = lv_label_create(lv_scr_act());
  lv_obj_add_flag(backgroundLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

PassKey::~PassKey() {
  lv_obj_clean(lv_scr_act());
}

