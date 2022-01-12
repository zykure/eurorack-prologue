OSCILLATOR = tidal
PROJECT = $(OSCILLATOR)

UCXXSRC = tidal-modulator.cc tides-data/resources.cc \
	eurorack/tides/generator.cc \
	eurorack/stmlib/dsp/units.cc

# Resources need to be re-generated from Tides module in order to save memory
tides-data/resources.cc: tides-data/waves.bin
	python2 eurorack/stmlib/tools/resources_compiler.py tides-data/resources.py

include makefile.inc
