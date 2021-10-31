#include "displayapp/screens/InfiniPaint.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"

#include <algorithm> // std::fill
#include <cmath> // fabs

using namespace Pinetime::Applications::Screens;

static void lv_touch_event(lv_event_t *event) {
  static bool is_pressed = false;
  static lv_point_t point_at_start;
  InfiniPaint* app = static_cast<InfiniPaint*>(event->user_data);
  lv_event_code_t code = lv_event_get_code(event);
  if (code == LV_EVENT_LONG_PRESSED) {
    lv_indev_t *indev = lv_event_get_indev(event);
    lv_point_t point;
    lv_indev_get_point(indev, &point);
    // issue long-tap only if the pointer stayed relatively still
    lv_coord_t dx = abs(point.x - point_at_start.x);
    lv_coord_t dy = abs(point.y - point_at_start.y);
    if (dx<10 && dy<10) {
      app->OnTouchEvent(Pinetime::Applications::TouchEvents::LongTap);
    }
  }
  else if (code == LV_EVENT_PRESSING) {
    lv_indev_t *indev = lv_event_get_indev(event);
    lv_point_t point;
    lv_indev_get_point(indev, &point);
    if(!is_pressed)
    {
      point_at_start = point;
      is_pressed = true;
    }
    app->OnTouchEvent(point.x, point.y);
  }
  else if (code == LV_EVENT_RELEASED) {
    is_pressed = false;
  }
}

InfiniPaint::InfiniPaint(Pinetime::Applications::DisplayApp* app,
                         Pinetime::Components::LittleVgl& lvgl,
                         Pinetime::Controllers::MotorController& motor)
  : Screen(app), lvgl {lvgl}, motor {motor} {
  canvas = lv_canvas_create(lv_scr_act());
  lv_canvas_set_buffer(canvas, buf, LV_HOR_RES, LV_VER_RES, LV_IMG_CF_TRUE_COLOR);
  lv_obj_center(canvas);
  lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
  // Handle input events
  lv_obj_add_event_cb(lv_scr_act(), lv_touch_event, LV_EVENT_LONG_PRESSED, this);
  lv_obj_add_event_cb(lv_scr_act(), lv_touch_event, LV_EVENT_PRESSING, this);
  lv_obj_add_event_cb(lv_scr_act(), lv_touch_event, LV_EVENT_RELEASED, this);
}

InfiniPaint::~InfiniPaint() {
  lv_obj_clean(lv_scr_act());
}

bool InfiniPaint::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case Pinetime::Applications::TouchEvents::LongTap:
      color = (color + 1) % 8;
      switch (color) {
        case 0:
          selectColor = lv_color_hex(0xFF00FF);
          break;
        case 1:
          selectColor = lv_palette_main(LV_PALETTE_GREEN);
          break;
        case 2:
          selectColor = lv_color_white();
          break;
        case 3:
          selectColor = lv_palette_main(LV_PALETTE_RED);
          break;
        case 4:
          selectColor = lv_palette_main(LV_PALETTE_CYAN);
          break;
        case 5:
          selectColor = lv_palette_main(LV_PALETTE_YELLOW);
          break;
        case 6:
          selectColor = lv_palette_main(LV_PALETTE_BLUE);
          break;
        case 7:
          selectColor = lv_color_black();
          break;

        default:
          color = 0;
          break;
      }

      motor.RunForDuration(35);
      return true;
    default:
      return true;
  }
  return true;
}

bool InfiniPaint::OnTouchEvent(uint16_t x, uint16_t y) {
  lv_area_t area;
  area.x1 = x - (width / 2);
  area.y1 = y - (height / 2);
  area.x2 = x + (width / 2) - 1;
  area.y2 = y + (height / 2) - 1;
  for (int i=area.x1; i<area.x2; i++) {
    for (int j=area.y1; j<area.y2; j++) {
      lv_canvas_set_px(canvas, i, j, selectColor);
    }
  }
  return true;
}
