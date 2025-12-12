CC=cl65
X16=~/src/x16/x16emu/x16emu -run

hotair:
	$(CC) -O -o HOTAIR.PRG -t cx16 -Ln hotair.lbl -C cx16-bank.cfg src/hotair.c src/wait.c

runhotair:
	../x16emu/x16emu -prg HOTAIR.PRG -run -debug

bitshift:
	$(CC) -O -o build/BITSHIFT.PRG -t cx16 src/bitshift.c

runbitshift: bitshift
	../x16emu/x16emu -prg BITSHIFT.PRG -run -debug

bankedram:
	$(CC) -O -o build/BANKEDRAM.PRG -t cx16 src/bankedram.c

runbankedram: bankedram
	../x16emu/x16emu -prg build/BANKEDRAM.PRG -run -debug

fire:
	$(CC) -O -o build/FIRE.PRG -t cx16  src/fire.c src/wait.c

run: fire
	cd assets && $(X16) -debug -prg ../build/FIRE.PRG && cd -

benchmark:
	$(CC) -O -o build/BENCHMARK.PRG -t cx16 src/benchmark.c

runbenchmark: benchmark
	../x16emu/x16emu -prg build/BENCHMARK.PRG -run -debug

benchmark-reads:
	$(CC) -O -o build/BENCHMARK-READS.PRG -t cx16 src/benchmark-reads.c

runbenchmark-reads: benchmark-reads
	../x16emu/x16emu  -prg build/BENCHMARK-READS.PRG -run -debug