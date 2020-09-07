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
  printf("%{l}%s%{r}%s %s %s\n", buf_kdwm, buf_volume, buf_power, buf_date);
}

void format_tag_mask(int num_of_tags) {
  char tmp_buf[64];
  int tag_mask = atoi(buf_kdwm);
  buf_kdwm[0] = '\0';
  for (int i = 0; i < num_of_tags; ++i) {
    if (1<<i & tag_mask) {
      sprintf(tmp_buf, "%{F#cc241d B#363636} %d %{B- F-}", i+1);
    } else {
      sprintf(tmp_buf, " %d ", i+1);
    }
    strcat(buf_kdwm, tmp_buf);
  }
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

  if (sendto(s, "reset tag_mask_observer", 24, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");

  if (sendto(s, "observe tag_mask", 19, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");

  if (sendto(s, "get tag_mask", 13, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");
  format_tag_mask(num_of_tags);
  update();

  while (1) {
    if ((in_len = recvfrom(s, buf_kdwm, BUFLEN, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
      die(__LINE__, "ERROR: Receiving response");
    format_tag_mask(num_of_tags);
    update();
  }
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
  if (c1 > -1 && c0 > 0)
    c = (c0 + c1) / 2;
  if (c0 > -1)
    c = c0;
  if (c == -1)
      die(__LINE__, "ERROR: No Battery found");

  sprintf(buf_power, "%{B#363636} Power: %d%% %{B-}", c);
}

void volume_module() {
  int v = 49;
  FILE *fp = popen("amixer sget Master | awk -F\"[][]\" '/%/ { print $2 }'", "r");
  if (!fp)
      die(__LINE__, "ERROR: Need edpendency amixer");
  fscanf(fp, "%d%", &v);

  sprintf(buf_volume, "%{B#363636} Volume: %d%% %{B-}", v);
}

int main(int argc, char *argv[]) {
  pthread_t kdwm_module_thread_id;
  pthread_create(&kdwm_module_thread_id, NULL, kdwm_module_thread, NULL);
  while (1) {
    date_module();
    power_module();
    volume_module();
    update();
    sleep(1);
  }
  return 0;
}
