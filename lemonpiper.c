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

#define PORT 8383
#define BUFLEN 512

char buf_kdwm[BUFLEN];
char buf_date[BUFLEN];
char buf_power[BUFLEN];
char buf_volume[BUFLEN];
char buf_backlight[BUFLEN];

FILE *lemonptr;

static void die (int line_number, const char * format, ...) {
  va_list vargs;
  va_start (vargs, format);
  fprintf (stderr, "%d: ", line_number);
  vfprintf (stderr, format, vargs);
  fprintf (stderr, ".\n");
  va_end (vargs);
  exit (1);
}

void update() {
  fprintf(lemonptr, "%{l}%s%{r}%s %s %s %s\n", buf_kdwm, buf_backlight, buf_volume, buf_power, buf_date);
  fflush(lemonptr);
}

void format_kdwmc_module(int num_of_tags) {
  char tmp_buf[64];
  char *layouts = "TM";
  int tag_mask, current_layout;
  sscanf(buf_kdwm, "%d %d", &tag_mask, &current_layout);
  buf_kdwm[0] = '\0';
  for (int i = 0; i < num_of_tags; ++i) {
    if (1<<i & tag_mask) {
      sprintf(tmp_buf, "%{F#cc241d B#363636} %d %{B- F-}", i+1);
    } else {
      sprintf(tmp_buf, " %d ", i+1);
    }
    strcat(buf_kdwm, tmp_buf);
  }

  sprintf(tmp_buf, "[%c] ", layouts[current_layout]);
  strcat(buf_kdwm, tmp_buf);
}

void *kdwm_module_thread(void *vargp) {
  struct sockaddr_in server_addr;
  int s, in_len, slen = sizeof(server_addr);
  int num_of_tags;

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    die(__LINE__, "ERROR: Creating socket");

  server_addr.sin_family = AF_INET; // IPv4
  server_addr.sin_addr.s_addr = htonl(0x7F000001); // Ip-Adresse
  server_addr.sin_port = htons(PORT); // Port

  if (sendto(s, "get num_of_tags", 16, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");
  num_of_tags = atoi(buf_kdwm);

  if (sendto(s, "subscribe", 19, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");

  if (sendto(s, "get tag_mask", 13, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");
  format_kdwmc_module(num_of_tags);
  update();

  while (1) {
    if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
      die(__LINE__, "ERROR: Receiving response");
    format_kdwmc_module(num_of_tags);
    update();
  }
}

void backlight_module() {
  FILE *fp = popen("xib", "r");
  if (!fp)
      die(__LINE__, "ERROR: Need dependency xib");
  int p;
  fscanf(fp, "%d", &p);
  sprintf(buf_backlight, "%{B#363636} Backlight: %d %{B-}", p);
}


void date_module() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(buf_date, "%{B#363636} Date: %02d.%02d.%04d %{B-} %{B#363636} Time: %02d:%02d:%02d %{B-}", tm.tm_mday, tm.tm_mon, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void power_module() {
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

  sprintf(buf_power, "%{B#363636} Power: %d%% %{B-}", c);
}

void volume_module() {
  FILE *fp = popen("amixer sget Master | awk -F\"[][]\" '/%/ { print $2 }'", "r");
  if (!fp)
      die(__LINE__, "ERROR: Need dependency amixer");
  int v;
  fscanf(fp, "%d%", &v);
  pclose(fp);
  fp = popen("amixer sget Master | awk -F\" \" '{ print $6 }' | grep off", "r");
  if (!fp)
      die(__LINE__, "ERROR: Need dependency amixer");
  char c = getc(fp);
  pclose(fp);
  if ( c == EOF ) {
    sprintf(buf_volume, "%{B#363636} Volume: %d%% %{B-}", v);
  } else {
    sprintf(buf_volume, "%{B#363636} Volume: mute %{B-}");
  }

}

int main(int argc, char *argv[]) {
  if (argc > 1 && !strcmp(argv[1], "nobar")) {
    lemonptr = stdout;
  } else {
    system("killall -q lemonbar");
    lemonptr = popen("lemonbar -p -g 1920x24+0+0 -F '#ebdbb2' -B '#262626' -U '#FF0000' -f 'InputMono'-9", "w");
    if (!lemonptr)
        die(__LINE__, "ERROR: Need dependency lemonbar");
  }


  pthread_t kdwm_module_thread_id;
  pthread_create(&kdwm_module_thread_id, NULL, kdwm_module_thread, NULL);
  while (1) {
    date_module();
    power_module();
    volume_module();
    backlight_module();
    update();
    sleep(1);
  }
  return 0;
}
