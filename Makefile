CC=cl65

hotair:
	$(CC) -O -o HOTAIR.PRG -t cx16 -Ln hotair.lbl src/hotair.c src/wait.c

run: hotair
	../x16emu/x16emu -prg HOTAIR.PRG -run -debug

bitshift:
	$(CC) -O -o BITSHIFT.PRG -t cx16 src/bitshift.c

runbitshift: bitshift
	../x16emu/x16emu -prg BITSHIFT.PRG -run -debug