#include <dirent.h>

char backlight_buffer[32];

int backlight_get_display(const char const *path, char *buffer) {
        struct dirent *dir;
        DIR *d = opendir(path);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_name[0] != '.') {
                    strcpy(buffer, dir->d_name);
                    closedir(d);
                    return 1;
                }
            }
            closedir(d);
        }
        return 0;
}

int backlight_get_number(const char const *path, const char const *name) {
    char file_name[128];
    strcpy(file_name, path);
    strcat(file_name, name);
    FILE *fp = fopen(file_name, "r");
    if (fp) {
        int result;
        fscanf(fp, "%d", &result);
        fclose(fp);
        return result;
    }
    return -1;
}

void backlight_update() {
  char path[128] = "/sys/class/backlight/";

  // get display
  backlight_get_display(path, path+21);

  // load data
  int max_brightness = backlight_get_number(path, "/max_brightness");
  int actual_brightness = backlight_get_number(path, "/actual_brightness");

  printf("%d %d\n", max_brightness, actual_brightness);

  // calculate percentage with transformation
  int current_percentage = 0;
  while ((int)(((double)current_percentage*current_percentage)/10000*max_brightness) < actual_brightness) {
    current_percentage++;
  }

  sprintf(backlight_buffer, " Backlight: %d%% ", current_percentage);
}

module_t backlight = {backlight_buffer, backlight_update};
