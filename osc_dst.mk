OSCILLATOR = dst
PROJECT = mo2_$(OSCILLATOR)

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/particle_engine.cc \
    eurorack/stmlib/utils/random.cc \
	eurorack/stmlib/dsp/units.cc

include makefile.inc
