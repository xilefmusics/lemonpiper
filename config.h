#include "definitions.h"

#define COLOR_MODULE_BACKGROUND "#363636"
#define COLOR_BACKGROUND "#262626"
#define COLOR_FOREGROUND "#ebdbb2"
#define COLOR_FOREGROUND_ACTIVE "#cc241d"
#define COLOR_UNDERLINE "#ff0000"
#define FONT "'InputMono'-9"
#define DIMENSIONS "1920x18+0+0"

// modules
#include "modules/kdwm.h"
#include "modules/datetime.h"
#include "modules/alsa.h"
#include "modules/battery.h"
#include "modules/backlight.h"

module_t *parallel_updates[] ={
  &kdwm
};

periodic_update_t periodic_updates[] = {
  {&backlight, 5},
  {&battery, 5},
  {&alsa, 5},
  {&datetime, 5}
};

block_t blocks_left[] = {
  {&kdwm, COLOR_BACKGROUND}
};

block_t blocks_center[] = {
};

block_t blocks_right[] = {
  {&backlight, COLOR_MODULE_BACKGROUND},
  {&battery, COLOR_MODULE_BACKGROUND},
  {&alsa, COLOR_MODULE_BACKGROUND},
  {&datetime, COLOR_MODULE_BACKGROUND}
};
