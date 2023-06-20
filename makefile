all:
	gcc -w -framework CoreGraphics -framework ApplicationServices -o OneClickQuits main.c

run:
	clear
	gcc -w -framework CoreGraphics -framework ApplicationServices -o OneClickQuits main.c
	./OneClickQuits