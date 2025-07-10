CC=cl65

make:
	$(CC) -O -o HOTAIR.PRG -t cx16 src/*.c

run: make
	../x16emu/x16emu -prg HOTAIR.PRG -run