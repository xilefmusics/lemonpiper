#include "config.h"

static void die (int line_number, const char *format, ...) {
  va_list vargs;
  va_start (vargs, format);
  fprintf (stderr, "%d: ", line_number);
  vfprintf (stderr, format, vargs);
  fprintf (stderr, ".\n");
  va_end (vargs);
  exit (1);
}

void update() {
  fputs("%{l}", lemonptr);
  for (int i = 0; i < LENGTH(blocks_left); ++i) {
    fprintf(lemonptr, "%{B%s}", blocks_left[i].background);
    fputs(blocks_left[i].module->buffer, lemonptr);
    fputs("%{B-} ", lemonptr);
  }
  fputs("%{c}", lemonptr);
  for (int i = 0; i < LENGTH(blocks_center); ++i) {
    fprintf(lemonptr, "% {B%s}", blocks_center[i].background);
    fputs(blocks_center[i].module->buffer, lemonptr);
    fputs("%{B-}", lemonptr);
  }
  fputs("%{r}", lemonptr);
  for (int i = 0; i < LENGTH(blocks_right); ++i) {
    fprintf(lemonptr, " %{B%s}", blocks_right[i].background);
    fputs(blocks_right[i].module->buffer, lemonptr);
    fputs("%{B-}", lemonptr);
  }
  fflush(lemonptr);
}

int main(int argc, char *argv[]) {
  if (argc > 1 && !strcmp(argv[1], "nobar")) {
    lemonptr = stdout;
  } else {
    system("killall -q lemonbar");
    lemonptr = popen("lemonbar -p -g" DIMENSIONS " -F" COLOR_FOREGROUND " -B" COLOR_BACKGROUND " -U" COLOR_UNDERLINE " -f" FONT, "w");
    if (!lemonptr)
        die(__LINE__, "ERROR: Need dependency lemonbar");
  }

  for (int i = 0; i < LENGTH(parallel_updates); ++i)
    parallel_updates[i]->update();

  long int counter = 0;
  while (1) {
    for (int i = 0; i < LENGTH(periodic_updates); ++i) {
      if (!(counter % periodic_updates[i].seconds)) {
        periodic_updates[i].module->update();
      }
    }
    sleep(1);
    update();
    ++counter;
  }

  return 0;
}
