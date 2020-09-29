char battery_buffer[16];

void battery_update() {
  int c = -1, c0 = -1, c1 = -1;
  FILE *bat0 = fopen("/sys/class/power_supply/BAT0/capacity", "r");
  FILE *bat1 = fopen("/sys/class/power_supply/BAT1/capacity", "r");
  if (bat0) {
    fscanf(bat0, "%d", &c0);
    fclose(bat0);
  }
  if (bat1) {
    fscanf(bat1, "%d", &c1);
    fclose(bat1);
  }
  if (c1 > -1 && c0 > -1) {
    c = (c0 + c1) / 2;
  } else if (c0 > -1) {
    c = c0;
  } else if (c1 > -1) {
    c = c1;
  } else {
    die(__LINE__, "ERROR: No Battery found");
  }

  sprintf(battery_buffer, " Battery: %d%% ", c);
}

module_t battery = {battery_buffer, battery_update};
