#include "displayapp/screens/Twos.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <lvgl/lvgl.h>
#include <utility>
#include <vector>

using namespace Pinetime::Applications::Screens;


namespace {
  const lv_color_t TWOS_COLOR_0 = lv_color_hex(0xcdc0b4);
  const lv_color_t TWOS_COLOR_2_4 = lv_color_hex(0xefdfc6);
  const lv_color_t TWOS_COLOR_8_16 = lv_color_hex(0xef9263);
  const lv_color_t TWOS_COLOR_32_64 = lv_color_hex(0xf76142);
  const lv_color_t TWOS_COLOR_128_PLUS = lv_color_hex(0x007dc5);
  const lv_color_t TWOS_BORDER_DEFAULT = lv_color_hex(0xbbada0);
  const lv_color_t TWOS_BORDER_NEW = lv_color_darken(lv_color_hex(0xbbada0), LV_OPA_50);

  static void draw_part_event_cb(lv_event_t* event){
    lv_obj_t * obj = lv_event_get_target(event);
    std::array<std::array<Pinetime::Applications::TwosTile,4>,4> &grid = *static_cast<std::array<std::array<Pinetime::Applications::TwosTile,4>,4>*>(event->user_data);
    lv_obj_draw_part_dsc_t *dsc = static_cast<lv_obj_draw_part_dsc_t*>(lv_event_get_param(event));
    if(dsc->part == LV_PART_ITEMS) {
      uint32_t row = dsc->id /  lv_table_get_col_cnt(obj);
      uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
      switch (grid[row][col].value) {
        case 0:
          dsc->rect_dsc->bg_color = TWOS_COLOR_0;
          break;
        case 2:
        case 4:
          dsc->rect_dsc->bg_color = TWOS_COLOR_2_4;
          dsc->label_dsc->color = lv_color_black();
          break;
        case 8:
        case 16:
          dsc->rect_dsc->bg_color = TWOS_COLOR_8_16;
          dsc->label_dsc->color = lv_color_white();
          break;
        case 32:
        case 64:
          dsc->rect_dsc->bg_color = TWOS_COLOR_32_64;
          dsc->label_dsc->color = lv_color_white();
          break;
        default:
          dsc->rect_dsc->bg_color = TWOS_COLOR_128_PLUS;
          dsc->label_dsc->color = lv_color_white();
          break;
      }
    }
  }
  static void lv_event_gesture_cb(lv_event_t *event) {
    Twos* twos = static_cast<Twos*>(event->user_data);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_LEFT)
      twos->OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeLeft);
    else if (dir == LV_DIR_RIGHT)
      twos->OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeRight);
    else if (dir == LV_DIR_TOP)
      twos->OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeUp);
    else if (dir == LV_DIR_BOTTOM)
      twos->OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeDown);
  }
}

Twos::Twos(Pinetime::Applications::DisplayApp* app) : Screen(app) {

  // create default style to apply to tiles
  lv_style_init(&style_cell_default);
  lv_style_set_border_color(&style_cell_default, TWOS_BORDER_DEFAULT);
  lv_style_set_border_width(&style_cell_default, 3);
  lv_style_set_bg_opa(&style_cell_default, LV_OPA_COVER);
  lv_style_set_bg_color(&style_cell_default, TWOS_COLOR_0);
  lv_style_set_pad_top(&style_cell_default, 25);
  lv_style_set_text_color(&style_cell_default, lv_color_black());
  lv_style_set_text_align(&style_cell_default, LV_TEXT_ALIGN_CENTER);

  gridDisplay = lv_table_create(lv_scr_act());
  lv_obj_add_style(gridDisplay, &style_cell_default, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_table_set_col_cnt(gridDisplay, 4);
  lv_table_set_row_cnt(gridDisplay, 4);
  lv_table_set_col_width(gridDisplay, 0, LV_HOR_RES / 4);
  lv_table_set_col_width(gridDisplay, 1, LV_HOR_RES / 4);
  lv_table_set_col_width(gridDisplay, 2, LV_HOR_RES / 4);
  lv_table_set_col_width(gridDisplay, 3, LV_HOR_RES / 4);
  lv_obj_align(gridDisplay, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_clear_flag(gridDisplay, LV_OBJ_FLAG_CLICKABLE);

  // initialize grid
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      grid[row][col].value = 0;
    }
  }
  placeNewTile();
  placeNewTile();

  // format score text
  scoreText = lv_label_create(lv_scr_act());
  lv_obj_set_width(scoreText, LV_HOR_RES);
  lv_obj_set_style_text_align(scoreText, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(scoreText, LV_ALIGN_TOP_LEFT, 0, 10);
  lv_label_set_recolor(scoreText, true);
  lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", score);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  // Need a draw callback to color the cells
  lv_obj_add_event_cb(gridDisplay, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, grid.data());
  // Handle LVGL swipe events
  lv_obj_add_event_cb(lv_scr_act(), lv_event_gesture_cb, LV_EVENT_GESTURE, this);
}

Twos::~Twos() {
  lv_style_reset(&style_cell_default);
  lv_obj_remove_event_cb(lv_scr_act(), lv_event_gesture_cb);
  lv_obj_remove_event_cb(gridDisplay, draw_part_event_cb);
  lv_obj_clean(lv_scr_act());
}

bool Twos::placeNewTile() {
  std::vector<std::pair<int, int>> availableCells;
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if(grid[row][col].isNew) {
        grid[row][col].isNew = false;
        grid[row][col].changed = true;
      }
      if (!grid[row][col].value) {
        availableCells.push_back(std::make_pair(row, col));
      }
    }
  }

  if (availableCells.size() == 0) {
    return false; // game lost
  }

  auto it = availableCells.cbegin();
  int random = rand() % availableCells.size();
  std::advance(it, random);
  std::pair<int, int> newCell = *it;
  TwosTile *curTile = &grid[newCell.first][newCell.second];

  if ((rand() % 100) < 90) {
    curTile->value = 2;
  } else {
    curTile->value = 4;
  }
  curTile->changed = true;
  curTile->isNew = true;
  updateGridDisplay(grid);
  return true;
}

bool Twos::tryMerge(std::array<std::array<TwosTile,4>,4> &grid, int& newRow, int& newCol, int oldRow, int oldCol) {
  if (grid[newRow][newCol].value == grid[oldRow][oldCol].value) {
    if ((newCol != oldCol) || (newRow != oldRow)) {
      if (!grid[newRow][newCol].merged) {
        unsigned int newVal = grid[oldRow][oldCol].value *= 2;
        grid[newRow][newCol].value = newVal;
        score += newVal;
        lv_label_set_text_fmt(scoreText, "Score #FFFF00 %i#", score);
        grid[oldRow][oldCol].value = 0;
        grid[newRow][newCol].merged = true;
        return true;
      }
    }
  }
  return false;
}

bool Twos::tryMove(std::array<std::array<TwosTile,4>,4> &grid, int newRow, int newCol, int oldRow, int oldCol) {
  if (((newCol >= 0) && (newCol != oldCol)) || ((newRow >= 0) && (newRow != oldRow))) {
    grid[newRow][newCol].value = grid[oldRow][oldCol].value;
    grid[oldRow][oldCol].value = 0;
    return true;
  }
  return false;
}

bool Twos::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  bool validMove = false;
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      grid[row][col].merged = false; // reinitialize merge state
    }
  }
  switch (event) {
    case TouchEvents::SwipeLeft:
      for (int col = 1; col < 4; col++) { // ignore tiles already on far left
        for (int row = 0; row < 4; row++) {
          if (grid[row][col].value) {
            int newCol = -1;
            for (int potentialNewCol = col - 1; potentialNewCol >= 0; potentialNewCol--) {
              if (!grid[row][potentialNewCol].value) {
                newCol = potentialNewCol;
              } else { // blocked by another tile
                if (tryMerge(grid, row, potentialNewCol, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, row, newCol, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    case TouchEvents::SwipeRight:
      for (int col = 2; col >= 0; col--) { // ignore tiles already on far right
        for (int row = 0; row < 4; row++) {
          if (grid[row][col].value) {
            int newCol = -1;
            for (int potentialNewCol = col + 1; potentialNewCol < 4; potentialNewCol++) {
              if (!grid[row][potentialNewCol].value) {
                newCol = potentialNewCol;
              } else { // blocked by another tile
                if (tryMerge(grid, row, potentialNewCol, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, row, newCol, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    case TouchEvents::SwipeUp:
      for (int row = 1; row < 4; row++) { // ignore tiles already on top
        for (int col = 0; col < 4; col++) {
          if (grid[row][col].value) {
            int newRow = -1;
            for (int potentialNewRow = row - 1; potentialNewRow >= 0; potentialNewRow--) {
              if (!grid[potentialNewRow][col].value) {
                newRow = potentialNewRow;
              } else { // blocked by another tile
                if (tryMerge(grid, potentialNewRow, col, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, newRow, col, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    case TouchEvents::SwipeDown:
      for (int row = 2; row >= 0; row--) { // ignore tiles already on bottom
        for (int col = 0; col < 4; col++) {
          if (grid[row][col].value) {
            int newRow = -1;
            for (int potentialNewRow = row + 1; potentialNewRow < 4; potentialNewRow++) {
              if (!grid[potentialNewRow][col].value) {
                newRow = potentialNewRow;
              } else { // blocked by another tile
                if (tryMerge(grid, potentialNewRow, col, row, col))
                  validMove = true;
                break;
              }
            }
            if (tryMove(grid, newRow, col, row, col))
              validMove = true;
          }
        }
      }
      if (validMove) {
        placeNewTile();
      }
      break;
    default:
      validMove = false;
      break;
  }
  return validMove;
}

void Twos::updateGridDisplay(std::array<std::array<TwosTile,4>,4> &grid) {
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      if (grid[row][col].value) {
        lv_table_set_cell_value_fmt(gridDisplay, row, col, "%u", grid[row][col].value);
      } else {
        lv_table_set_cell_value(gridDisplay, row, col, "");
      }
    }
  }
}
