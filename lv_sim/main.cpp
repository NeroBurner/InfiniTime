
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
#include "displayapp/screens/SystemInfo.h"
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
#include "drivers/Cst816s.h"
#include "drivers/Hrs3300.h"
#include "drivers/Spi.h"
#include "drivers/SpiMaster.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/St7789.h"
#include "drivers/Watchdog.h"
#include "touchhandler/TouchHandler.h"

// get the simulator-headers
#include "displayapp/DisplayApp.h"
#include "displayapp/LittleVgl.h"

#include <nrfx_gpiote.h>

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
lv_indev_t *kb_indev;
lv_indev_t *mouse_indev = nullptr;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {}

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
constexpr NRF_TWIM_Type *NRF_TWIM1 = nullptr;


static constexpr uint8_t touchPanelTwiAddress = 0x15;
static constexpr uint8_t motionSensorTwiAddress = 0x18;
static constexpr uint8_t heartRateSensorTwiAddress = 0x44;

Pinetime::Drivers::SpiMaster spi {Pinetime::Drivers::SpiMaster::SpiModule::SPI0,
                                  {Pinetime::Drivers::SpiMaster::BitOrder::Msb_Lsb,
                                   Pinetime::Drivers::SpiMaster::Modes::Mode3,
                                   Pinetime::Drivers::SpiMaster::Frequencies::Freq8Mhz,
                                   Pinetime::PinMap::SpiSck,
                                   Pinetime::PinMap::SpiMosi,
                                   Pinetime::PinMap::SpiMiso}};

Pinetime::Drivers::Spi lcdSpi {spi, Pinetime::PinMap::SpiLcdCsn};
Pinetime::Drivers::St7789 lcd {lcdSpi, Pinetime::PinMap::LcdDataCommand};

Pinetime::Drivers::Spi flashSpi {spi, Pinetime::PinMap::SpiFlashCsn};
Pinetime::Drivers::SpiNorFlash spiNorFlash {flashSpi};

// The TWI device should work @ up to 400Khz but there is a HW bug which prevent it from
// respecting correct timings. According to erratas heet, this magic value makes it run
// at ~390Khz with correct timings.
static constexpr uint32_t MaxTwiFrequencyWithoutHardwareBug {0x06200000};
Pinetime::Drivers::TwiMaster twiMaster {NRF_TWIM1, MaxTwiFrequencyWithoutHardwareBug, Pinetime::PinMap::TwiSda, Pinetime::PinMap::TwiScl};
Pinetime::Drivers::Cst816S touchPanel; // {twiMaster, touchPanelTwiAddress};
//#ifdef PINETIME_IS_RECOVERY
//  #include "displayapp/DummyLittleVgl.h"
//  #include "displayapp/DisplayAppRecovery.h"
//#else
//  #include "displayapp/LittleVgl.h"
//  #include "displayapp/DisplayApp.h"
//#endif
Pinetime::Components::LittleVgl lvgl {lcd, touchPanel};

Pinetime::Drivers::Bma421 motionSensor {twiMaster, motionSensorTwiAddress};
Pinetime::Drivers::Hrs3300 heartRateSensor {twiMaster, heartRateSensorTwiAddress};

TimerHandle_t debounceTimer;
TimerHandle_t debounceChargeTimer;
Pinetime::Controllers::Battery batteryController;
Pinetime::Controllers::Ble bleController;

Pinetime::Controllers::HeartRateController heartRateController;
Pinetime::Applications::HeartRateTask heartRateApp(heartRateSensor, heartRateController);

Pinetime::Controllers::DateTime dateTimeController;
Pinetime::Drivers::Watchdog watchdog;
Pinetime::Drivers::WatchdogView watchdogView(watchdog);
Pinetime::Controllers::NotificationManager notificationManager;
Pinetime::Controllers::MotionController motionController;
Pinetime::Controllers::TimerController timerController;
Pinetime::Controllers::AlarmController alarmController {dateTimeController};
Pinetime::Controllers::TouchHandler touchHandler(touchPanel, lvgl);
Pinetime::Controllers::ButtonHandler buttonHandler;

Pinetime::Controllers::FS fs; // {spiNorFlash};
Pinetime::Controllers::Settings settingsController {fs};
Pinetime::Controllers::MotorController motorController {};
Pinetime::Controllers::BrightnessController brightnessController {};

Pinetime::Applications::DisplayApp displayApp(lcd,
                                              lvgl,
                                              touchPanel,
                                              batteryController,
                                              bleController,
                                              dateTimeController,
                                              watchdogView,
                                              notificationManager,
                                              heartRateController,
                                              settingsController,
                                              motorController,
                                              motionController,
                                              timerController,
                                              alarmController,
                                              brightnessController,
                                              touchHandler);

Pinetime::System::SystemTask systemTask(spi,
                                        lcd,
                                        spiNorFlash,
                                        twiMaster,
                                        touchPanel,
                                        lvgl,
                                        batteryController,
                                        bleController,
                                        dateTimeController,
                                        timerController,
                                        alarmController,
                                        watchdog,
                                        notificationManager,
                                        motorController,
                                        heartRateSensor,
                                        motionController,
                                        motionSensor,
                                        settingsController,
                                        heartRateController,
                                        displayApp,
                                        heartRateApp,
                                        fs,
                                        touchHandler,
                                        buttonHandler);

// variable used in SystemTask.cpp Work loop
std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> NoInit_BackUpTime;

class Framework {
public:
    // Contructor which initialize the parameters.
    Framework(bool visible_, int height_, int width_) :
      visible(visible_), height(height_), width(width_)
      {
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

        lvgl.Init();

        lv_mem_monitor(&mem_mon);
        printf("initial free_size = %u\n", mem_mon.free_size);

        // update time to current system time once on startup
        dateTimeController.SetCurrentTime(std::chrono::system_clock::now());

        systemTask.Start();

        // initialize the first LVGL screen
        //const auto clockface = settingsController.GetClockFace();
        //switch_to_screen(1+clockface);
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
      notificationManager.Push(std::move(notif));
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
      debounce('p', 'P', state[SDL_SCANCODE_P], key_handled_p);
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
        motorController.StartRinging();
      } else if (key == 'R') {
        motorController.StopRinging();
      } else if (key == 'm') {
        motorController.RunForDuration(100);
      } else if (key == 'M') {
        motorController.RunForDuration(255);
      } else if (key == 'n') {
        send_notification();
      } else if (key == 'N') {
        notificationManager.ClearNewNotificationFlag();
      } else if (key == 'b') {
        bleController.Connect();
      } else if (key == 'B') {
        bleController.Disconnect();
      } else if (key == 'v') {
        if (batteryController.percentRemaining >= 90) {
          batteryController.percentRemaining = 100;
        } else {
          batteryController.percentRemaining += 10;
        }
      } else if (key == 'V') {
        if (batteryController.percentRemaining <= 10) {
          batteryController.percentRemaining = 0;
        } else {
          batteryController.percentRemaining -= 10;
        }
      } else if (key == 'c') {
        batteryController.isCharging = true;
        batteryController.isPowerPresent = true;
      } else if (key == 'C') {
        batteryController.isCharging = false;
        batteryController.isPowerPresent = false;
      } else if (key == 'l') {
        brightnessController.Higher();
      } else if (key == 'L') {
        brightnessController.Lower();
      } else if (key == 'p') {
        this->print_memory_usage = true;
      } else if (key == 'P') {
        this->print_memory_usage = false;
      } else if (key >= '0' && key <= '9') {
        this->switch_to_screen(key-'0');
      } else if (key >= '!'+0 && key <= '!'+9) {
        this->switch_to_screen(key-'!'+10);
      }
      batteryController.voltage = batteryController.percentRemaining * 50;
    }

    void handle_touch_and_button() {
      int x, y;
      uint32_t buttons = SDL_GetMouseState(&x, &y);
      const bool left_click = (buttons & SDL_BUTTON_LMASK) != 0;
      const bool right_click = (buttons & SDL_BUTTON_RMASK) != 0;
      if (left_click) {
        left_release_sent = false;
        systemTask.OnTouchEvent();
        return;
      } else {
        if (!left_release_sent) {
          left_release_sent = true;
          systemTask.OnTouchEvent();
          return;
        }
      }
      if (right_click != right_last_state) {
        right_last_state =right_click;
        systemTask.PushMessage(Pinetime::System::Messages::HandleButtonEvent);
        return;
      }
    }

    // helper function to switch between screens
    void switch_to_screen(uint8_t screen_idx)
    {
      if (screen_idx == 1) {
        settingsController.SetClockFace(0);
        displayApp.StartApp(Pinetime::Applications::Apps::Clock, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 2) {
        settingsController.SetClockFace(1);
        displayApp.StartApp(Pinetime::Applications::Apps::Clock, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 3) {
        settingsController.SetClockFace(2);
        displayApp.StartApp(Pinetime::Applications::Apps::Clock, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 4) {
        displayApp.StartApp(Pinetime::Applications::Apps::Paddle, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 5) {
        displayApp.StartApp(Pinetime::Applications::Apps::Twos, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 6) {
        displayApp.StartApp(Pinetime::Applications::Apps::Metronome, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 7) {
        displayApp.StartApp(Pinetime::Applications::Apps::FirmwareUpdate, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 8) {
        displayApp.StartApp(Pinetime::Applications::Apps::BatteryInfo, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 9) {
        displayApp.StartApp(Pinetime::Applications::Apps::FlashLight, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 0) {
        displayApp.StartApp(Pinetime::Applications::Apps::QuickSettings, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 11) {
        displayApp.StartApp(Pinetime::Applications::Apps::Music, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 12) {
        displayApp.StartApp(Pinetime::Applications::Apps::Paint, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 13) {
        displayApp.StartApp(Pinetime::Applications::Apps::SysInfo, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 14) {
        displayApp.StartApp(Pinetime::Applications::Apps::Steps, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 15) {
        displayApp.StartApp(Pinetime::Applications::Apps::Error, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 17) {
        displayApp.StartApp(Pinetime::Applications::Apps::PassKey, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else if (screen_idx == 10) {
        displayApp.StartApp(Pinetime::Applications::Apps::Missing, Pinetime::Applications::DisplayApp::FullRefreshDirections::None);
      }
      else {
        std::cout << "unhandled screen_idx: " << int(screen_idx) << std::endl;
      }
    }
    // render the current status of the simulated controller
    void refresh_screen() {
      const Pinetime::Controllers::BrightnessController::Levels level = brightnessController.Level();
      if (level == Pinetime::Controllers::BrightnessController::Levels::Off) {
        if (!screen_off_created) {
          screen_off_created = true;
          screen_off_bg = lv_obj_create(lv_scr_act(), nullptr);
          lv_obj_set_size(screen_off_bg, 240, 240);
          lv_obj_set_pos(screen_off_bg, 0, 0);
          lv_obj_set_style_local_bg_color(screen_off_bg, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
          screen_off_label = lv_label_create(lv_scr_act(), nullptr);
          lv_label_set_text_static(screen_off_label, "Screen is OFF");
          lv_obj_align(screen_off_label, nullptr, LV_ALIGN_CENTER, 0, -20);
        }
      } else {
        if (screen_off_created) {
          screen_off_created = false;
          lv_obj_del(screen_off_bg);
          lv_obj_del(screen_off_label);
        }
      }
      if (print_memory_usage) {
        // print free memory with the knowledge that 14KiB RAM is the actual PineTime-Memory
        lv_mem_monitor(&mem_mon);
        printf("actual free_size = %d\n", int64_t(mem_mon.free_size) - (LV_MEM_SIZE - 14U*1024U));
      }
    }

    bool print_memory_usage = false;
    lv_mem_monitor_t mem_mon;

    // variables to create and destroy an lvgl overlay to indicate a turned off screen
    bool screen_off_created = false;
    lv_obj_t *screen_off_bg;
    lv_obj_t *screen_off_label;

private:
    bool key_handled_r = false; // r ... enable ringing, R ... disable ringing
    bool key_handled_m = false; // m ... let motor run, M ... stop motor
    bool key_handled_n = false; // n ... send notification, N ... clear all notifications
    bool key_handled_b = false; // b ... connect Bluetooth, B ... disconnect Bluetooth
    bool key_handled_v = false; // battery Voltage and percentage, v ... increase, V ... decrease
    bool key_handled_c = false; // c ... charging, C ... not charging
    bool key_handled_l = false; // l ... increase brightness level, L ... lower brightness level
    bool key_handled_p = false; // p ... enable print memory usage, P ... disable print memory usage
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

    bool left_release_sent = true; // make sure to send one mouse button release event
    bool right_last_state = false; // varable used to send message only on changing state
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
      lv_task_handler();
      fw.handle_keys(); // key event polling
      fw.handle_touch_and_button();
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

  // use pinetime_theme
  lv_theme_t* th = lv_pinetime_theme_init(
    LV_COLOR_WHITE, LV_COLOR_SILVER, 0, &jetbrains_mono_bold_20, &jetbrains_mono_bold_20, &jetbrains_mono_bold_20, &jetbrains_mono_bold_20);
  lv_theme_set_act(th);

  /*Create a display buffer*/
  static lv_disp_buf_t disp_buf1;
  static lv_color_t buf1_1[LV_HOR_RES_MAX * 120];
  lv_disp_buf_init(&disp_buf1, buf1_1, NULL, LV_HOR_RES_MAX * 120);

  /*Create a display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/
  disp_drv.buffer = &disp_buf1;
  disp_drv.flush_cb = monitor_flush;
  lv_disp_drv_register(&disp_drv);

  /* Add the mouse as input device
   * Use the 'mouse' driver which reads the PC's mouse*/
  //mouse_init();
  //static lv_indev_drv_t indev_drv_1;
  //lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
  //indev_drv_1.type = LV_INDEV_TYPE_POINTER;

  /*This function will be called periodically (by the library) to get the mouse position and state*/
  //indev_drv_1.read_cb = mouse_read;
  //mouse_indev = lv_indev_drv_register(&indev_drv_1);

  /*Add the keyboard as input device.*/
  //lv_indev_drv_t kb_drv;
  //lv_indev_drv_init(&kb_drv);
  //kb_drv.type = LV_INDEV_TYPE_KEYPAD;
  //kb_drv.read_cb = keyboard_read;
  //lv_indev_drv_register(&kb_drv);

  /*Add the mousewheel as input device.*/
  //lv_indev_drv_t enc_drv;
  //lv_indev_drv_init(&enc_drv);
  //enc_drv.type = LV_INDEV_TYPE_ENCODER;
  //enc_drv.read_cb = mousewheel_read;
  //lv_indev_drv_register(&enc_drv);

  /*Set a cursor for the mouse*/
  //LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  //lv_obj_t * cursor_obj = lv_img_create(lv_scr_act(), NULL); /*Create an image object for the cursor */
  //lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  //lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/
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
