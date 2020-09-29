#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <pthread.h>
#include <time.h>

#define LENGTH(X) (sizeof X / sizeof X[0])

FILE *lemonptr;

typedef struct module {
  char *buffer;
  void (*update)();
} module_t;

typedef struct periodic_update {
  module_t *module;
  int seconds;
} periodic_update_t;

typedef struct block {
  module_t *module;
  char *background;
} block_t;

static void die (int line_number, const char *format, ...);

void update();
