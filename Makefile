lemonpiper: lemonpiper.c
	cc -lpthread -o lemonpiper lemonpiper.c

install: lemonpiper
	cp -f lemonpiper /bin/lemonpiper

clean:
	rm -rf lemonpiper

run: lemonpiper
	./lemonpiper
