#include <alsa/asoundlib.h>

char alsa_buffer[16];

void alsa_update() {

  // create connection
  snd_mixer_t *handle;
  snd_mixer_open(&handle, 0);
  snd_mixer_attach(handle, "default");
  snd_mixer_selem_register(handle, NULL, NULL);
  snd_mixer_load(handle);

  snd_mixer_selem_id_t *sid;
  snd_mixer_selem_id_alloca(&sid);
  snd_mixer_selem_id_set_index(sid, 0);
  snd_mixer_selem_id_set_name(sid, "Master");

  snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

  // get mute
  int is_not_muted;
  if (snd_mixer_selem_has_playback_switch(elem))
    snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &is_not_muted);

  if (is_not_muted) {
    // get percentage
    long min, max, value;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &value);
    value = (int)(100*((double)(value-min))/((double)(max-min))+0.5);
    sprintf(alsa_buffer, " Volume: %d%% ", value);
  } else {
    sprintf(alsa_buffer, " Volume: muted ");
  }

  // close connection
  snd_mixer_close(handle);

}

module_t alsa = {alsa_buffer, alsa_update};
