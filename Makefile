CC=cl65
X16=~/src/x16/x16emu/x16emu -run -scale 2
hotair:
	$(CC) -O -o HOTAIR.PRG -t cx16 -Ln hotair.lbl -C cx16-bank.cfg src/hotair.c src/wait.c

runhotair:
	../x16emu/x16emu -prg HOTAIR.PRG -run -debug

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

burning-petscii:
	$(CC) -O -o build/BURNING-PETSCII.PRG -t cx16  src/burning-petscii.c src/wait.c src/vera-util.c src/bomb.c

run-burning-petscii: burning-petscii
	cd assets && $(X16) -debug -prg ../build/BURNING-PETSCII.PRG && cd -

clean:
	rm -f *.PRG build/*.PRG *.lbl build/*.lbl build/*.o