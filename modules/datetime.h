char datetime_buffer[32];

void datetime_update() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(datetime_buffer, " Datetime: %02d.%02d.%04d %02d:%02d:%02d ", tm.tm_mday, tm.tm_mon, tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

module_t datetime = {datetime_buffer, datetime_update};
