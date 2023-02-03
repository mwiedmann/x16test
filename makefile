CC=cl65

make:
	$(CC) -I lib -o pixeus.prg -t cx16 \
	game/main.c game/startup.c game/level.c game/gamelevels.c game/gametiles.c game/gamesprites.c game/welcome.c \
	lib/x16graphics.c lib/sprites.c lib/imageload.c lib/waitforjiffy.s

clean:
	rm lib/*.o images/*.o game/*.o

run:
	../emu/x16emu -prg pixeus.prg -run
