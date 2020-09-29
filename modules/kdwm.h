#define PORT 8383

char kdwm_buffer[512];

void kdwm_format(int num_of_tags) {
  char tmp_buf[64];
  char *layouts = "TMF?";
  int tag_mask, current_layout;
  sscanf(kdwm_buffer, "%d %d", &tag_mask, &current_layout);
  current_layout = current_layout < 3 ? current_layout : 3;
  kdwm_buffer[0] = '\0';
  for (int i = 0; i < num_of_tags; ++i) {
    if (1<<i & tag_mask) {
      sprintf(tmp_buf, "%{F%s B%s} %d %{B- F-}", COLOR_FOREGROUND_ACTIVE, COLOR_MODULE_BACKGROUND, i+1);
    } else {
      sprintf(tmp_buf, " %d ", i+1);
    }
    strcat(kdwm_buffer, tmp_buf);
  }

  sprintf(tmp_buf, "[%c] ", layouts[current_layout]);
  strcat(kdwm_buffer, tmp_buf);
}

void *kdwm_thread(void *vargp) {
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
  if ((in_len = recvfrom(s, kdwm_buffer, 512, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");
  num_of_tags = atoi(kdwm_buffer);

  if (sendto(s, "subscribe", 19, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, kdwm_buffer, 512, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");

  if (sendto(s, "get tag_mask", 13, MSG_CONFIRM, (struct sockaddr*) &server_addr, slen) == -1)
    die(__LINE__, "ERROR: Sending request");
  if ((in_len = recvfrom(s, kdwm_buffer, 512, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
    die(__LINE__, "ERROR: Receiving response");
  kdwm_format(num_of_tags);
  update();

  while (1) {
    if ((in_len = recvfrom(s, kdwm_buffer, 512, MSG_WAITALL, (struct sockaddr *) &server_addr, &slen)) == -1)
      die(__LINE__, "ERROR: Receiving response");
    kdwm_format(num_of_tags);
    update();
  }
}

void kdwm_update() {
  pthread_t kdwm_thread_id;
  pthread_create(&kdwm_thread_id, NULL, kdwm_thread, NULL);
}


module_t kdwm = {kdwm_buffer, kdwm_update};
