#include "Styles.h"

void Pinetime::Applications::Screens::SetRadioButtonStyle(lv_obj_t* checkbox) {
  lv_obj_set_style_radius(checkbox, LV_RADIUS_CIRCLE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(checkbox, 9, LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_border_color(checkbox, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(checkbox, lv_color_white(), LV_PART_INDICATOR | LV_STATE_CHECKED);
}
