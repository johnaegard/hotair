CC=cl65
X16=~/src/x16/x16emu/x16emu -run -scale 2

burning-petscii:
	$(CC) -O -o build/burning-petscii.prg -t cx16  src/burning-petscii.c src/wait.c src/vera-util.c src/bomb.c src/map.c

distrib-burning-petscii: burning-petscii
	mkdir -p distrib && \
	cp build/burning-petscii.prg distrib && \
	cp assets/face.bin assets/tiles.bin assets/overlay.bin assets/needle.bin assets/circle.bin distrib

run-burning-petscii: distrib-burning-petscii
	cd distrib && $(X16) -debug -prg burning-petscii.prg && cd -

bitshift:
	$(CC) -O -o build/BITSHIFT.PRG -t cx16 src/ancilliary/bitshift.c

runbitshift: bitshift
	../x16emu/x16emu -prg BITSHIFT.PRG -run -debug

bankedram:
	$(CC) -O -o build/BANKEDRAM.PRG -t cx16 src/ancilliary/bankedram.c

runbankedram: bankedram
	../x16emu/x16emu -prg build/BANKEDRAM.PRG -run -debug

benchmark:
	$(CC) -O -o build/BENCHMARK.PRG -t cx16 src/ancilliary/benchmark.c

runbenchmark: benchmark
	../x16emu/x16emu -prg build/BENCHMARK.PRG -run -debug

benchmark-reads:
	$(CC) -O -o build/BENCHMARK-READS.PRG -t cx16 src/ancilliary/benchmark-reads.c

runbenchmark-reads: benchmark-reads
	../x16emu/x16emu  -prg build/BENCHMARK-READS.PRG -run -debug

tile-edit: 
	$(X16) -capture -scale 2 -startin assets -prg assets/TELOADER.PRG -run 

petscii-dump:
	$(CC) -O -o build/PETSCIIDUMP.PRG -t cx16 src/ancilliary/petscii-dump.c

run-petscii-dump: petscii-dump
	rm -f assets/PETSCII*.BIN && $(X16) -startin assets -prg build/PETSCIIDUMP.PRG -run -debug

display-tileset:
	$(CC) -O -o build/DISPLAY-TILESET.PRG -t cx16 src/ancilliary/display-tileset.c src/vera-util.c

run-display-tileset: display-tileset
	cd assets && $(X16) -debug -prg ../build/DISPLAY-TILESET.PRG && cd -

clean:
	rm -f *.PRG build/*.PRG *.lbl build/*.lbl build/*.o distrib/*
