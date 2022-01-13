OSCILLATOR = tidal
PROJECT = $(OSCILLATOR)

UCXXSRC = tidal-modulator.cc \
	tides-resources.cc \
	eurorack/tides/generator.cc \
	eurorack/stmlib/dsp/units.cc

UINCDIR = tides_data
	
include makefile.inc

# Resources need to be re-generated from Tides module in order to save memory
tides-resources.cc: tides_data/waves.bin
	python2 eurorack/stmlib/tools/resources_compiler.py tides_data/resources.py
