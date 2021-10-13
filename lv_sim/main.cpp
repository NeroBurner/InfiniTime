
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
//#include "lvgl/examples/lv_examples.h"
//#include "lv_demos/lv_demo.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"

// get PineTime header
#include "displayapp/lv_pinetime_theme.h"
#include "displayapp/screens/Paddle.h"
#include "displayapp/screens/InfiniPaint.h"
#include "displayapp/screens/Twos.h"
#include "displayapp/screens/Metronome.h"
#include "displayapp/screens/Meter.h"
#include "displayapp/screens/Music.h"
#include "displayapp/screens/BatteryInfo.h"
#include "displayapp/screens/FlashLight.h"
#include "displayapp/screens/WatchFaceDigital.h"
#include "displayapp/screens/WatchFaceAnalog.h"
#include "displayapp/screens/PineTimeStyle.h"
#include "displayapp/screens/Clock.h"
#include "displayapp/screens/settings/QuickSettings.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/MusicService.h"
#include "components/ble/NotificationManager.h"
#include "components/brightness/BrightnessController.h"
#include "components/datetime/DateTimeController.h"
#include "components/fs/FS.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motor/MotorController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

// get the simulator-headers
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"

#include <iostream>
#include <typeinfo>
#include <algorithm>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void *data);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *mouse_indev = nullptr;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

class Framework {
public:
    // Contructor which initialize the parameters.
    Framework(bool visible_, int height_, int width_) :
      visible(visible_), height(height_), width(width_),
      settingsController(fsController),
      musicService(systemTask) {
        if (visible) {
          //SDL_Init(SDL_INIT_VIDEO);       // Initializing SDL as Video
          SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
          SDL_SetWindowTitle(window, "LV Simulator Status");
          { // move window a bit to the right, to not be over the PineTime Screen
              int x,y;
              SDL_GetWindowPosition(window, &x, &y);
              SDL_SetWindowPosition(window, x+LV_HOR_RES_MAX, y);
          }
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);      // setting draw color
          SDL_RenderClear(renderer);      // Clear the newly created window
          SDL_RenderPresent(renderer);    // Reflects the changes done in the
                                          //  window.
        }
        motorController.Init();
        settingsController.Init();
        brightnessController.Init();
        brightnessController.Set(settingsController.GetBrightness());
        const auto clockface = settingsController.GetClockFace();

        // initialize the first LVGL screen
        switch_to_screen(1+clockface);
    }

    // Destructor
    ~Framework(){
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void draw_circle_red(int center_x, int center_y, int radius_){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        draw_circle_(center_x, center_y, radius_);
    }
    void draw_circle_green(int center_x, int center_y, int radius_){
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        draw_circle_(center_x, center_y, radius_);
    }
    void draw_circle_blue(int center_x, int center_y, int radius_){
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        draw_circle_(center_x, center_y, radius_);
    }
    void draw_circle_yellow(int center_x, int center_y, int radius_){
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        draw_circle_(center_x, center_y, radius_);
    }
    void draw_circle_grey(int center_x, int center_y, int radius_){
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        draw_circle_(center_x, center_y, radius_);
    }
    void draw_circle_white(int center_x, int center_y, int radius_){
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        draw_circle_(center_x, center_y, radius_);
    }
    void draw_circle_(int center_x, int center_y, int radius_){
        // Drawing circle
        for(int x=center_x-radius_; x<=center_x+radius_; x++){
            for(int y=center_y-radius_; y<=center_y+radius_; y++){
                if((std::pow(center_y-y,2)+std::pow(center_x-x,2)) <=
                    std::pow(radius_,2)){
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }
    }

    void refresh() {
      // always refresh the LVGL screen
      this->refresh_screen();
      // update time to current system time
      this->dateTimeController.SetCurrentTime(std::chrono::system_clock::now());

      if (!visible) {
        return;
      }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        { // motorController.is_ringing
          constexpr const int center_x = 15;
          constexpr const int center_y = 15;
          if (motorController.is_ringing) {
              draw_circle_red(center_x, center_y, 15);
          } else {
              draw_circle_grey(center_x, center_y, 15);
          }
        }
        { // motorController.motor_is_running
          constexpr const int center_x = 45;
          constexpr const int center_y = 15;
          if (motorController.motor_is_running) {
              draw_circle_red(center_x, center_y, 15);
          } else {
              draw_circle_grey(center_x, center_y, 15);
          }
        }
        { // ble.motor_is_running
          constexpr const int center_x = 75;
          constexpr const int center_y = 15;
          if (bleController.IsConnected()) {
              draw_circle_blue(center_x, center_y, 15);
          } else {
              draw_circle_grey(center_x, center_y, 15);
          }
        }
        // batteryController.percentRemaining
        for (uint8_t percent=0; percent<=10; percent++) {
          const int center_x = 15+15*percent;
          const int center_y = 60;
          if (batteryController.percentRemaining < percent*10) {
            draw_circle_grey(center_x, center_y, 15);
          } else {
            draw_circle_green(center_x, center_y, 15);
          }
        }
        { // batteryController.isCharging
          constexpr const int center_x = 15;
          constexpr const int center_y = 90;
          if (batteryController.isCharging) {
              draw_circle_yellow(center_x, center_y, 15);
          } else
          {
              draw_circle_grey(center_x, center_y, 15);
          }
        }
        { // brightnessController.Level
          constexpr const int center_y = 15;
          const Pinetime::Controllers::BrightnessController::Levels level = brightnessController.Level();
          uint8_t level_idx = 0;
          if (level == Pinetime::Controllers::BrightnessController::Levels::Low)
          {
            level_idx = 1;
          } else if (level == Pinetime::Controllers::BrightnessController::Levels::Medium)
          {
            level_idx = 2;
          } else if (level == Pinetime::Controllers::BrightnessController::Levels::High)
          {
            level_idx = 3;
          }
          for (uint8_t i=0; i<4; i++) {
            const int center_x = 115+15*i;
            if (i <= level_idx) {
              draw_circle_white(center_x, center_y, 15);
            } else {
              draw_circle_grey(center_x, center_y, 15);
            }
          }
        }
        // Show the change on the screen
        SDL_RenderPresent(renderer);
    }

    void send_notification() {
      Pinetime::Controllers::NotificationManager::Notification notif;
      const std::string message("Lorem Ipsum");
      std::copy(message.begin(), message.end(), notif.message.data());
      notif.message[message.size() - 1] = '\0';
      notif.size = message.size();
      notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
      this->notificationManager.Push(std::move(notif));
    }

    // can't use SDL_PollEvent, as those are fed to lvgl
    // implement a non-descructive key-pressed handler (as not consuming SDL_Events)
    void handle_keys() {
      const Uint8 *state = SDL_GetKeyboardState(NULL);
      const bool key_shift = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];
      auto debounce = [&] (const char key_low, const char key_capital, const bool scancode, bool &key_handled) {
        if (scancode && !key_handled) {
          if (key_shift) {
            this->handle_key(key_capital);
          } else {
            this->handle_key(key_low);
          }
          key_handled = true;
        } else if (scancode && key_handled) {
          // ignore, already handled
        } else {
          key_handled = false;
        }
      };
      debounce('r', 'R', state[SDL_SCANCODE_R], key_handled_r);
      debounce('n', 'N', state[SDL_SCANCODE_N], key_handled_n);
      debounce('m', 'M', state[SDL_SCANCODE_M], key_handled_m);
      debounce('b', 'B', state[SDL_SCANCODE_B], key_handled_b);
      debounce('v', 'V', state[SDL_SCANCODE_V], key_handled_v);
      debounce('c', 'C', state[SDL_SCANCODE_C], key_handled_c);
      debounce('l', 'L', state[SDL_SCANCODE_L], key_handled_l);
      // screen switcher buttons
      debounce('1', '!'+1, state[SDL_SCANCODE_1], key_handled_1);
      debounce('2', '!'+2, state[SDL_SCANCODE_2], key_handled_2);
      debounce('3', '!'+3, state[SDL_SCANCODE_3], key_handled_3);
      debounce('4', '!'+4, state[SDL_SCANCODE_4], key_handled_4);
      debounce('5', '!'+5, state[SDL_SCANCODE_5], key_handled_5);
      debounce('6', '!'+6, state[SDL_SCANCODE_6], key_handled_6);
      debounce('7', '!'+7, state[SDL_SCANCODE_7], key_handled_7);
      debounce('8', '!'+8, state[SDL_SCANCODE_8], key_handled_8);
      debounce('9', '!'+9, state[SDL_SCANCODE_9], key_handled_9);
      debounce('0', '!'+0, state[SDL_SCANCODE_0], key_handled_0);
    }
    // modify the simulated controller depending on the pressed key
    void handle_key(SDL_Keycode key) {
      if (key == 'r') {
        this->motorController.StartRinging();
      } else if (key == 'R') {
        this->motorController.StopRinging();
      } else if (key == 'm') {
        this->motorController.RunForDuration(100);
      } else if (key == 'M') {
        this->motorController.RunForDuration(255);
      } else if (key == 'n') {
        send_notification();
      } else if (key == 'N') {
        this->notificationManager.ClearNewNotificationFlag();
      } else if (key == 'b') {
        this->bleController.Connect();
      } else if (key == 'B') {
        this->bleController.Disconnect();
      } else if (key == 'v') {
        if (this->batteryController.percentRemaining >= 90) {
          this->batteryController.percentRemaining = 100;
        } else {
          this->batteryController.percentRemaining += 10;
        }
      } else if (key == 'V') {
        if (this->batteryController.percentRemaining <= 10) {
          this->batteryController.percentRemaining = 0;
        } else {
          this->batteryController.percentRemaining -= 10;
        }
      } else if (key == 'c') {
        this->batteryController.isCharging = true;
        this->batteryController.isPowerPresent = true;
      } else if (key == 'C') {
        this->batteryController.isCharging = false;
        this->batteryController.isPowerPresent = false;
      } else if (key == 'l') {
        this->brightnessController.Higher();
      } else if (key == 'L') {
        this->brightnessController.Lower();
      } else if (key >= '0' && key <= '9') {
        this->switch_to_screen(key-'0');
      } else if (key >= '!'+0 && key <= '!'+9) {
        this->switch_to_screen(key-'!'+10);
      }
      this->batteryController.voltage = this->batteryController.percentRemaining * 50;
    }
    Pinetime::Applications::Screens::Screen *ClockScreen() {
      return new Pinetime::Applications::Screens::Clock(
        &app,
        dateTimeController,
        batteryController,
        bleController,
        notificationManager,
        settingsController,
        heartRateController,
        motionController);
    };
    Pinetime::Applications::Screens::Screen *WatchFaceDigitalScreen() {
      settingsController.SetClockFace(0);
      return ClockScreen();
    }
    Pinetime::Applications::Screens::Screen *WatchFaceAnalogScreen() {
      settingsController.SetClockFace(1);
      return ClockScreen();
    }
    Pinetime::Applications::Screens::Screen *PineTimeStyleScreen() {
      settingsController.SetClockFace(2);
      return ClockScreen();
    }
    Pinetime::Applications::Screens::Paddle *PaddleScreen() {
      return new Pinetime::Applications::Screens::Paddle(&app, lvgl);
    }
    Pinetime::Applications::Screens::InfiniPaint *InfiniPaintScreen() {
      return new Pinetime::Applications::Screens::InfiniPaint(&app, lvgl, motorController);
    }
    Pinetime::Applications::Screens::Twos *TwosScreen() {
      return new Pinetime::Applications::Screens::Twos(&app);
    }
    Pinetime::Applications::Screens::Metronome *MetronomeScreen() {
      return new Pinetime::Applications::Screens::Metronome(&app, motorController, systemTask);
    }
    Pinetime::Applications::Screens::Meter *MeterScreen() {
      return new Pinetime::Applications::Screens::Meter(&app);
    }
    Pinetime::Applications::Screens::Music *MusicScreen() {
      return new Pinetime::Applications::Screens::Music(&app, musicService);
    }
    Pinetime::Applications::Screens::BatteryInfo *BatteryInfoScreen() {
      return new Pinetime::Applications::Screens::BatteryInfo(&app, batteryController);
    }
    Pinetime::Applications::Screens::FlashLight *FlashLightScreen() {
      return new Pinetime::Applications::Screens::FlashLight(&app, systemTask, brightnessController);
    }
    Pinetime::Applications::Screens::QuickSettings *QuickSettingsScreen() {
      return new Pinetime::Applications::Screens::QuickSettings(&app, batteryController, dateTimeController, brightnessController, motorController, settingsController);
    }

    // helper function to switch between screens
    void switch_to_screen(uint8_t screen_idx)
    {
      screen.reset();
      if (screen_idx == 1) {
        screen.reset(WatchFaceDigitalScreen());
      }
      else if (screen_idx == 2) {
        screen.reset(WatchFaceAnalogScreen());
      }
      else if (screen_idx == 3) {
        screen.reset(PineTimeStyleScreen());
      }
      else if (screen_idx == 4) {
        screen.reset(PaddleScreen());
      }
      else if (screen_idx == 5) {
        screen.reset(TwosScreen());
      }
      else if (screen_idx == 6) {
        screen.reset(MetronomeScreen());
      }
      else if (screen_idx == 7) {
        screen.reset(MeterScreen());
      }
      else if (screen_idx == 8) {
        screen.reset(BatteryInfoScreen());
      }
      else if (screen_idx == 9) {
        screen.reset(FlashLightScreen());
      }
      else if (screen_idx == 0) {
        screen.reset(QuickSettingsScreen());
      }
      else if (screen_idx == 11) {
        screen.reset(MusicScreen());
      }
      else {
        std::cout << "unhandled screen_idx: " << int(screen_idx) << std::endl;
      }
    }
    // render the current status of the simulated controller
    void refresh_screen() {
      if (!screen)
        return;
      if (typeid(*screen.get()) == typeid(Pinetime::Applications::Screens::Paddle)) {
        auto *paddle = dynamic_cast<Pinetime::Applications::Screens::Paddle*>(screen.get());
        lv_point_t mouse_point;
        lv_indev_get_point(mouse_indev, &mouse_point);
        paddle->OnTouchEvent(mouse_point.x, mouse_point.y);
        paddle->Refresh();
      }
    }

    Pinetime::Applications::DisplayApp app;
    Pinetime::Components::LittleVgl lvgl;

    Pinetime::Controllers::Battery batteryController;
    Pinetime::Controllers::Ble bleController;
    Pinetime::Controllers::BrightnessController brightnessController;
    Pinetime::Controllers::HeartRateController heartRateController;
    Pinetime::Controllers::DateTime dateTimeController;
    Pinetime::Controllers::MotorController motorController;
    Pinetime::Controllers::MotionController motionController;
    Pinetime::Controllers::MusicService musicService;
    Pinetime::Controllers::NotificationManager notificationManager;
    Pinetime::Controllers::FS fsController;
    Pinetime::Controllers::Settings settingsController;
    Pinetime::System::SystemTask systemTask;

    std::unique_ptr<Pinetime::Applications::Screens::Screen> screen;

private:
    bool key_handled_r = false; // r ... enable ringing, R ... disable ringing
    bool key_handled_m = false; // m ... let motor run, M ... stop motor
    bool key_handled_n = false; // n ... send notification, N ... clear all notifications
    bool key_handled_b = false; // b ... connect Bluetooth, B ... disconnect Bluetooth
    bool key_handled_v = false; // battery Voltage and percentage, v ... increase, V ... decrease
    bool key_handled_c = false; // c ... charging, C ... not charging
    bool key_handled_l = false; // l ... increase brightness level, L ... lower brightness level
    // numbers from 0 to 9 to switch between screens
    bool key_handled_1 = false;
    bool key_handled_2 = false;
    bool key_handled_3 = false;
    bool key_handled_4 = false;
    bool key_handled_5 = false;
    bool key_handled_6 = false;
    bool key_handled_7 = false;
    bool key_handled_8 = false;
    bool key_handled_9 = false;
    bool key_handled_0 = false;
    bool visible;   // show Simulator window
    int height;     // Height of the window
    int width;      // Width of the window
    SDL_Renderer *renderer = NULL;      // Pointer for the renderer
    SDL_Window *window = NULL;      // Pointer for the window
};

int main(int argc, char **argv)
{
  // parse arguments
  bool fw_status_window_visible = true;
  bool arg_help = false;
  for (int i=1; i<argc; i++)
  {
    const std::string arg(argv[i]);
    if (arg == "--hide-status")
    {
      fw_status_window_visible = false;
    } else if (arg == "-h" || arg == "--help")
    {
      arg_help = true;
    } else
    {
      std::cout << "unknown argument '" << arg << "'" << std::endl;
      return 1;
    }
  }
  if (arg_help) {
    std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help           show this help message and exit" << std::endl;
    std::cout << "      --hide-status    don't show simulator status window, so only lvgl window is open" << std::endl;
    return 0;
  }

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init();

  // initialize the core of our Simulator
  Framework fw(fw_status_window_visible, 240,240);

  while(1) {
      /* Periodically call the lv_task handler.
       * It could be done in a timer interrupt or an OS task too.*/
      lv_timer_handler();
      fw.handle_keys(); // key event polling
      fw.refresh();
      usleep(LV_DISP_DEF_REFR_PERIOD * 1000);
  }

  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(void)
{
  /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
  monitor_init();
  /* Tick init.
   * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
   * how much time were elapsed Create an SDL thread to do this*/
  SDL_CreateThread(tick_thread, "tick", NULL);

  /*Create a display buffer*/
  static lv_disp_draw_buf_t disp_buf1;
  static lv_color_t buf1_1[MONITOR_HOR_RES * 100];
  static lv_color_t buf1_2[MONITOR_HOR_RES * 100];
  lv_disp_draw_buf_init(&disp_buf1, buf1_1, buf1_2, MONITOR_HOR_RES * 100);

  /*Create a display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf1;
  disp_drv.flush_cb = monitor_flush;
  disp_drv.hor_res = MONITOR_HOR_RES;
  disp_drv.ver_res = MONITOR_VER_RES;
  disp_drv.antialiasing = 1;

  lv_disp_t * disp = lv_disp_drv_register(&disp_drv);

  //lv_theme_t * th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
  //lv_disp_set_theme(disp, th);
  // use pinetime_theme
  lv_theme_t* th = lv_pinetime_theme_init(lv_disp_get_default(), lv_color_white(), lv_color_hex(0xC0C0C0), &jetbrains_mono_bold_20);
  lv_disp_set_theme(lv_disp_get_default(), th);

  lv_group_t * g = lv_group_create();
  lv_group_set_default(g);

  /* Add the mouse as input device
   * Use the 'mouse' driver which reads the PC's mouse*/
  mouse_init();
  static lv_indev_drv_t indev_drv_1;
  lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
  indev_drv_1.type = LV_INDEV_TYPE_POINTER;

  /*This function will be called periodically (by the library) to get the mouse position and state*/
  indev_drv_1.read_cb = mouse_read;
  mouse_indev = lv_indev_drv_register(&indev_drv_1);

  keyboard_init();
  static lv_indev_drv_t indev_drv_2;
  lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
  indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv_2.read_cb = keyboard_read;
  lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
  lv_indev_set_group(kb_indev, g);
  mousewheel_init();
  static lv_indev_drv_t indev_drv_3;
  lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
  indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
  indev_drv_3.read_cb = mousewheel_read;

  lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
  lv_indev_set_group(enc_indev, g);

  /*Set a cursor for the mouse*/
  LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
  lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
    (void)data;

    while(1) {
        SDL_Delay(LV_DISP_DEF_REFR_PERIOD);
        lv_tick_inc(LV_DISP_DEF_REFR_PERIOD); /*Tell LittelvGL that 30 milliseconds were elapsed*/
    }

    return 0;
}
