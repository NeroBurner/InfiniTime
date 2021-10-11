#pragma once

#include <lvgl/src/core/lv_obj.h>
#include "displayapp/screens/Screen.h"

#include <array>

namespace Pinetime {
  namespace Applications {
    struct TwosTile {
      bool merged = false;
      bool changed = true;
      bool isNew = false;
      unsigned int value = 0;
    };
    namespace Screens {
      class Twos : public Screen {
      public:
        Twos(DisplayApp* app);
        ~Twos() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        lv_style_t style_cell_default;

        lv_obj_t* scoreText;
        lv_obj_t* gridDisplay;
        std::array<std::array<TwosTile, 4>,4> grid;
        unsigned int score = 0;
        void updateGridDisplay(std::array<std::array<TwosTile,4>,4> &grid);
        bool tryMerge(std::array<std::array<TwosTile,4>,4> &grid, int& newRow, int& newCol, int oldRow, int oldCol);
        bool tryMove(std::array<std::array<TwosTile,4>,4> &grid, int newRow, int newCol, int oldRow, int oldCol);
        bool placeNewTile();
      };
    }
  }
}
