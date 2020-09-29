lemonpiper: lemonpiper.c config.h definitions.h modules/kdwm.h modules/datetime.h modules/alsa.h modules/battery.h modules/backlight.h
	cc -lpthread -lasound -o lemonpiper lemonpiper.c

install: lemonpiper
	cp -f lemonpiper /bin/lemonpiper

clean:
	rm -rf lemonpiper

run: lemonpiper
	./lemonpiper
