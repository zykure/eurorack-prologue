OSCILLATOR = nse
PROJECT = mo2_$(OSCILLATOR)

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/noise_engine.cc \
    eurorack/stmlib/utils/random.cc \
	eurorack/stmlib/dsp/units.cc

include makefile.inc
