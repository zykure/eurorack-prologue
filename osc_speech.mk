OSCILLATOR = speech
PROJECT = $(OSCILLATOR)

OSC_DDEFS = -DUSE_LIMITER

UCXXSRC = speech_synth.cc \
	eurorack/plaits/dsp/speech/naive_speech_synth.cc \
	eurorack/plaits/dsp/speech/sam_speech_synth.cc \
	eurorack/plaits/resources.cc \
	eurorack/stmlib/utils/random.cc \
	eurorack/stmlib/dsp/units.cc


include makefile.inc
